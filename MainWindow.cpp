#include "wts_version.h"

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ui_Preferences.h"
#include "Exporter.h"
#include "EditController.hpp"
#include "WatchThatCode.h"
#include "Common.h"

#include <QLabel>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QHostInfo>
#include <QPrinter>
#include <QPainter>

using namespace WTS;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_project(0)
    , m_exporter(new Exporter(this))
    , m_muteOnRecord(true)
    , m_soloBuffer(0)
    , m_editController( new EditController(this) )
{
    ui->setupUi(this);
    if (qApp)
        qApp->installEventFilter(this);

    // set up controller
    connect(this, SIGNAL(projectChanged(Project*)), m_editController, SLOT(setProject(Project*)));
    connect(ui->actionAddMarker, SIGNAL(triggered()), m_editController, SLOT(addEventMarkerAtCursor()));
    connect(ui->actionAddScene, SIGNAL(triggered()), m_editController, SLOT(addSceneMarkerAtCursor()));

    // connect to the sampler
    connect(this, SIGNAL(samplerClear()), &m_audio, SLOT(samplerClear()));
    connect(this, SIGNAL(samplerClock(qint64)), &m_audio, SLOT(samplerClock(qint64)));
    connect(&m_audio, SIGNAL(endOfSample(WtsAudio::BufferAt*)), SLOT(onEndOfSample(WtsAudio::BufferAt*)) , Qt::QueuedConnection);

    // FIXME these two should be aware of each other
    connect(ui->timeLine, SIGNAL(bufferSelected(WtsAudio::BufferAt*)),
            ui->waveform, SLOT(updateWaveform(WtsAudio::BufferAt*)));
    connect(ui->waveform, SIGNAL(rangeChanged(SoundBuffer*)),
            ui->timeLine, SLOT(updateBuffer(SoundBuffer*)));

    connect(ui->gainSlider, SIGNAL(valueChanged(int)), ui->waveform, SLOT(setGain(int)));
    connect(ui->waveform, SIGNAL(adjustGainSlider(int)), ui->gainSlider, SLOT(setValue(int)));

    connect(this, SIGNAL(projectChanged(Project*)), ui->waveform, SLOT(setProject(Project*)));

    connect(ui->soloButton, SIGNAL(clicked()), ui->timeLine, SLOT(startSolo()));
    connect(ui->timeLine, SIGNAL(startSolo(WtsAudio::BufferAt*)), this, SLOT(startSolo(WtsAudio::BufferAt*)));

    connect(mediaObject(), SIGNAL(finished()), SLOT(onMovieFinished()));

    buildMovieSelector();

    constructStateMachine();

    // right aligning the logo (idea from http://www.ffuts.org/blog/right-aligning-a-button-in-a-qtoolbar/ )
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->toolBar->addWidget(spacer);

    QLabel * logo = new QLabel();
    logo->setPixmap( QPixmap(":Resources/favicon.png") );
    ui->toolBar->addWidget( logo );


    m_preferences = new Preferences(this);
    connect(ui->actionPreferences, SIGNAL(triggered()), m_preferences, SLOT(show()));
    connect(m_preferences->ui->muteOnRecord, SIGNAL(toggled(bool)), SLOT(setMuteOnRecord(bool)));

    m_preferences->ui->muteOnRecord->setChecked( m_settings.value("muteOnRecord", true).toBool() );
    m_preferences->ui->secPPage->setValue( m_settings.value("secPerPage", 15).toInt() );
    m_muteOnRecord = m_preferences->ui->muteOnRecord->isChecked();

}

MainWindow::~MainWindow()
{
    // this is necessary to ensure widgets won't get tick'ed after player is deaded
    ui->videoPlayer->stop();

    delete ui;
}

void MainWindow::seek(qint64 ms)
{
    mediaObject()->seek(ms);
    emit samplerClear();
    m_editController->seek(ms);
    if (ui->actionPlay->isChecked()) {
        emit samplerClock(ms);
    }
}

void MainWindow::loadMovie(const QString& path)
{
    resetData();

    ui->videoPlayer->load(Phonon::MediaSource(path));
    ui->videoPlayer->setVolume(0);

    connect(mediaObject(), SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
    mediaObject()->setTickInterval(50);

    ui->waveform->connect(this,
                          SIGNAL(scratchUpdated(WtsAudio::BufferAt *, bool)),
                          SLOT(updateWaveform(WtsAudio::BufferAt *, bool)));

    // further configuration
    ui->storyboard->setSeekOnDrag(true);
    ui->tension->setSeekOnDrag(true);
    ui->score->setSeekOnDrag(true);

    // scratch should be big enough to fit a movie-long sound
    // this should happen before loadData so we know video size and have
    // access to thumbnails
    m_project = new Project(path, this);
    emit projectChanged(m_project);
    connect(m_preferences->ui->secPPage, SIGNAL(valueChanged(int)), m_project, SLOT(setSecPerPage(int)));
    m_project->setSecPerPage(m_preferences->ui->secPPage->value());

    connect(m_editController, SIGNAL(samplerSchedule(WtsAudio::BufferAt*)),
            &m_audio, SLOT(samplerSchedule(WtsAudio::BufferAt*)));

    m_scratch.setBuffer(
                new SoundBuffer(
                    WtsAudio::msToSampleCount(m_project->duration())));
    m_scratch.buffer()->setColor( Qt::red );

    ui->storyboard->setVideoSize(m_project->videoWidth(), m_project->videoHeight());

    m_project->load();

    emit loaded();
}

void MainWindow::resetData()
{
    if (m_project) {
        delete m_project;
        m_project = 0;
    }

    m_scratch.setAt(0);
}

Phonon::MediaObject * MainWindow::mediaObject()
{
    Q_ASSERT(ui!=NULL);
    Q_ASSERT(ui->videoPlayer);
    return ui->videoPlayer->mediaObject();
}

void MainWindow::setFullscreen(bool fs) {
    ui->actionFullscreen->setChecked(fs);
    if (fs) {
        showFullScreen();
    } else {
        showNormal();
    }
}


void MainWindow::onPlay(bool play)
{
    if (play) {
        emit samplerClear();

        m_audio.start();
        m_editController->start();

        if (m_soloBuffer) {
            emit samplerClock( m_soloBuffer->rangeStartAt() );
            m_audio.samplerSchedule(m_soloBuffer);
        }

        // if at the very end of the film - start from the beginning
        if (m_project->duration() - mediaObject()->currentTime() < 40) {
            ui->videoPlayer->seek(0);
        }

        ui->videoPlayer->play();
    } else {
        m_audio.stop();
        emit samplerClear();
        emit stopped();
        m_soloBuffer = 0;
        ui->videoPlayer->pause();
        ui->actionRecord->setChecked(false);
    }
}

void MainWindow::onRecord(bool record)
{
    if (m_muteOnRecord)
        m_audio.setMute( record );

    if (record) {
        // started recording
        m_scratch.buffer()->setWritePos(0);
        m_scratch.buffer()->setColor(Qt::red);
    } else {
        // done with recording - make a new sample buffer

        if (m_scratch.buffer()->sampleCount() > 0) {
            WtsAudio::BufferAt * newBuff = m_project->copyScratch(&m_scratch);
            ui->videoPlayer->seek(newBuff->at());
            emit scratchUpdated(&m_scratch, false);
            ui->waveform->updateWaveform( newBuff, false );
            m_project->save();
        }
    }
}

void MainWindow::tick(qint64 ms)
{
    if (ui->actionRecord->isChecked()) {
        // recording
        if (m_scratch.buffer()->m_writePos == 0)
            m_scratch.setAt(ms);
        if ( m_audio.capture(m_scratch.buffer()) > 0) {
            emit scratchUpdated(&m_scratch, true);

            if (m_scratch.buffer()->freeToWrite() == 0) {
                ui->actionRecord->setChecked(false);
            }
        }
    }

    ui->waveform->tick(ms);
    emit samplerClock(ms);

    float volume = m_audio.getVolume();
    ui->vuLeft->setValue( volume );
    ui->vuRight->setValue( volume );

    // find out which samples to trigger
    if (!m_soloBuffer && ui->actionPlay->isChecked()) {
        m_editController->advanceSequenceCursor(ms);
    }
}

void MainWindow::exportMovie()
{
    // setting the parent to this makes it a "sheet" dialog on OSX
    QProgressDialog progress("Opslaan...", "Stop", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(500);

    m_exporter->configure(
            m_project->dataDir().filePath(
                QString("%1-%2-export.%3")
                .arg(m_project->movieFilename())
                .arg(QHostInfo::localHostName())
                .arg(VIDEO_FMT)),
            m_project->videoFile(),
            m_project,
            &m_audio,
            &progress);
    m_exporter->run();
}

void MainWindow::constructStateMachine()
{
    QState * selector = new QState();
    QState * firstPlay = new QState();
    m_workshop = new QState();

    selector->assignProperty(menuBar(), "visible", false);

    selector->addTransition(this,SIGNAL(loaded()), firstPlay);
    connect(selector, SIGNAL(exited()), ui->movieSelectorScrollArea, SLOT(hide()));

    firstPlay->addTransition(this, SIGNAL(stopped()), m_workshop);
    connect(firstPlay, SIGNAL(entered()), ui->videoStripe, SLOT(show()));
    connect(firstPlay, SIGNAL(entered()), ui->actionPlay, SLOT(toggle()));

    connect(m_workshop, SIGNAL(entered()), ui->toolBar, SLOT(show()));

    m_machine.setGlobalRestorePolicy(QStateMachine::RestoreProperties);
    m_machine.addState(firstPlay);
    m_machine.addState(m_workshop);
    m_machine.addState(selector);
    m_machine.setInitialState(selector);

    // first hide all
    ui->videoStripe->hide();
    ui->toolBar->hide();
    ui->storyboard->hide();
    ui->timeLine->hide();
    ui->recorder->hide();
    ui->tension->hide();
    ui->score->hide();

    m_tabActions = new QActionGroup(this);

    QState * storyboardPage = addPage("1", QList<QWidget*>()
            << ui->storyboard,
            QList<QAction*>()
            << ui->actionAddMarker
            << ui->actionAddScene);
    ui->storyboard->setEditMode(false);
    storyboardPage->assignProperty( ui->storyboard, "editMode", true);

    //QState * tensionPage =
            addPage("2",  QList<QWidget*>()
                    << ui->storyboard
                    << ui->tension);
    QState * scorePage = addPage("3", QList<QWidget*>()
            << ui->storyboard
            << ui->score);

    ui->score->setEditMode(false);
    scorePage->assignProperty( ui->score, "editMode", true);

    addPage("4", QList<QWidget*>()
            << ui->storyboard
            << ui->score
            << ui->timeLine
            << ui->recorder);

    m_machine.start();
}

QState * MainWindow::addPage(const QString& name, QList<QWidget*> widgets, QList<QAction*> actions)
{
    QState * state = new QState(m_workshop);
    foreach(QWidget * w, widgets) {
        state->assignProperty(w, "visible", true);
    }
    foreach(QAction * a, actions) {
        state->assignProperty(a, "enabled", true);
    }
    QAction * action = ui->toolBar->addAction(name);
    m_tabActions->addAction( action );
    action->setCheckable(true);
    action->setShortcut(QKeySequence(name));
    m_workshop->addTransition(action, SIGNAL(triggered()), state);

    if (!m_workshop->initialState()) {
        m_workshop->setInitialState(state);
        action->setChecked(true);
    }
    return state;
}

void MainWindow::buildMovieSelector()
{
    QVBoxLayout * layout0 = new QVBoxLayout;
    ui->movieSelector->setLayout(layout0);

    QLabel * title = new QLabel("Watch That Sound");
    QFont titleFont("Sans-Serif", 32, 500);
    title->setFont(titleFont);
    layout0->addWidget(title, 0, Qt::AlignCenter);

    QLabel * version = new QLabel( QString("<center>%1<br/><img src='%2'/></center>")
                                   .arg(WTS_VERSION).arg(":Resources/Icon-nobg.png") );
    layout0->addWidget(version, 0, Qt::AlignCenter);

    QWidget * grid = new QWidget;
    layout0->addWidget(grid, 1);


    QGridLayout * layout = new QGridLayout;
    grid->setLayout(layout);


    QSignalMapper * mapper = new QSignalMapper(this);

    QFileInfoList movList = Project::movDir().entryInfoList(QStringList()
            << QString("*.%1").arg(VIDEO_FMT));

    int count = movList.size();

    if (count > 0) {

        int minColWidth = 100, maxCols = width() / minColWidth;
        int cols = (count>3) ? (int)ceilf( sqrtf( (float) count ) ) : count;
        if (cols > maxCols)
            cols = maxCols;

        QSize iconSize = size() / cols;

        int i = 0, offs = 1;
        foreach(QFileInfo fi, movList) {
            VideoFile vf;
            try {
                vf.open(fi.absoluteFilePath());
            } catch (const WTS::AssertFailed& e) {
                qDebug() << "Skipping movie" << qPrintable(fi.fileName());
                qDebug() << e.pMessage();
                continue;
            }

            QPushButton * button = new QPushButton( );
            layout->addWidget( button, offs + i/cols, i%cols );
            i++;
            button->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
            mapper->setMapping( button, fi.absoluteFilePath() );
            connect(button, SIGNAL(clicked()), mapper, SLOT(map()));

            vf.seek( vf.duration()/3 );
            QPixmap thumb = QPixmap::fromImage(
                    vf.frame().scaled(iconSize,Qt::KeepAspectRatio) );

            qint64 tt = vf.duration(), min = tt / 60000, sec = tt / 1000 % 60;
            button->setToolTip(QString("%1\n%2x%3\n%4:%5")
                    .arg(fi.fileName())
                    .arg(vf.width()).arg(vf.height())
                    .arg(min).arg(sec,2,10,QLatin1Char('0')) );

            button->setIcon(QIcon(thumb));
            button->setIconSize(iconSize);
        }

        connect(mapper, SIGNAL(mapped(QString)), SLOT(loadMovie(QString)));
    } else {
        QLabel * oops = new QLabel(
                QString("De map '%1' bevat geen filmpjes (formaat '%2').\n"
                    "Plaats een aantal van zulke bestanden in de map en start de tool opnieuw.")
                .arg(Project::movDir().path()).arg(VIDEO_FormatTitle));
        layout->addWidget(oops,0,0);
        layout->setAlignment(oops, Qt::AlignHCenter);
    }
}

void MainWindow::refreshTension()
{
    ui->tension->setCurve(
            m_project->tensionCurve(ui->tension->sceneRect().width()));
}

QPainterPath MainWindow::tensionCurve() const
{
    return ui->tension->curve();
}

void MainWindow::onMovieFinished()
{
    ui->actionPlay->setChecked(false);
}

bool MainWindow::eventFilter( QObject * watched, QEvent * event )
{
    if ( watched != qApp )
        goto finished;

    if ( event->type() != QEvent::ApplicationActivate )
        goto finished;

    if (m_project)
        m_project->reScanSamples();

finished:
    return QMainWindow::eventFilter( watched, event );

}

void MainWindow::startSolo(WtsAudio::BufferAt * bufferAt)
{
    ui->actionPlay->setChecked(false);
    seek( bufferAt->rangeStartAt() );
    m_soloBuffer = bufferAt;
    ui->actionPlay->setChecked(true);
}

void WTS::MainWindow::onEndOfSample(WtsAudio::BufferAt * seqBuffer)
{
    if (seqBuffer == m_soloBuffer)
        ui->actionPlay->setChecked(false);
}

void WTS::MainWindow::printAction()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPaperSize( QPrinter::A4 );
    printer.setOrientation( QPrinter::Landscape );

    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle(tr("Print Document"));
    if (dialog->exec() != QDialog::Accepted)
        return;

    m_project->print(printer, ui->score->scoreSymbols());
}

void WTS::MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void WTS::MainWindow::writeSettings()
{
    m_settings.setValue("muteOnRecord", m_muteOnRecord);
    m_settings.setValue("secPerPage", m_preferences->ui->secPPage->value());
    m_settings.sync();
}

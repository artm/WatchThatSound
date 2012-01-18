#include "wts_version.h"

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ui_Preferences.h"
#include "Rainbow.h"
#include "Exporter.h"

#include "WatchThatCode.h"
#include "Common.h"

#include <QLabel>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QHostInfo>

using namespace WTS;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_movDirFound(false)
    , m_lastSampleNameNum(0)
    , m_videoFile(0)
    , m_loading(false)
    , m_exporter(new Exporter(this))
    , m_finalTension(0.5)
    , m_muteOnRecord(true)
    , m_settings("WatchThatSound","WTS-Workshop-Tool")
{
    ui->setupUi(this);

    // connect to the sampler
    connect(this, SIGNAL(samplerClear()), &m_audio, SLOT(samplerClear()));
    connect(this, SIGNAL(samplerClock(qint64)), &m_audio, SLOT(samplerClock(qint64)));
    connect(this, SIGNAL(samplerSchedule(WtsAudio::BufferAt*)),
            &m_audio, SLOT(samplerSchedule(WtsAudio::BufferAt*)));

    // change signals to save data...
    connect(ui->tension, SIGNAL(updateLevel(int,float)), SLOT(updateMarkerTension(int,float)));
    connect(ui->tension, SIGNAL(dataChanged()), SLOT(saveData()));
    connect(ui->tension, SIGNAL(dataChanged()), ui->storyboard, SLOT(update()));
    connect(ui->tension, SIGNAL(dataChanged()), ui->timeLine, SLOT(update()));
    connect(ui->tension, SIGNAL(dataChanged()), ui->score, SLOT(update()));
    connect(ui->score, SIGNAL(dataChanged()), SLOT(saveData()));
    connect(ui->waveform, SIGNAL(rangeChanged()), SLOT(saveData()));

    connect(ui->timeLine, SIGNAL(bufferSelected(WtsAudio::BufferAt*)),
            ui->waveform, SLOT(updateWaveform(WtsAudio::BufferAt*)));
    connect(ui->waveform, SIGNAL(rangeChanged(SoundBuffer*)),
            ui->timeLine, SLOT(updateBuffer(SoundBuffer*)));

    connect(ui->gainSlider, SIGNAL(valueChanged(int)), ui->waveform, SLOT(setGain(int)));
    connect(ui->waveform, SIGNAL(adjustGainSlider(int)), ui->gainSlider, SLOT(setValue(int)));

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
    m_muteOnRecord = m_preferences->ui->muteOnRecord->isChecked();

}

MainWindow::~MainWindow()
{
    // this is necessary to ensure widgets won't get tick'ed after player is deaded
    ui->videoPlayer->stop();

    m_settings.setValue("muteOnRecord", m_muteOnRecord);

    delete ui;
}

QString MainWindow::makeSampleName()
{
    return QString("sample_%1.raw").arg(++m_lastSampleNameNum);
}

void MainWindow::seek(qint64 ms)
{
    mediaObject()->seek(ms);
    emit samplerClear();
    if (ui->actionPlay->isChecked()) {
        m_sequenceCursor = m_sequence.begin();
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
    if (m_videoFile) delete m_videoFile;
    m_videoFile = new VideoFile(path, this);

    m_scratch.setBuffer(
                new SoundBuffer(
                    WtsAudio::msToSampleCount(m_videoFile->duration())));
    m_scratch.buffer()->setColor( Qt::red );

    // now setup dataPath and try to load files from there
    QDir movieDir = QFileInfo(path).dir();
    m_movInfo = QFileInfo(path);
    m_dataDir = QDir( movieDir.filePath( QFileInfo(path).completeBaseName() + ".data") );

    if (! m_dataDir.exists() ) { movieDir.mkdir( m_dataDir.dirName() ); }

    ui->storyboard->setVideoSize(m_videoFile->width(), m_videoFile->height());

    loadData();

    emit loaded();
}

void MainWindow::resetData()
{
    m_dataDir.setPath("");;
    m_scratch.setAt(0);
    m_sequence.clear();;
    m_sequenceCursor = m_sequence.begin();
    m_markers.clear();
    m_lastSampleNameNum = 0;
    if (m_videoFile) delete m_videoFile;
    m_videoFile = 0;
    m_loading = false;
}

void MainWindow::saveData()
{
    // make sure we don't overwrite while loading...
    if (m_loading)
        return;

    QFile dataFile( m_dataDir.filePath("metadata.xml.tmp") );
    dataFile.open(QFile::WriteOnly | QFile::Text);

    QXmlStreamWriter xml( &dataFile );

    xml.setAutoFormatting(true);

    xml.writeStartDocument();
    xml.writeDTD("<!DOCTYPE soundtrack>");
    xml.writeStartElement("soundtrack");
    xml.writeAttribute("version", WTS_VERSION);
    xml.writeAttribute("movie", m_movInfo.fileName());

    xml.writeStartElement("storyboard");
    xml.writeAttribute("final_tension", QString("%1").arg(m_finalTension));
    foreach(Marker * m, m_markers) {
        xml.writeStartElement("marker");
        QString ms;
        ms.setNum(m->at());
        xml.writeAttribute("ms",ms);
        xml.writeAttribute("type", m->type() == SCENE ? "scene" : "event");
        xml.writeAttribute("tension", QString("%1").arg(m->tension()));
        xml.writeEndElement();
    }
    xml.writeEndElement();

    xml.writeStartElement("score");
    ui->score->saveData(xml);
    xml.writeEndElement();

    xml.writeStartElement("sequence");
    xml.writeAttribute("counter",QString("%1").arg(m_lastSampleNameNum));
    foreach(WtsAudio::BufferAt * buffer, m_sequence) {
        xml.writeStartElement("sample");
        qint64 at_int = buffer->at();
        QString at_s = QString("%1").arg(at_int);
        xml.writeAttribute("ms",at_s);
        xml.writeAttribute("id",buffer->buffer()->name());
        // FIXME: this shouldn't be here, but in a separate samples chunk
        xml.writeAttribute("range_start", QString("%1").arg(buffer->buffer()->rangeStart()));
        xml.writeAttribute("range_end", QString("%1").arg(buffer->buffer()->rangeEnd()));
        xml.writeAttribute("gain", QString("%1").arg( buffer->buffer()->gain() ));
        xml.writeEndElement();

        QFile wav( m_dataDir.filePath( buffer->buffer()->name() ));
        buffer->buffer()->save(wav);
    }

    xml.writeEndElement();
    xml.writeEndDocument();
    dataFile.close();
    QFile::remove( m_dataDir.filePath("metadata.xml") );
    dataFile.rename(  m_dataDir.filePath("metadata.xml") );
}

void MainWindow::loadData()
{
    m_loading = true;

    // always have a scene starting at 0
    addMarker(SCENE, 0);

    QFile dataFile( m_dataDir.filePath("metadata.xml") );
    dataFile.open(QFile::ReadOnly | QFile::Text);
    QRegExp idRe = QRegExp("sample_(\\d+)");
    QXmlStreamReader xml( &dataFile );
    xml.readNextStartElement();
    if (xml.name() == "soundtrack") {
        while (xml.readNextStartElement()) {
            if (xml.name() == "storyboard") {
                m_finalTension = xml.attributes().hasAttribute("final_tension")
                        ? xml.attributes().value("final_tension").toString().toFloat()
                        : 0.5;
                while(xml.readNextStartElement()) {

                    float tension = xml.attributes().hasAttribute("tension")
                            ? xml.attributes().value("tension").toString().toFloat()
                            : 0.5;

                    addMarker(xml.attributes().value("type") == "scene" ? SCENE : EVENT,
                              xml.attributes().value("ms").toString().toLongLong(),
                              tension);


                    // this is one way to read a flat list of <foo/> not recursing
                    xml.readElementText();
                }
                refreshTension();
            } else if (xml.name() == "score") {
                ui->score->loadData(xml);
            } else if (xml.name() == "sequence") {
                m_lastSampleNameNum =
                        xml.attributes().value("counter").toString().toInt();
                while(xml.readNextStartElement()) {
                    QString id = xml.attributes().value("id").toString();
                    QFile wav( m_dataDir.filePath( id ));

                    SoundBuffer * sb = new SoundBuffer();
                    sb->load(wav);
                    sb->setRange(xml.attributes().value("range_start").toString().toLongLong(),
                                 xml.attributes().value("range_end").toString().toLongLong());

                    QString at_s =  xml.attributes().value("ms").toString();
                    qint64 at_int = at_s.toLongLong();
                    WtsAudio::BufferAt * buffer =
                            new WtsAudio::BufferAt(sb,
                                                   at_int,
                                                   this);
                    m_sequence.append( buffer );

                    // find out original number / color
                    if (idRe.indexIn(id) > -1) {
                        int color_index = idRe.cap(1).toInt();
                        buffer->buffer()->setColor( Rainbow::getColor( color_index ) );
                    } else {
                        qDebug() << "Color index didn't parse...";
                    }

                    buffer->buffer()->initGains();
                    if (xml.attributes().hasAttribute("gain"))
                        buffer->buffer()->setGain( xml.attributes().value("gain").toString().toFloat() );

                    emit newBufferAt(buffer);
                    // finish off the element...
                    xml.readElementText();
                }
            }
        }
    } else {
        qDebug() << dataFile.fileName() << " is not a soundtrack file";
    }

    m_loading = false;

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
        qSort(m_sequence.begin(), m_sequence.end(), WtsAudio::startsBefore);
        m_sequenceCursor = m_sequence.begin();

        // if at the very end of the film - start from the beginning
        if (duration() - mediaObject()->currentTime() < 40) {
            ui->videoPlayer->seek(0);
        }

        ui->videoPlayer->play();
    } else {
        m_audio.stop();
        emit samplerClear();
        emit stopped();
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
            WtsAudio::BufferAt * newBuff =
                    new WtsAudio::BufferAt(
                        new SoundBuffer(makeSampleName(),
                                        *m_scratch.buffer(),
                                        m_scratch.buffer()->m_writePos),
                        m_scratch.at(),
                        this);
            newBuff->buffer()->setColor( Rainbow::getColor(m_lastSampleNameNum) );
            newBuff->buffer()->initGains();
            m_sequence.append(newBuff);
            emit scratchUpdated(newBuff, false);
            emit newBufferAt(newBuff);
            ui->videoPlayer->seek(m_scratch.at());
            saveData();
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

    emit samplerClock(ms);

    float volume = m_audio.getVolume();
    ui->vuLeft->setValue( volume );
    ui->vuRight->setValue( volume );

    // find out which samples to trigger
    if (ui->actionPlay->isChecked()) {
        while( m_sequenceCursor != m_sequence.end()
              && ((*m_sequenceCursor)->at()
              + WtsAudio::sampleCountToMs((*m_sequenceCursor)->buffer()->rangeStart())) <= ms ) {
            emit samplerSchedule( *m_sequenceCursor );
            m_sequenceCursor++;
        }
    }

}

void MainWindow::addMarker(MarkerType type, qint64 when, float tension)
{
    if (when < 0)
        when = mediaObject()->currentTime();
    m_markers[when] = new Marker(type, when, this);
    m_markers[when]->setTension( tension );
    // load frameshot...
    m_videoFile->seek(when);
    m_markers[when]->setSnapshot( QPixmap::fromImage(m_videoFile->frame()) );

    refreshTension();
    emit storyBoardChanged();
    saveData();
}

void MainWindow::removeMark(Marker * m)
{
    m_markers.remove( m->at() );
    refreshTension();
    emit storyBoardChanged();
    saveData();
}

QList<MainWindow::Marker *> MainWindow::getMarkers(MarkerType type, bool forward) const
{
    QList<Marker *> scenes;
    foreach(Marker * m, m_markers) {
        if (type == ANY || m->type() == type) {
            if (forward)
                scenes.append(m);
            else
                scenes.prepend(m);
        }
    }

    return scenes;
}

void MainWindow::exportMovie()
{
    // setting the parent to this makes it a "sheet" dialog on OSX
    QProgressDialog progress("Opslaan...", "Stop", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(500);

    m_exporter->configure(
            m_dataDir.filePath(
                QString("%1-%2-export.%3")
                .arg(m_movInfo.fileName())
                .arg(QHostInfo::localHostName())
                .arg(VIDEO_FMT)),
            m_videoFile,
            m_sequence,
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

    QFileInfoList movList = movDir().entryInfoList(QStringList()
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
                .arg(movDir().path()).arg(VIDEO_FormatTitle));
        layout->addWidget(oops,0,0);
        layout->setAlignment(oops, Qt::AlignHCenter);
    }
}

void MainWindow::refreshTension()
{
    QMapIterator<qint64, Marker *> iter(m_markers);
    QPainterPath curve;
    bool init = true;
    while(iter.hasNext()){
        iter.next();
        Marker * m = iter.value();

        float x = (float)ui->tension->sceneRect().width()
            * m->at() / m_videoFile->duration();

        if (init) {
            curve.moveTo(QPointF(x, m->tension()));
            init = false;
        } else
            curve.lineTo( QPointF(x, m->tension()) );
    }

    curve.lineTo( QPointF(ui->tension->sceneRect().width(), m_finalTension));
    ui->tension->setCurve(curve);
}

QPainterPath MainWindow::tensionCurve() const
{
    return ui->tension->curve();
}

void MainWindow::onMovieFinished()
{
    ui->actionPlay->setChecked(false);
}

void MainWindow::updateMarkerTension(int markerIndex, float tension)
{
    if (markerIndex < m_markers.size() )
        m_markers[ m_markers.keys()[markerIndex] ]->setTension(tension);
    else if (markerIndex  == m_markers.size()) {
        m_finalTension = tension;
    }
}

void MainWindow::removeBuffer(WtsAudio::BufferAt *bufferAt)
{
    ui->waveform->clearWaveform(bufferAt->buffer());
    m_sequence.removeAll(bufferAt);
    saveData();
}


#if defined(__APPLE__)
#define nextToExe(p) QCoreApplication::applicationDirPath () + "/../../.." + p
#else
#define nextToExe(p) QCoreApplication::applicationDirPath () + p
#endif

QDir MainWindow::movDir()
{
    if (!m_movDirFound) {

        QString stdMoviesPath = QDesktopServices::storageLocation(
                QDesktopServices::MoviesLocation );

        m_movDir = QDir(stdMoviesPath).filePath("Watch That Sound Movies");

        if (!m_movDir.exists()) {
            // first try pre 3.0.3 convention

            QDir tryDir;
            tryDir = QDir(nextToExe("/WTSmovie"));
            if (!tryDir.exists())
                // then try 3-beta convention
                tryDir = QDir(nextToExe("/movie"));

            // upgrade from beta to actual version
            if (tryDir.exists()) {
                QDir().rename(tryDir.path(),m_movDir.path());

                QString info =
                    QString("Oude movie map %1 verplaatst naar de nieuwe locatie: %2")
                    .arg(tryDir.path()).arg(m_movDir.path());

                QMessageBox message(QMessageBox::Information, "Upgrade info",
                        info, QMessageBox::Ok);
                message.exec();
            } else {
                m_movDir.mkdir(m_movDir.path());
                // see if we have sample films installed
                QDir distVideos = QDir(nextToExe("/../video"));
                if (distVideos.exists()) {
                    foreach(QString path,
                            distVideos.entryList(QStringList()
                                << QString("*.%1").arg(VIDEO_FMT))) {
                        // copy sample films to movDir
                        qDebug() << "cp " << path << " to " << m_movDir.filePath(path);
                        QFile(distVideos.filePath(path)).copy(m_movDir.filePath(path));
                    }
                } else {
                    qWarning() << "No dist videos at " << distVideos.path();
                }
            }
        }
    }

    return m_movDir;
}

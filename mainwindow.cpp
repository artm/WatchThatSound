#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "rainbow.h"
#include "exporter.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_lastSampleNameNum(0)
    , m_videoFile(0)
    , m_loading(false)
    , m_exporter(new Exporter(this))
{
    ui->setupUi(this);

    // connect to the sampler
    connect(this, SIGNAL(samplerClear()),
            &m_audio, SLOT(samplerClear()));
    connect(this, SIGNAL(samplerClock(qint64)),
            &m_audio, SLOT(samplerClock(qint64)));
    connect(this, SIGNAL(samplerSchedule(WtsAudio::BufferAt*)),
            &m_audio, SLOT(samplerSchedule(WtsAudio::BufferAt*)));

    connect(ui->tension, SIGNAL(dataChanged()), SLOT(saveData()));
    connect(ui->tension, SIGNAL(dataChanged()), ui->storyboard, SLOT(update()));
    connect(ui->tension, SIGNAL(dataChanged()), ui->timeLine, SLOT(update()));
    connect(ui->tension, SIGNAL(dataChanged()), ui->score, SLOT(update()));
    connect(ui->score, SIGNAL(dataChanged()), SLOT(saveData()));

    buildMovieSelector();

    constructStateMachine();
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

    // we need to be in pause so the seek bar is always "connected"
    //ui->videoPlayer->pause();

    connect(mediaObject(), SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
    mediaObject()->setTickInterval(50);

    ui->waveform->connect(this,
                          SIGNAL(scratchUpdated(bool, qint64, const SoundBuffer&)),
                          SLOT(updateWaveform(bool, qint64, const SoundBuffer&)));

    // further configuration
    ui->storyboard->setSeekOnDrag(true);

    // scratch should be big enough to fit a movie-long sound
    m_scratch = SoundBuffer( WtsAudio::msToSampleCount(mediaObject()->totalTime()) );
    m_scratch.setColor( Qt::red );

    // now setup dataPath and try to load files from there
    QDir movieDir = QFileInfo(path).dir();
    m_dataDir = QDir( movieDir.filePath( QFileInfo(path).completeBaseName() + ".data") );

    if (! m_dataDir.exists() ) { movieDir.mkdir( m_dataDir.dirName() ); }

    // this should happen before loadData so we know video size and have
    // access to thumbnails
    if (m_videoFile) delete m_videoFile;
    m_videoFile = new VideoFile(path, this);
    ui->storyboard->setVideoSize(m_videoFile->width(), m_videoFile->height());

    loadData();

    emit loaded();
}

void MainWindow::resetData()
{
    m_dataDir.setPath("");;
    m_scratchInsertTime = 0;
    m_scratch = SoundBuffer();
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

    QFile dataFile( m_dataDir.filePath("metadata.xml") );
    dataFile.open(QFile::WriteOnly | QFile::Text);

    QXmlStreamWriter xml( &dataFile );

    xml.setAutoFormatting(true);

    xml.writeStartDocument();
    xml.writeDTD("<!DOCTYPE soundtrack>");
    xml.writeStartElement("soundtrack");
    xml.writeAttribute("version", "1.0");

    xml.writeStartElement("storyboard");
    foreach(Marker * m, m_markers) {
        xml.writeStartElement("marker");
        QString ms;
        ms.setNum(m->at());
        xml.writeAttribute("ms",ms);
        xml.writeAttribute("type", m->type() == SCENE ? "scene" : "event");
        xml.writeEndElement();
    }
    xml.writeEndElement();

    xml.writeStartElement("tension");
    ui->tension->saveData(xml);
    xml.writeEndElement();

    xml.writeStartElement("score");
    ui->score->saveData(xml);
    xml.writeEndElement();

    xml.writeStartElement("sequence");
    xml.writeAttribute("counter",QString("%1").arg(m_lastSampleNameNum));
    foreach(WtsAudio::BufferAt * buffer, m_sequence) {
        xml.writeStartElement("sample");
        xml.writeAttribute("ms",QString("%1").arg(buffer->at()));
        xml.writeAttribute("id",buffer->buffer()->name());
        xml.writeEndElement();

        QFile wav( m_dataDir.filePath( buffer->buffer()->name() ));
        buffer->buffer()->save(wav);
    }

    xml.writeEndElement();
    xml.writeEndDocument();
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
                while(xml.readNextStartElement()) {
                    addMarker(xml.attributes().value("type") == "scene" ? SCENE : EVENT,
                              xml.attributes().value("ms").toString().toLongLong());

                    // this is one way to read a flat list of <foo/> not recursing
                    xml.readElementText();
                }
            } else if (xml.name() == "tension") {
                ui->tension->loadData(xml);
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

                    WtsAudio::BufferAt * buffer =
                            new WtsAudio::BufferAt(sb,
                                                   xml.attributes().value("ms").toString().toLongLong(),
                                                   this);
                    m_sequence.append( buffer );

                    // find out original number / color
                    if (idRe.indexIn(id) > -1) {
                        int color_index = idRe.cap(1).toInt();
                        buffer->buffer()->setColor( Rainbow::getColor( color_index ) );
                    } else {
                        qDebug() << "Color index didn't parse...";
                    }

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

MainWindow::~MainWindow()
{
    // this is necessary to ensure widgets won't get tick'ed after player is deaded
    ui->videoPlayer->stop();
    delete ui;
}

Phonon::MediaObject * MainWindow::mediaObject()
{
    Q_ASSERT(ui!=NULL);
    Q_ASSERT(ui->videoPlayer);
    return ui->videoPlayer->mediaObject();
}

void MainWindow::setFullscreen(bool fs) {
    ui->actionFullscreen->setChecked(fs);
    if (fs)
        showFullScreen();
    else
        showNormal();
}

void MainWindow::onPlay(bool play)
{
    if (play) {
        emit samplerClear();
        m_audio.start();
        qSort(m_sequence.begin(), m_sequence.end(), WtsAudio::startsBefore);
        m_sequenceCursor = m_sequence.begin();
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
    if (record) {
        m_scratch.setWritePos(0);
        m_scratch.setColor(Qt::red);
    } else {
        WtsAudio::BufferAt * newBuff = new WtsAudio::BufferAt(
                    new SoundBuffer(makeSampleName(), m_scratch, m_scratch.m_writePos),
                    m_scratchInsertTime,
                    this);
        newBuff->buffer()->setColor( Rainbow::getColor(m_lastSampleNameNum) );
        m_sequence.append(newBuff);
        emit scratchUpdated(false, 0, m_scratch);
        emit newBufferAt(newBuff);
        ui->videoPlayer->seek(m_scratchInsertTime);
        saveData();
    }
}

void MainWindow::tick(qint64 ms)
{
    // FIXME shouldn't this happen AFTER scheduling new samples?
    // sync with the sampler
    emit samplerClock(ms);

    if (ui->actionRecord->isChecked()) {
        // recording
        if (m_scratch.m_writePos == 0)
            m_scratchInsertTime = ms;
        if ( m_audio.capture(&m_scratch) > 0) {
            emit scratchUpdated(true, m_scratchInsertTime, m_scratch);

            if (m_scratch.freeToWrite() == 0) {
                ui->actionRecord->setChecked(false);
            }
        }
    }

    // find out which samples to trigger
    if (ui->actionPlay->isChecked()) {
        while( m_sequenceCursor != m_sequence.end() && (*m_sequenceCursor)->at() <= ms ) {
            emit samplerSchedule( *m_sequenceCursor );
            m_sequenceCursor++;
        }
    }
}

void MainWindow::addMarker(MarkerType type, qint64 when)
{
    if (when < 0)
        when = mediaObject()->currentTime();
    m_markers[when] = new Marker(type, when, this);
    // load frameshot...
    m_videoFile->seek(when);
    m_markers[when]->setSnapshot( QPixmap::fromImage(m_videoFile->frame()) );

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

void MainWindow::loadToScratch(WtsAudio::BufferAt * bufferAt)
{
    m_scratch.m_writePos = 0;
    m_scratch.paste(bufferAt->buffer());
    m_scratch.setColor(bufferAt->buffer()->color());
    emit scratchUpdated(false, bufferAt->at(), m_scratch);
}

void MainWindow::exportMovie()
{
    // setting the parent to this makes it a "sheet" dialog on OSX
    QProgressDialog progress("Opslaan...", "Stop", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);

    m_exporter->configure(m_dataDir.filePath("export.mov"),
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

    selector->addTransition(this,SIGNAL(loaded()), firstPlay);
    connect(selector, SIGNAL(exited()), ui->movieSelectorScrollArea, SLOT(hide()));

    firstPlay->addTransition(this, SIGNAL(stopped()), m_workshop);
    connect(firstPlay, SIGNAL(entered()), ui->videoPlayer, SLOT(show()));
    connect(firstPlay, SIGNAL(entered()), ui->actionPlay, SLOT(toggle()));

    connect(m_workshop, SIGNAL(entered()), ui->toolBar, SLOT(show()));

    m_machine.setGlobalRestorePolicy(QStateMachine::RestoreProperties);
    m_machine.addState(firstPlay);
    m_machine.addState(m_workshop);
    m_machine.addState(selector);
    m_machine.setInitialState(selector);

    // first hide all
    ui->videoPlayer->hide();
    ui->toolBar->hide();
    ui->storyboard->hide();
    ui->timeLine->hide();
    ui->recorder->hide();
    ui->tension->hide();
    ui->score->hide();
    // ...
    ui->soundBank->hide();

    m_tabActions = new QActionGroup(this);

    addPage("1", QList<QWidget*>()
            << ui->storyboard,
            QList<QAction*>()
            << ui->actionAddMarker
            << ui->actionAddScene);
    QState * tensionPage =
            addPage("2",  QList<QWidget*>()
                    << ui->storyboard
                    << ui->tension);
    connect(tensionPage, SIGNAL(entered()), SLOT(maybeInitTension()));
    addPage("3", QList<QWidget*>()
            << ui->storyboard
            << ui->score);
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
    QGridLayout * layout = new QGridLayout;
    ui->movieSelector->setLayout(layout);
    QDir movDir(QCoreApplication::applicationDirPath () + "/../../../movie");

    QSignalMapper * mapper = new QSignalMapper(this);

    QFileInfoList movList = movDir.entryInfoList(QStringList() << "*.mov");

    int minColWidth = 100, maxCols = width() / minColWidth;
    int cols = (movList.size()>3) ? (int)ceilf( sqrtf( (float) movList.size() ) ) : 1;
    if (cols > maxCols)
        cols = maxCols;

    QSize iconSize = size() / cols;

    int i = 0;
    foreach(QFileInfo fi, movList) {
        QPushButton * button = new QPushButton( );
        layout->addWidget( button, i/cols, i%cols );
        i++;
        button->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
        mapper->setMapping( button, fi.absoluteFilePath() );
        connect(button, SIGNAL(clicked()), mapper, SLOT(map()));

        VideoFile vf(fi.absoluteFilePath());
        vf.seek( vf.duration()/3 );
        QPixmap thumb = QPixmap::fromImage( vf.frame().scaled(iconSize,Qt::KeepAspectRatio) );

        qint64 tt = vf.duration(), min = tt / 60000, sec = tt / 1000 % 60;
        button->setToolTip(QString("%1\n%2x%3\n%4:%5").arg(fi.fileName()).arg(vf.width()).arg(vf.height())
                           .arg(min).arg(sec,2,10,QLatin1Char('0')) );
        button->setIcon(QIcon(thumb));
        button->setIconSize(iconSize);
    }

    connect(mapper, SIGNAL(mapped(QString)), SLOT(loadMovie(QString)));
}

void MainWindow::maybeInitTension()
{
    if (ui->tension->isEdited())
        return;

    float level = 0.5;
    QMapIterator<qint64, Marker *> iter(m_markers);

    QPainterPath curve;
    bool init = true;
    while(iter.hasNext()){
        iter.next();
        Marker * m = iter.value();
        if (m->type() == SCENE) {
            level = 0.5;
        } else {
            level = 0.5 * level;
        }

        float x = (float)ui->tension->sceneRect().width()
                * m->at() / m_videoFile->duration();

        if (init) {
            curve.moveTo(QPointF(x, level));
            init = false;
        } else
            curve.lineTo( QPointF(x, level) );
    }

    curve.lineTo( QPointF(ui->tension->sceneRect().width(), 0.5));

    ui->tension->setCurve(curve);
}

QPainterPath MainWindow::tensionCurve() const
{
    return ui->tension->curve();
}

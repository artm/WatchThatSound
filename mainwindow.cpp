#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "rainbow.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_lastSampleNameNum(0)
{
    ui->setupUi(this);

    // connect to the sampler
    connect(this, SIGNAL(samplerClear()), &m_audio, SLOT(samplerClear()));
    connect(this, SIGNAL(samplerClock(qint64)), &m_audio, SLOT(samplerClock(qint64)));
    connect(this, SIGNAL(samplerSchedule(WtsAudio::BufferAt*)), &m_audio, SLOT(samplerSchedule(WtsAudio::BufferAt*)));

    // FIXME these are bits in progress....
    ui->tension->hide();
    ui->score->hide();
    ui->soundBank->hide();

    loadMovie(QCoreApplication::applicationDirPath () + "/../../../movie/edje.mov");
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
    ui->videoPlayer->load(Phonon::MediaSource(path));
    ui->videoPlayer->setVolume(0);

    // we need to be in pause so the seek bar is always "connected"
    ui->videoPlayer->pause();

    connect(mediaObject(), SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
    mediaObject()->setTickInterval(50);

    ui->waveform->connect(this,SIGNAL(scratchUpdated(bool,qint64,const SoundBuffer&)),SLOT(updateWaveform(bool,qint64,const SoundBuffer&)));

    // further configuration
    ui->storyboard->setSeekOnDrag(true);

    // scratch should be big enough to fit a movie-long sound
    m_scratch = SoundBuffer( WtsAudio::msToSampleCount(mediaObject()->totalTime()));
    m_scratch.setColor( Qt::red );

    // now setup dataPath and try to load files from there
    QDir movieDir = QFileInfo(path).dir();
    m_dataDir = QDir( movieDir.filePath( QFileInfo(path).completeBaseName() + ".data" ) );

    if (! m_dataDir.exists() ) {
        movieDir.mkdir( m_dataDir.dirName() );
    }

    loadData();
}

void MainWindow::saveData()
{
    QFile dataFile( m_dataDir.filePath("metadata.xml") );
    dataFile.open(QFile::WriteOnly | QFile::Text);

    QXmlStreamWriter xml( &dataFile );

    xml.setAutoFormatting(true);

    xml.writeStartDocument();
    xml.writeDTD("<!DOCTYPE soundtrack>");
    xml.writeStartElement("soundtrack");
    xml.writeAttribute("version", "1.0");

    xml.writeStartElement("storyboard");
    foreach(Marker m, m_markers) {
        xml.writeStartElement("marker");
        QString ms;
        ms.setNum(m.ms);
        xml.writeAttribute("ms",ms);
        xml.writeAttribute("type", m.type == SCENE ? "scene" : "event");
        xml.writeEndElement();
    }
    xml.writeEndElement();

    xml.writeStartElement("sequence");
    xml.writeAttribute("counter",QString("%1").arg(m_lastSampleNameNum));
    foreach(WtsAudio::BufferAt * buffer, m_sequence) {
        xml.writeStartElement("sample");
        xml.writeAttribute("ms",QString("%1").arg(buffer->m_at));
        xml.writeAttribute("id",buffer->m_buffer->name());
        xml.writeEndElement();

        QFile wav( m_dataDir.filePath( buffer->m_buffer->name() ));
        buffer->m_buffer->save(wav);
    }

    xml.writeEndElement();
    xml.writeEndDocument();
}

void MainWindow::loadData()
{
    // FIXME reset project first
    QFile dataFile( m_dataDir.filePath("metadata.xml") );
    dataFile.open(QFile::ReadOnly | QFile::Text);

    QRegExp idRe = QRegExp("sample_(\\d+)");

    QXmlStreamReader xml( &dataFile );
    xml.readNextStartElement();
    if (xml.name() == "soundtrack") {
        while (xml.readNextStartElement()) {
            if (xml.name() == "storyboard") {
                while(xml.readNextStartElement()) {
                    Marker m( xml.attributes().value("type") == "scene" ? SCENE : EVENT,
                              xml.attributes().value("ms").toString().toLongLong() );
                    m_markers[m.ms] = m;

                    // this is one way to read a flat list of <foo/> not recursing
                    xml.readElementText();
                }
            } else if (xml.name() == "sequence") {
                m_lastSampleNameNum = xml.attributes().value("counter").toString().toInt();
                while(xml.readNextStartElement()) {
                    WtsAudio::BufferAt * buffer = new WtsAudio::BufferAt();
                    QString id = xml.attributes().value("id").toString();
                    QFile wav( m_dataDir.filePath( id ));
                    buffer->m_buffer = new SoundBuffer();
                    buffer->m_buffer->load(wav);

                    // find out original number / color
                    if (idRe.indexIn(id) > -1) {
                        int color_index = idRe.cap(1).toInt();
                        buffer->m_buffer->setColor( Rainbow::getColor( color_index ) );
                    } else {
                        qDebug() << "Color index didn't parse...";
                    }

                    buffer->m_at = xml.attributes().value("ms").toString().toLongLong();
                    m_sequence.append( buffer );
                    emit newBufferAt(buffer);
                    // finish off the element...
                    xml.readElementText();
                }
            }
        }
    } else {
        qDebug() << dataFile.fileName() << " is not a soundtrack file";
    }
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
        WtsAudio::BufferAt * newBuff = new WtsAudio::BufferAt( new SoundBuffer(makeSampleName(), m_scratch, m_scratch.m_writePos), m_sampleSyncMs);
        newBuff->m_buffer->setColor( Rainbow::getColor(m_lastSampleNameNum) );
        m_sequence.append(newBuff);
        emit scratchUpdated(false, 0, m_scratch);
        emit newBufferAt(newBuff);
        ui->videoPlayer->seek(m_sampleSyncMs);
        saveData();
    }
}

void MainWindow::tick(qint64 ms)
{
    // sync with the sampler
    emit samplerClock(ms);

    if (ui->actionRecord->isChecked()) {
        // recording
        if (m_scratch.m_writePos == 0)
            m_sampleSyncMs = ms;
        if ( m_audio.capture(&m_scratch) > 0) {
            emit scratchUpdated(true, m_sampleSyncMs, m_scratch);

            if (m_scratch.freeToWrite() == 0) {
                ui->actionRecord->setChecked(false);
            }
        }
    }

    // find out which samples to trigger
    if (ui->actionPlay->isChecked()) {
        while( m_sequenceCursor != m_sequence.end() && (*m_sequenceCursor)->m_at <= ms ) {
            emit samplerSchedule( *m_sequenceCursor );
            m_sequenceCursor++;
        }
    }
}

void MainWindow::addScene()
{
    qint64 now = ui->videoPlayer->mediaObject()->currentTime();
    m_markers[now] = Marker(SCENE, now);
    emit storyBoardChanged();
    saveData();
}

void MainWindow::addMarker()
{
    qint64 now = ui->videoPlayer->mediaObject()->currentTime();
    m_markers[now] = Marker(EVENT, now);
    emit storyBoardChanged();
    saveData();
}

QList<MainWindow::Marker> MainWindow::getMarkers(MarkerType type) const
{
    QList<Marker> scenes;
    foreach(Marker m, m_markers) {
        if (type == ANY || m.type == type) {
            scenes.append(m);
        }
    }

    return scenes;
}

void MainWindow::loadToScratch(WtsAudio::BufferAt * bufferAt)
{
    m_scratch.m_writePos = 0;
    m_scratch.paste(bufferAt->m_buffer);
    m_scratch.setColor(bufferAt->m_buffer->color());
    emit scratchUpdated(false, bufferAt->m_at, m_scratch);
}

#include "wts_version.h"

#include "Project.h"
#include "Common.h"
#include "SoundBuffer.h"
#include "Rainbow.h"

using namespace WTS;

Project::Project(const QString& path, QObject * parent)
    : QObject(parent)
{
    setup();

    m_videoFile = new VideoFile(path, this);

    QDir movieDir = QFileInfo(path).dir();
    m_dataDir = QDir( movieDir.filePath( QFileInfo(path).completeBaseName()
                + ".data") );
    if (! m_dataDir.exists() )
        movieDir.mkdir( m_dataDir.dirName() );
}

Project::Project(QObject * parent)
    : QObject(parent)
{
    setup();
}

void Project::setup()
{
    m_loading = false;
    m_finalTension = 0.5;
    m_videoFile = NULL;
    m_lastSampleNameNum = 0;

    // connect to itself...
    connect(this,SIGNAL(saveSection(QXmlStreamWriter&)),
            SLOT(saveSequence(QXmlStreamWriter&)));
    connect(this,SIGNAL(saveSection(QXmlStreamWriter&)),
            SLOT(saveStoryboard(QXmlStreamWriter&)));
    connect(this,SIGNAL(loadSection(QXmlStreamReader&)),
            SLOT(loadSequence(QXmlStreamReader&)));
    connect(this,SIGNAL(loadSection(QXmlStreamReader&)),
            SLOT(loadStoryboard(QXmlStreamReader&)));

}

void Project::saveStoryboard(QXmlStreamWriter& xml)
{
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
}

bool Project::loadStoryboard(QXmlStreamReader& xml)
{
    if (xml.name() != "storyboard")
        return false;

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

    emit tensionChanged();
    return true;
}

void Project::saveSequence(QXmlStreamWriter& xml)
{
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

        buffer->buffer()->save( samplePath( buffer->buffer() ) );
    }
    xml.writeEndElement();
}

bool Project::loadSequence(QXmlStreamReader& xml)
{
    if (xml.name() != "sequence")
        return false;

    QRegExp idRe = QRegExp("sample_(\\d+)");
    m_lastSampleNameNum =
        xml.attributes().value("counter").toString().toInt();
    while(xml.readNextStartElement()) {
        QString id = xml.attributes().value("id").toString();

        SoundBuffer * sb = new SoundBuffer();
        try {
            sb->load( samplePath( id ) );
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
        } catch (SoundBuffer::FileNotFoundError& e) {
            delete sb;
        }
        // finish off the element...
        xml.readElementText();
    }
    return true;
}

void Project::addMarker(Project::MarkerType type, qint64 when, float tension)
{
    m_markers[when] = new Marker(type, when, this);
    m_markers[when]->setTension( tension );
    m_videoFile->seek(when);
    m_markers[when]->setSnapshot( QPixmap::fromImage(m_videoFile->frame() ) );

    emit storyBoardChanged();
    emit tensionChanged();

    save();
}

QList<Project::Marker *> Project::getMarkers(MarkerType type, bool forward) const
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

void Project::removeMarkerAt(quint64 at)
{
    if (at > 0) {
        m_markers.remove(at);
        emit storyBoardChanged();
        emit tensionChanged();
        save();
    }
}

QPainterPath Project::tensionCurve(float width)
{
    QPainterPath curve;
    QMapIterator<qint64, Project::Marker *> iter(m_markers);
    bool init = true;
    while(iter.hasNext()){
        iter.next();
        Project::Marker * m = iter.value();

        float x = width * m->at() / duration();

        if (init) {
            curve.moveTo(QPointF(x, m->tension()));
            init = false;
        } else
            curve.lineTo( QPointF(x, m->tension()) );
    }

    curve.lineTo( QPointF(width, finalTension()));

    return curve;
}

void Project::setMarkerTension(int markerIndex, float tension)
{
    if (markerIndex < m_markers.size() )
        m_markers[ m_markers.keys()[markerIndex] ]->setTension(tension);
    else if (markerIndex  == m_markers.size())
        setFinalTension(tension);
    emit tensionChanged();
}

void Project::removalRequested(Synced *synced)
{
    {
        Marker * marker = dynamic_cast<Marker*>(synced);
        if (marker) {
            removeMarkerAt(marker->at());
            return;
        }
    }

    {
        WtsAudio::BufferAt * buffer = dynamic_cast<WtsAudio::BufferAt *>(synced);
        if (buffer) {
            removeBufferAt(buffer);
            return;
        }
    }

    qWarning("Requested removal of unknown synced type");
}

void Project::save()
{
    // make sure we don't overwrite while loading...
    if (m_loading)
        return;

    QFile dataFile( dataDir().filePath("metadata.xml.tmp") );
    dataFile.open(QFile::WriteOnly | QFile::Text);

    QXmlStreamWriter xml( &dataFile );

    xml.setAutoFormatting(true);

    xml.writeStartDocument();
    xml.writeDTD("<!DOCTYPE soundtrack>");
    xml.writeStartElement("soundtrack");
    xml.writeAttribute("version", WTS_VERSION);
    xml.writeAttribute("movie", movieFilename());

    // FIXME temporary solution for contaminated views
    emit saveSection(xml);


    xml.writeEndElement();
    xml.writeEndDocument();
    dataFile.close();
    QFile::remove( dataDir().filePath("metadata.xml") );
    dataFile.rename(  dataDir().filePath("metadata.xml") );
}

void Project::load()
{
    m_loading = true;

    // always have a scene starting at 0
    addMarker(Project::SCENE, 0, 0.5);

    QFile dataFile( dataDir().filePath("metadata.xml") );
    dataFile.open(QFile::ReadOnly | QFile::Text);
    QXmlStreamReader xml( &dataFile );
    xml.readNextStartElement();
    if (xml.name() == "soundtrack") {
        while (xml.readNextStartElement()) {
            // try every section (a bit stupid, but simple to implement)
            // the following calls ATTEMPT to load, or do nothing if it's not their sections
            emit loadSection(xml);
        }

    } else {
        qDebug() << dataFile.fileName() << " is not a soundtrack file";
    }

    m_loading = false;
}

#if defined(__APPLE__)
#define nextToExe(p) QCoreApplication::applicationDirPath () + "/../../.." + p
#else
#define nextToExe(p) QCoreApplication::applicationDirPath () + p
#endif

QDir Project::s_movDir;
bool Project::s_movDirFound = false;

QDir Project::movDir()
{
    if (!s_movDirFound) {

        QString stdMoviesPath = QDesktopServices::storageLocation(
                QDesktopServices::MoviesLocation );

        s_movDir = QDir(stdMoviesPath).filePath("Watch That Sound Movies");

        if (!s_movDir.exists()) {
            // first try pre 3.0.3 convention

            QDir tryDir;
            tryDir = QDir(nextToExe("/WTSmovie"));
            if (!tryDir.exists())
                // then try 3-beta convention
                tryDir = QDir(nextToExe("/movie"));

            // upgrade from beta to actual version
            if (tryDir.exists()) {
                QDir().rename(tryDir.path(),s_movDir.path());

                QString info =
                    QString("Oude movie map %1 verplaatst naar de nieuwe locatie: %2")
                    .arg(tryDir.path()).arg(s_movDir.path());

                QMessageBox message(QMessageBox::Information, "Upgrade info",
                        info, QMessageBox::Ok);
                message.exec();
            } else {
                s_movDir.mkdir(s_movDir.path());
                // see if we have sample films installed
                QDir distVideos = QDir(nextToExe("/../video"));
                if (distVideos.exists()) {
                    foreach(QString path,
                            distVideos.entryList(QStringList()
                                << QString("*.%1").arg(VIDEO_FMT))) {
                        // copy sample films to movDir
                        qDebug() << "cp " << path << " to " << s_movDir.filePath(path);
                        QFile(distVideos.filePath(path)).copy(s_movDir.filePath(path));
                    }
                } else {
                    qWarning() << "No dist videos at " << distVideos.path();
                }
            }
        }
    }

    return s_movDir;
}

void Project::addBufferAt(WtsAudio::BufferAt * newBuff)
{
    m_sequence.append(newBuff);
    emit newBufferAt(newBuff);
}

WtsAudio::BufferAt *  Project::copyScratch(WtsAudio::BufferAt * scratch)
{
    WtsAudio::BufferAt * newBuff =
        new WtsAudio::BufferAt(
                new SoundBuffer(makeSampleName(),
                    *scratch->buffer(),
                    scratch->buffer()->m_writePos),
                scratch->at(),
                this);
    newBuff->buffer()->setColor( Rainbow::getColor(m_lastSampleNameNum) );
    newBuff->buffer()->initGains();
    addBufferAt(newBuff);
    return newBuff;
}

void Project::removeBufferAt(WtsAudio::BufferAt * bufferAt)
{
    m_sequence.removeAll(bufferAt);
    emit syncedItemRemoved(bufferAt);
    save();
}

QString Project::makeSampleName()
{
    return QString("sample_%1.wav").arg(++m_lastSampleNameNum);
}

void Project::reScanSamples()
{
    foreach(WtsAudio::BufferAt * bufat, m_sequence) {
        SoundBuffer * sample = bufat->buffer();
        // see if corresponding sample changed on disk...
        if ( sample->maybeReload( samplePath(sample) ) )
            emit sampleChanged(sample);
    }
}

void Project::openInExternalApp(SoundBuffer * buffer)
{
    QString path = samplePath(buffer);
    if (QFile(path).exists() &&
            QRegExp("\\.wav$").indexIn(path) > -1) {
        QDesktopServices::openUrl( QUrl( QString("file:///%0").arg(path)));
    }
}

QString Project::samplePath(SoundBuffer * sample)
{
    return  samplePath( sample->name() );
}

QString Project::samplePath(const QString& sampleName)
{
    return  m_dataDir.filePath( sampleName );
}


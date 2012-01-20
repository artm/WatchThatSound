#include "Project.h"
#include "Common.h"

using namespace WTS;

    Project::Project(const QString& path, QObject * parent)
    : QObject(parent)
      , m_finalTension(0.5)
      , m_videoFile(new VideoFile(path, this))
{}

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
    return true;
}

void Project::addMarker(Project::MarkerType type, qint64 when, float tension)
{
    m_markers[when] = new Marker(type, when, this);
    m_markers[when]->setTension( tension );
    m_videoFile->seek(when);
    m_markers[when]->setSnapshot( QPixmap::fromImage(m_videoFile->frame() ) );
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
    m_markers.remove(at);
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

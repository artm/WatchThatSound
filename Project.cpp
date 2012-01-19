#include "Project.h"

using namespace WTS;

    Project::Project(QObject * parent)
    : QObject(parent)
      , m_finalTension(0.5)
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
}

void Project::setMarkerSnapshot( quint64 when, const QPixmap& pixmap )
{
    if (m_markers.contains(when))
        m_markers[when]->setSnapshot( pixmap );
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

QPainterPath Project::tensionCurve(float width, quint64 duration)
{
    QPainterPath curve;
    QMapIterator<qint64, Project::Marker *> iter(m_markers);
    bool init = true;
    while(iter.hasNext()){
        iter.next();
        Project::Marker * m = iter.value();

        float x = width * m->at() / duration;

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

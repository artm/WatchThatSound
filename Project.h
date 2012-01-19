#pragma once
#include "Synced.h"

namespace WTS {

/** Project data container.
 * Eventually all project specific data should end up here.
 */
class Project : public QObject {
    Q_OBJECT
public:
    enum MarkerType {
        NONE,
        EVENT,
        SCENE,
        ANY,
    };

    class Marker : public WTS::Synced {
    protected:
        MarkerType m_type;
        QPixmap m_snapshot;

        float m_tension;

    public:
        Marker(MarkerType type, qint64 ms, QObject * parent)
            : WTS::Synced(ms, parent)
            , m_type(type)
            , m_tension(0.5)
        {}

        MarkerType type() const { return m_type; }
        const QPixmap& snapshot() const { return m_snapshot; }
        void setSnapshot(const QPixmap& snapshot) { m_snapshot = snapshot; }

        float tension() const { return m_tension; }
        void setTension(float value) { m_tension = value; }
    };

    Project(QObject * parent = 0);
    void saveStoryboard(QXmlStreamWriter&);
    bool loadStoryboard(QXmlStreamReader&);

    double finalTension() const { return m_finalTension; }
    void setFinalTension(double value) { m_finalTension = value; }
    void setMarkerTension(int markerIndex, float tension);
    void addMarker(MarkerType type, qint64 when, float tension);
    void removeMarkerAt(quint64 at);
    void setMarkerSnapshot( quint64 when, const QPixmap& pixmap );
    QList<Marker *> getMarkers(MarkerType type = ANY, bool forward = true) const;
    QPainterPath tensionCurve(float width, quint64 duration);
protected:
    double m_finalTension;
    QMap<qint64, Project::Marker *> m_markers;
};

}

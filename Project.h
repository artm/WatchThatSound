#pragma once

#include <QObject>

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
};

}

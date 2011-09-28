#ifndef SYNCED_H
#define SYNCED_H

#include <QObject>
#include <QMetaType>

namespace WTS {

class Synced : public QObject
{
    Q_OBJECT

public:
    Q_PROPERTY( qint64 at READ at WRITE setAt )

    explicit Synced(qint64 at, QObject *parent = 0);

    qint64 at() const { return m_at; }
    virtual void setAt(qint64 at) {
        m_at = at;
    }
    bool operator<(const Synced& other) const { return at() < other.at(); }

protected:
    qint64 m_at; // ms
};

} // namespace WTS

Q_DECLARE_METATYPE(WTS::Synced*)

#endif // SYNCED_H

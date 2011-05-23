#ifndef TIMELINEITEM_H
#define TIMELINEITEM_H

#include <QGraphicsItem>

namespace WTS {
class Synced;
}

class TimeLineItem : public QGraphicsItem
{
public:
    TimeLineItem(WTS::Synced * synced, QGraphicsScene * scene);

    WTS::Synced * synced() { return m_synced; }

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

protected:

    WTS::Synced * m_synced;
};

#endif // TIMELINEITEM_H

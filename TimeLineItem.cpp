#include <QGraphicsSceneMouseEvent>

#include "TimeLineItem.h"

TimeLineItem::TimeLineItem(WTS::Synced * synced, QGraphicsScene * scene)
    : QGraphicsItem(0, scene)
    , m_synced(synced)
{
    setFlags( QGraphicsItem::ItemIsSelectable );
}

void TimeLineItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (! boundingRect().contains(event->pos()) ) {
        event->ignore();
        clearFocus();
    } else  {
        event->accept();
        scene()->clearSelection();
        setSelected( true );
    }
}

QRectF TimeLineItem::boundingRect() const
{
    return childrenBoundingRect();
}

void TimeLineItem::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
}

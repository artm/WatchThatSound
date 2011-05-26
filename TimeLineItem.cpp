#include <QGraphicsSceneMouseEvent>

#include "TimeLineItem.h"
#include "TimeLineWidget.h"

using namespace WTS;

TimeLineItem::TimeLineItem(WTS::Synced * synced, QGraphicsScene * scene)
    : QGraphicsItem(0, scene)
    , m_synced(synced)
{
    setFlags( QGraphicsItem::ItemIsSelectable );
    TimeLineWidget::assignSynced(this, synced);
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

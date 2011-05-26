#include "BufferItem.h"
#include "SoundBuffer.h"
#include "TimeLineWidget.h"

using namespace WTS;

BufferItem::BufferItem(WtsAudio::BufferAt * buffer, qint64 duration)
    : QGraphicsRectItem()
    , m_duration(duration)
    , m_buffer(buffer)
    , m_constrain(false)
{
    setX( (qreal) m_buffer->at() / m_duration );
    setRect(0,0, (float)m_buffer->buffer()->duration() / m_duration, 0.3);
    setPen(Qt::NoPen);
    setBrush(buffer->buffer()->color());

    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable
             | QGraphicsItem::ItemSendsScenePositionChanges);

    TimeLineWidget::assignSynced(this, buffer);
}

void BufferItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (! boundingRect().contains(event->pos()) )
        event->ignore();
    else  {
        setFocus(Qt::MouseFocusReason);
        scene()->clearSelection();
        setSelected( true );
        m_constrain = true;
    }
    QGraphicsRectItem::mousePressEvent(event);
}

QVariant BufferItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene()) {
        // value is the new position.
        QPointF newPos = value.toPointF();
        // constrain vertical motion
        if (m_constrain) newPos.setY(y());
        m_buffer->setAt( newPos.x() * m_duration );
        return newPos;
    }
    return QGraphicsItem::itemChange(change, value);
}

void BufferItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_constrain = false;
    QGraphicsRectItem::mouseReleaseEvent(event);
}


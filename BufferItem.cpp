#include "BufferItem.h"
#include "SoundBuffer.h"
#include "TimeLineWidget.h"

#include <QGraphicsSceneMouseEvent>
#include <QPixmap>

using namespace WTS;

BufferItem::BufferItem(WtsAudio::BufferAt * buffer, qint64 duration, float height, QGraphicsView *view)
    : QGraphicsRectItem()
    , m_duration(duration)
    , m_buffer(buffer)
    , m_constrain(false)
    , m_view(view)
    , m_pixmap(new QGraphicsPixmapItem(this))
    , m_title(new QGraphicsSimpleTextItem(this))
{
    setX( (qreal) m_buffer->at() / m_duration );
    setRect(0,0, (float)m_buffer->buffer()->duration() / m_duration, height);
    setPen(Qt::NoPen);

    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable
             | QGraphicsItem::ItemSendsScenePositionChanges);

    TimeLineWidget::assignSynced(this, buffer);

    // title display
    m_title->setBrush( Qt::black );
    m_title->setFont( QFont("Helvetica", 10) );
    m_title->setFlags( QGraphicsItem::ItemIgnoresTransformations );
    m_title->setPos( 0, 1.1 * height );

    update();
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

void BufferItem::update()
{
    QRectF r = rect();
    r.setWidth( (float)m_buffer->buffer()->duration() / m_duration );
    setRect(r);

    // convert coordinates to pixels and see if pixmap needs to be updated
    QRect viewRect = m_view->mapFromScene(r).boundingRect();
    if (m_pixmap->pixmap().size() != viewRect.size()) {
        QPixmap pixmap(viewRect.size());
        buffer()->buffer()->draw( pixmap );
        m_pixmap->setPixmap(pixmap);
        m_pixmap->setVisible(true);

        // now we want the picture dimensions to be those of the parent item...
        qreal sx, sy;
        sx = r.width() / (qreal)m_pixmap->pixmap().width();
        sy = r.height() / (qreal)m_pixmap->pixmap().height();
        QTransform tr;
        tr.scale(sx, sy);
        m_pixmap->setTransform( tr );
    }

    // the title
    QString name = m_buffer->buffer()->name();
    name.replace(".wav","");
    m_title->setText( name );
}

void WTS::BufferItem::bufferChanged()
{
    update();
}

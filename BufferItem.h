#ifndef BUFFERITEM_H
#define BUFFERITEM_H

#include "WtsAudio.h"
#include <QGraphicsRectItem>
#include <QPointer>

class QGraphicsView;

namespace WTS {

class BufferItem : public QGraphicsRectItem
{
public:
    BufferItem(WtsAudio::BufferAt * buffer, qint64 duration, float height, QGraphicsView * view);
    WtsAudio::BufferAt * buffer() { return m_buffer; }
    void update();

    void bufferChanged();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    qint64 m_duration;
    WtsAudio::BufferAt * m_buffer;
    bool m_constrain;
    QGraphicsPixmapItem * m_pixmap;
    QGraphicsSimpleTextItem * m_title;
    QPointer<QGraphicsView> m_view;
};

}

#endif // BUFFERITEM_H

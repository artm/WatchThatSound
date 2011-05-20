#ifndef BUFFERITEM_H
#define BUFFERITEM_H

#include "wtsaudio.h"
#include <QGraphicsRectItem>

class BufferItem : public QGraphicsRectItem
{
public:
    BufferItem(WtsAudio::BufferAt * buffer, qint64 duration);
    WtsAudio::BufferAt * buffer() { return m_buffer; }
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    qint64 m_duration;
    WtsAudio::BufferAt * m_buffer;
    bool m_constrain;
};

#endif // BUFFERITEM_H

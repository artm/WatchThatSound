#ifndef SEQUENCERTIMELINE_H
#define SEQUENCERTIMELINE_H

#include "timelinewidget.h"
#include "soundbuffer.h"

class SequencerTimeLine : public TimeLineWidget
{
    Q_OBJECT
public:
    explicit SequencerTimeLine(QWidget *parent = 0);

signals:
    void bufferSelected(WtsAudio::BufferAt * buffer);

public slots:
    void insertBufferAt(WtsAudio::BufferAt * buffer);
    void showScratch(bool on, qint64 at, const SoundBuffer& scratch);

protected:
    QPen m_pen;
    QBrush m_brush;
    QGraphicsRectItem * m_scratchRect;

    QGraphicsItem * m_dragItem;
    QPointF m_dragLastP;

    QHash< QGraphicsItem *, WtsAudio::BufferAt * > m_itemToBuffer;
    QList< QGraphicsItem * > m_bufferItems;

    void restackItems();

    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseReleaseEvent ( QMouseEvent * event );
    virtual void mouseMoveEvent ( QMouseEvent * event );
};

#endif // SEQUENCERTIMELINE_H

#ifndef SEQUENCERTIMELINE_H
#define SEQUENCERTIMELINE_H

#include "timelinewidget.h"
#include "soundbuffer.h"

class BufferItem;

class SequencerTimeLine : public TimeLineWidget
{
    Q_OBJECT
public:
    explicit SequencerTimeLine(QWidget *parent = 0);

signals:
    void bufferSelected(WtsAudio::BufferAt * buffer);

public slots:
    void insertBufferAt(WtsAudio::BufferAt * buffer);
    void showScratch(WtsAudio::BufferAt * scratchAt, bool on);
    void updateBuffer(SoundBuffer * buffer);
    virtual void updateSelection();

protected:
    QPen m_pen;
    QBrush m_brush, m_muteBrush;
    QGraphicsRectItem * m_scratchRect;

    QGraphicsItem * m_dragItem;
    QPointF m_dragLastP;

    QList< BufferItem * > m_bufferItems;

    void restackItems();
    void showRange(QGraphicsItem * root, SoundBuffer * buffer);

    /*
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseMoveEvent ( QMouseEvent * event );
    */
    virtual void mouseReleaseEvent ( QMouseEvent * event );
};

#endif // SEQUENCERTIMELINE_H

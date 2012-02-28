#ifndef SEQUENCERTIMELINE_H
#define SEQUENCERTIMELINE_H

#include "TimeLineWidget.h"
#include "SoundBuffer.h"

namespace WTS {

class BufferItem;

class SequencerTimeLine : public TimeLineWidget
{
    Q_OBJECT
public:
    explicit SequencerTimeLine(QWidget *parent = 0);
    WtsAudio::BufferAt * selectedBufferAt();

signals:
    void bufferSelected(WtsAudio::BufferAt * buffer);
    void startSolo( WtsAudio::BufferAt * );

public slots:
    void insertBufferAt(WtsAudio::BufferAt * buffer);
    void showScratch(WtsAudio::BufferAt * scratchAt, bool on);
    void updateBuffer(SoundBuffer * buffer);
    virtual void updateSelection();
    virtual void setProject(Project * project);
    void startSolo();

protected:
    QPen m_pen;
    QBrush m_brush, m_muteBrush;
    QGraphicsRectItem * m_scratchRect;

    QGraphicsItem * m_dragItem;
    QPointF m_dragLastP;

    QList< BufferItem * > m_bufferItems;

    float m_levelH, m_sampleH;

    void restackItems();
    void showRange(QGraphicsItem * root, SoundBuffer * buffer);
    virtual void onRemoved(WTS::Synced * synced, QGraphicsItem * item);

    virtual void mouseReleaseEvent ( QMouseEvent * event );
    virtual void resizeEvent ( QResizeEvent * event );
};

}

#endif // SEQUENCERTIMELINE_H

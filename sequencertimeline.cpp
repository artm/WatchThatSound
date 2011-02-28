#include "sequencertimeline.h"
#include <QMultiMap>
#include <QMouseEvent>

SequencerTimeLine::SequencerTimeLine(QWidget *parent) :
        TimeLineWidget(parent), m_pen(Qt::black), m_brush(Qt::white), m_dragItem(0)
{
    connect(mainWindow,SIGNAL(newBufferAt(WtsAudio::BufferAt*)),SLOT(insertBufferAt(WtsAudio::BufferAt*)));
    connect(mainWindow,SIGNAL(scratchUpdated(bool,qint64,const SoundBuffer&)),SLOT(showScratch(bool,qint64,const SoundBuffer&)));
    mainWindow->connect(this, SIGNAL(bufferSelected(WtsAudio::BufferAt*)),SLOT(loadToScratch(WtsAudio::BufferAt *)));

    QBrush scratchBrush(QColor(255,200,200,127));
    m_scratchRect = scene()->addRect(0,0,0,0);
    m_scratchRect->setBrush(scratchBrush);
    m_scratchRect->hide();
    m_scratchRect->setZValue(1.0);

    setSeekOnDrag(true);
}

void SequencerTimeLine::insertBufferAt(WtsAudio::BufferAt * buffer)
{
    QBrush brush(buffer->m_buffer->color());
    float tt = (float)mainWindow->mediaObject()->totalTime();
    float relX = (float)buffer->m_at / tt;
    float relW = (float)buffer->m_buffer->duration() / tt;
    QGraphicsItem * item = static_cast<QGraphicsItem*>(scene()->addRect(0, 0, relW, 0.3, m_pen, brush));
    item->moveBy(relX, 0);

    m_itemToBuffer[item] = buffer;
    m_bufferItems.append(item);

    restackItems();
}

bool lefterItem(const QGraphicsItem * a, const QGraphicsItem * b)
{ return a->x() < b->x(); }

void SequencerTimeLine::restackItems()
{
    QList<float> levelRight;
    qStableSort(m_bufferItems.begin(), m_bufferItems.end(), lefterItem);
    QList< QGraphicsItem * >::iterator it;
    for(it = m_bufferItems.begin(); it != m_bufferItems.end(); ++it) {
        QGraphicsItem * item = *it;
        int until = levelRight.size();
        for(int level = 0; level <= until ; ++level) {
            float x = item->x();
            if (level == until)
                levelRight.append( x );

            if (x >= levelRight[level]) {
                // insert on level
                item->setY(0.25 * (float) level);
                levelRight[level] = x + item->boundingRect().width();
                break;
            }
        }
    }
}

void SequencerTimeLine::showScratch(bool on, qint64 at, const SoundBuffer& scratch)
{
    if (on) {
        qint64 duration = WtsAudio::sampleCountToMs(scratch.m_writePos);

        float tt = (float)mainWindow->mediaObject()->totalTime();
        float relX = (float)at / tt;
        float relW = (float)duration / tt;
        m_scratchRect->setRect(relX, .1, relW, .8 );
        m_scratchRect->show();
    } else {
        m_scratchRect->hide();
    }
}

void SequencerTimeLine::mousePressEvent ( QMouseEvent * event )
{
    if (event->buttons() & Qt::LeftButton) {
        m_dragLastP = mapToScene(event->pos());
        m_dragItem  = itemAt( event->pos() );
        if (m_dragItem) {
            if (!m_itemToBuffer.contains(m_dragItem)) {
                m_dragItem = 0;
            } else {
                WtsAudio::BufferAt * buffer = m_itemToBuffer[m_dragItem];
                emit bufferSelected( buffer );
                mainWindow->seek( buffer->m_at );
                return;
            }
        }
    }
    TimeLineWidget::mousePressEvent( event );
}

void SequencerTimeLine::mouseReleaseEvent ( QMouseEvent * event )
{
    if (event->button() == Qt::LeftButton) {
        if (m_dragItem) {
           m_dragItem = 0;
           restackItems();
       }
    }
    mainWindow->saveData();
}

void SequencerTimeLine::mouseMoveEvent ( QMouseEvent * event )
{
    if (event->buttons() & Qt::LeftButton) {
        if (m_dragItem) {
            QPointF newPos = mapToScene(event->pos());
            float dx = newPos.x() - m_dragLastP.x();
            m_dragLastP = newPos;
            m_dragItem->moveBy(dx, 0);

            qint64 newTime = (float)mainWindow->mediaObject()->totalTime() * m_dragItem->x();
            mainWindow->seek( newTime );
            m_itemToBuffer[m_dragItem]->m_at = newTime;
            return;
        }
    }

    TimeLineWidget::mouseMoveEvent( event );
}

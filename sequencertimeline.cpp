#include "sequencertimeline.h"
#include <QMultiMap>
#include <QMouseEvent>

SequencerTimeLine::SequencerTimeLine(QWidget *parent)
    : TimeLineWidget(parent)
    , m_pen(Qt::black)
    , m_brush(Qt::white)
    , m_muteBrush(QColor(0,0,0,70))
    , m_dragItem(0)
{
    connect(m_mainWindow,SIGNAL(newBufferAt(WtsAudio::BufferAt*)),
            SLOT(insertBufferAt(WtsAudio::BufferAt*)));
    connect(m_mainWindow,SIGNAL(scratchUpdated(WtsAudio::BufferAt*,bool)),
            SLOT(showScratch(WtsAudio::BufferAt*,bool)));

    QBrush scratchBrush(QColor(255,200,200,127));
    m_scratchRect = scene()->addRect(0,0,0,0);
    m_scratchRect->setBrush(scratchBrush);
    m_scratchRect->hide();
    m_scratchRect->setZValue(1.0);

    setSeekOnDrag(true);
}

void SequencerTimeLine::insertBufferAt(WtsAudio::BufferAt * bufferAt)
{
    SoundBuffer * buffer = bufferAt->buffer();

    QBrush brush(buffer->color());
    float tt = (float)m_mainWindow->mediaObject()->totalTime();
    float relX = (float)bufferAt->at() / tt;
    float relW = (float)buffer->duration() / tt;
    QGraphicsItem * item =
            static_cast<QGraphicsItem*>(scene()->addRect(0, 0, relW, 0.3,
                                                         Qt::NoPen, brush));

    item->moveBy(relX, 0);
    showRange(item, buffer);

    m_itemToBuffer[item] = bufferAt;
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

void SequencerTimeLine::showScratch(WtsAudio::BufferAt * scratchAt, bool on)
{
    if (on) {
        qint64 at = scratchAt->at();
        qint64 duration = WtsAudio::sampleCountToMs(scratchAt->buffer()->m_writePos);

        float tt = (float)m_mainWindow->mediaObject()->totalTime();
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

        while( m_dragItem && !m_itemToBuffer.contains(m_dragItem) ) {
            m_dragItem = m_dragItem->parentItem();
        }

        if (m_dragItem) {
            WtsAudio::BufferAt * buffer = m_itemToBuffer[m_dragItem];
            emit bufferSelected( buffer );
            m_mainWindow->seek( buffer->at() + buffer->buffer()->rangeStart() );
            return;
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
    m_mainWindow->saveData();
}

void SequencerTimeLine::mouseMoveEvent ( QMouseEvent * event )
{
    if (event->buttons() & Qt::LeftButton) {
        if (m_dragItem) {
            QPointF newPos = mapToScene(event->pos());
            float dx = newPos.x() - m_dragLastP.x();
            m_dragLastP = newPos;
            m_dragItem->moveBy(dx, 0);

            qint64 newTime =
                    m_mainWindow->mediaObject()->currentTime() +
                    (float)m_mainWindow->mediaObject()->totalTime()
                    * dx;
            m_mainWindow->seek( newTime );
            WtsAudio::BufferAt * bufferAt = m_itemToBuffer[m_dragItem];

            bufferAt->setAt( newTime - bufferAt->buffer()->rangeStart() );

            return;
        }
    }

    TimeLineWidget::mouseMoveEvent( event );
}

void SequencerTimeLine::showRange(QGraphicsItem * root, SoundBuffer *buffer)
{
    float tt = (float)m_mainWindow->mediaObject()->totalTime();
    float relW = (float)buffer->duration() / tt;
    float selX1 = (float)buffer->rangeStart() / tt;
    float selX2 = (float)buffer->rangeEnd() / tt;

    scene()->addRect(
                0, 0, selX1, 0.3,
                Qt::NoPen, m_muteBrush )
            ->setParentItem(root);

    scene()->addRect(
                selX2, 0, relW-selX2, 0.3,
                Qt::NoPen, m_muteBrush )
            ->setParentItem(root);

    scene()->addRect(
                0, 0,
                relW, 0.3,
                m_pen, Qt::NoBrush )
            ->setParentItem(root);

}

void SequencerTimeLine::updateBuffer(SoundBuffer *buffer)
{
    QHashIterator< QGraphicsItem *, WtsAudio::BufferAt * > i(m_itemToBuffer);
    while(i.hasNext()) {
        i.next();
        if (i.value()->buffer() == buffer) {
            foreach(QGraphicsItem * child, i.key()->childItems()) {
                scene()->removeItem(child);
                delete child;
            }
            showRange(i.key(), buffer);
        }
    }
}

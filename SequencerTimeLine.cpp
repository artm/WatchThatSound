#include "SequencerTimeLine.h"
#include "BufferItem.h"

#include <QMultiMap>
#include <QMouseEvent>

using namespace WTS;

SequencerTimeLine::SequencerTimeLine(QWidget *parent)
    : TimeLineWidget(parent)
    , m_pen(Qt::black)
    , m_brush(Qt::white)
    , m_muteBrush(QColor(0,0,0,70))
    , m_dragItem(0)
    , m_levelH(0.15)
    , m_sampleH(0.17)
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

    BufferItem * item = new BufferItem(bufferAt, m_mainWindow->project()->duration(), m_sampleH);
    scene()->addItem(item);


    showRange(item, buffer);

    m_bufferItems.append(item);

    restackItems();
}

bool lefterItem(const QGraphicsItem * a, const QGraphicsItem * b)
{ return a->x() < b->x(); }

void SequencerTimeLine::restackItems()
{
    QList<float> levelRight;
    qStableSort(m_bufferItems.begin(), m_bufferItems.end(), lefterItem);
    QList< BufferItem * >::iterator it;
    for(it = m_bufferItems.begin(); it != m_bufferItems.end(); ++it) {
        QGraphicsItem * item = *it;
        int until = levelRight.size();
        float x = item->x();
        for(int level = 0; level <= until ; ++level) {
            if (level == until)
                levelRight.append( x );

            if (x >= levelRight[level]) {
                // insert on level
                item->setY(m_levelH * (float) level);
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

        float tt = (float)m_mainWindow->project()->duration();
        float relX = (float)at / tt;
        float relW = (float)duration / tt;
        m_scratchRect->setRect(relX, .1, relW, .8 );
        m_scratchRect->show();
    } else {
        m_scratchRect->hide();
    }
}

void SequencerTimeLine::mouseReleaseEvent ( QMouseEvent * event )
{
    TimeLineWidget::mouseReleaseEvent(event);
    restackItems();
    m_mainWindow->saveData();
}

void SequencerTimeLine::showRange(QGraphicsItem * root, SoundBuffer *buffer)
{
    float tt = (float)m_mainWindow->project()->duration();
    float relW = (float)buffer->duration() / tt;
    float selX1 = (float)WtsAudio::sampleCountToMs(buffer->rangeStart()) / tt;
    float selX2 = (float)WtsAudio::sampleCountToMs(buffer->rangeEnd()) / tt;

    scene()->addRect(
                0, 0, selX1, m_sampleH,
                Qt::NoPen, m_muteBrush )
            ->setParentItem(root);

    scene()->addRect(
                selX2, 0, relW-selX2, m_sampleH,
                Qt::NoPen, m_muteBrush )
            ->setParentItem(root);

    scene()->addRect(
                0, 0,
                relW, m_sampleH,
                m_pen, Qt::NoBrush )
            ->setParentItem(root);

}

void SequencerTimeLine::updateBuffer(SoundBuffer *buffer)
{
    foreach(BufferItem * bi, m_bufferItems) {

        if (bi->buffer()->buffer() == buffer) {
            scene()->clearSelection();

            foreach(QGraphicsItem * child, bi->childItems()) {
                scene()->removeItem(child);
                delete child;
            }
            showRange(bi, buffer);
            bi->setSelected(true);
        }
    }
}

void SequencerTimeLine::updateSelection()
{
    TimeLineWidget::updateSelection();

    QList<QGraphicsItem *> sel = scene()->selectedItems();
    if (sel.length() > 0) {
        BufferItem * bi = dynamic_cast<BufferItem*>(sel[0]);
        if (bi)
            emit bufferSelected( bi->buffer() );
    } else {
    }
}

void SequencerTimeLine::deleteSynced(QGraphicsItem * item, WTS::Synced * synced)
{
    WtsAudio::BufferAt * bufferAt = dynamic_cast<WtsAudio::BufferAt *>(synced);
    BufferItem * bi = dynamic_cast<BufferItem*>(item);
    Q_ASSERT( bufferAt );
    Q_ASSERT( bi );

    m_bufferItems.removeAll(bi);
    m_mainWindow->removeBuffer( bufferAt );
    restackItems();
}

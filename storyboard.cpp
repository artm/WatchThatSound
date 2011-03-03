#include "storyboard.h"
#include <cmath>

StoryBoard::StoryBoard(QWidget *parent)
    : TimeLineWidget(parent)
    , m_videoWidth(640)
    , m_videoHeight(360)
    , m_selectedThumb(0)
    , m_dragItem(0)
{
    disconnect(mainWindow, SIGNAL(storyBoardChanged()), this, SLOT(repaint()));
    connect(mainWindow,SIGNAL(storyBoardChanged()),SLOT(updateSnapshots()));
}

void StoryBoard::drawForeground ( QPainter * painter, const QRectF & rect )
{
    int maxLines = width() / 5;

    float totalMin = (float)mainWindow->mediaObject()->totalTime() / 60000.0f;
    float N[] = { 60, 30, 15, 12, 6, 5, 4, 3, 2 };
    int Ncount = sizeof(N)/sizeof(N[0]);

    int i;
    for(i = 0; i<(Ncount-1) && ((totalMin * N[i]) > maxLines) ; i++) {}
    float dx = 1.0f / (N[i] * totalMin);

    painter->setPen(QColor(50,60,50));
    int j0 = floor(rect.x() / dx), j1 = ceil(rect.right() / dx);
    for(int j = j0; j<j1; ++j) {
        float x = dx * j;

        int rem = j % (int)N[i];
        float y = 1.0 - (rem ? 0.5 * s_marginBottom : s_marginBottom);

        painter->drawLine(QPointF(x,y),QPointF(x,1.0));
    }

    TimeLineWidget::drawForeground(painter, rect);
}

void StoryBoard::updateSnapshots()
{
    float tt = mainWindow->mediaObject()->totalTime();
    scene()->clear();
    m_itemToMarker.clear();
    m_msToItem.clear();
    m_dragItem = 0;
    m_selectedThumb = 0;

    int n = 0;

    foreach(MainWindow::Marker m, mainWindow->getMarkers(MainWindow::ANY, false)) {
        float x = (float)m.m_ms / tt - 0.5 * m_thumbWidth;

        int gapCount = s_levelCount - 1;
        float y = s_marginY
                  // zig zag function
                  + fabs(n++ % (2*gapCount) - gapCount )
                  // fit centers between two margins + two halves
                  * (1.0 - s_marginY - s_marginBottom - m_thumbHeight) / (float)gapCount;

        QGraphicsItem * frameItem = scene()->addRect(
                QRectF(x - m_marginX,
                       y - s_marginY,
                       m_thumbWidth + 2.0*m_marginX,
                       m_thumbHeight + s_marginY*2.0),
                QPen(Qt::NoPen),QBrush(QColor(255,255,255)));
        QGraphicsPixmapItem * gpi = scene()->addPixmap( m.m_snapshot );
        gpi->setPos(x,y);
        gpi->scale(m_thumbScale/(float)width(), m_thumbScale/(float)height());
        gpi->setParentItem(frameItem);

        m_itemToMarker[frameItem] = m;
        m_msToItem[m.m_ms] = frameItem;
    }
}

void StoryBoard::resizeEvent ( QResizeEvent * event )
{
    TimeLineWidget::resizeEvent(event);

    m_thumbHeight = 0.55;
    float pixH = (float)height() * m_thumbHeight;
    float pixW = pixH * (float)m_videoWidth / (float)m_videoHeight;
    m_thumbWidth = pixW / (float)width();
    m_thumbScale = pixH/(float) m_videoHeight;
    m_marginX = s_marginY * (float)height() / (float)width();

    updateSnapshots();
}

void StoryBoard::mousePressEvent ( QMouseEvent * event )
{
    if (event->buttons() & Qt::LeftButton) {
        m_dragLastP = mapToScene(event->pos());
        m_dragItem  = itemAt( event->pos() );

        while (m_dragItem) {
            if (!m_itemToMarker.contains(m_dragItem)) {
                m_dragItem = m_dragItem->parentItem();
            } else {
                MainWindow::Marker m = m_itemToMarker[m_dragItem];
                mainWindow->seek( m.m_ms );
                event->accept();
                return;
            }
        }
    }
    TimeLineWidget::mousePressEvent( event );
}

/*
void StoryBoard::mouseReleaseEvent ( QMouseEvent * event )
{
    if (event->button() == Qt::LeftButton) {
        if (m_dragItem) {
           m_dragItem = 0;
           restackItems();
       }
    }
    mainWindow->saveData();
}
*/

void StoryBoard::mouseMoveEvent ( QMouseEvent * event )
{
    if (event->buttons() & Qt::LeftButton) {
        // FIXME: thumb dragging?
        /*
        if (m_dragItem) {
            QPointF newPos = mapToScene(event->pos());
            float dx = newPos.x() - m_dragLastP.x();
            m_dragLastP = newPos;
            m_dragItem->moveBy(dx, 0);

            qint64 newTime = (float)mainWindow->mediaObject()->totalTime() *
                             m_dragItem->x();
            mainWindow->seek( newTime );
            m_itemToBuffer[m_dragItem]->m_at = newTime;
            return;
        }
        */
        mousePressEvent(event);
        if (event->isAccepted())
            return;
    }

    TimeLineWidget::mouseMoveEvent( event );
}

void StoryBoard::setCurrentTime(qint64 time)
{
    TimeLineWidget::setCurrentTime(time);
    // find the last marker / item that we've passed
    QMap<qint64, QGraphicsItem *>::iterator iter = m_msToItem.upperBound(time);
    if (iter != m_msToItem.begin())
        iter--;
    QGraphicsItem * lastItem = iter.value();



    if (lastItem != m_selectedThumb) {

        if (m_selectedThumb) {
            m_selectedThumb->setZValue(0.0);
            QGraphicsRectItem * ri = dynamic_cast<QGraphicsRectItem *>(m_selectedThumb);
            if (ri)
                ri->setPen(Qt::NoPen);
        }

        m_selectedThumb = lastItem;
        m_selectedThumb->setZValue(1.0);
        QGraphicsRectItem * ri = dynamic_cast<QGraphicsRectItem *>(m_selectedThumb);
        if (ri) ri->setPen(QPen(Qt::blue));
    }
}

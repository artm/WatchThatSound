#include "storyboard.h"
#include <cmath>

StoryBoard::StoryBoard(QWidget *parent)
    : TimeLineWidget(parent)
    , m_videoWidth(640)
    , m_videoHeight(360)
    , m_selectedThumb(0)
    , m_dragItem(0)
{
    disconnect(m_mainWindow, SIGNAL(storyBoardChanged()), this, SLOT(repaint()));
    connect(m_mainWindow,SIGNAL(storyBoardChanged()),SLOT(updateSnapshots()));       

    QFrame * box = new QFrame();
    box->setFrameShadow( QFrame::Raised );
    box->setFrameShape( QFrame::Panel );
    QHBoxLayout * layout = new QHBoxLayout(box);
    layout->setMargin(0);
    layout->setSpacing(0);

    QPushButton * b = new QPushButton("<");
    b->setFlat(true);
    b->setMaximumSize(QSize(16,16));
    layout->addWidget(b);
    b = new QPushButton("x");
    b->setFlat(true);
    b->setMaximumSize(QSize(16,16));
    layout->addWidget(b);
    b = new QPushButton(">");
    b->setFlat(true);
    b->setMaximumSize(QSize(16,16));
    layout->addWidget(b);

    m_itemPopup = scene()->addWidget(box);
    m_itemPopup->setZValue(2.0);
    m_itemPopup->hide();
    m_popupsItem = 0;
}

void StoryBoard::drawBackground ( QPainter * painter, const QRectF & rect )
{
    TimeLineWidget::drawBackground(painter, rect);

    int maxLines = width() / 5;

    float totalMin = (float)m_mainWindow->mediaObject()->totalTime() / 60000.0f;
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

}

void StoryBoard::updateSnapshots()
{
    float tt = m_mainWindow->mediaObject()->totalTime();

    foreach(QGraphicsItem * item, m_msToItem) {
        scene()->removeItem(item);
    }
    m_itemToMarker.clear();
    m_msToItem.clear();
    m_dragItem = 0;
    m_selectedThumb = 0;

    int n = 0;

    foreach(MainWindow::Marker * m, m_mainWindow->getMarkers(MainWindow::ANY, false)) {
        float x = (float)m->at() / tt - 0.5 * m_thumbWidth;

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
                QPen(Qt::NoPen),QBrush(QColor(255,255,255,150)));
        QGraphicsPixmapItem * gpi = scene()->addPixmap( m->snapshot() );
        gpi->setPos(x,y);
        gpi->scale(m_thumbScale/(float)width(), m_thumbScale/(float)height());
        gpi->setParentItem(frameItem);

        m_itemToMarker[frameItem] = m;
        m_msToItem[m->at()] = frameItem;
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

    m_itemPopup->setScale(1.0);
    m_itemPopup->scale(1.0/(qreal)width(),1.0/(qreal)height());

    updateSnapshots();
}

void StoryBoard::mousePressEvent ( QMouseEvent * event )
{
    if (event->buttons() & Qt::LeftButton) {
        m_dragLastP = mapToScene(event->pos());
        m_dragItem  = itemAt( event->pos() );

        while (m_dragItem) {
            if (m_itemToMarker.contains(m_dragItem)) {
                MainWindow::Marker * m = m_itemToMarker[m_dragItem];
                m_mainWindow->seek( m->at() );
                return;
            }
            m_dragItem = m_dragItem->parentItem();
        }
    }
    TimeLineWidget::mousePressEvent( event );
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

void StoryBoard::showItemPopup( QGraphicsItem * item)
{
    if (m_popupsItem != item) {
        // first click on item: calculate new geometry then hide
        // have to "show" first to update the bounding rect
        m_itemPopup->show();
        m_itemPopup->setPos( item->boundingRect().bottomLeft() );

        QRectF g = m_itemPopup->geometry();

        // keep size to compensate for rounding errors
        QSizeF keepSize = g.size();
        qreal w = g.width() / width(), h = g.height() / height();
        g.setTop( std::min(1.0 - h, g.top()) );
        g.setLeft( item->boundingRect().right() - w );
        g.setSize(keepSize);

        m_itemPopup->setGeometry( g );

        // now hide
        m_itemPopup->hide();
        // remember the daddy
        m_popupsItem = item;
    } else {
        // not the first click on item - toggle
        m_itemPopup->setVisible( ! m_itemPopup->isVisible() );
    }
}

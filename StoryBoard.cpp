#include "StoryBoard.h"
#include <cmath>
#include <TimeLineItem.h>
#include <QHBoxLayout>
#include <QPushButton>

using namespace WTS;

StoryBoard::StoryBoard(QWidget *parent)
    : TimeLineWidget(parent)
    , m_videoWidth(640)
    , m_videoHeight(360)
    , m_selectedThumb(0)
    , m_dragItem(0)
{
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
}

void StoryBoard::drawBackground ( QPainter * painter, const QRectF & rect )
{
    TimeLineWidget::drawBackground(painter, rect);

    painter->setFont( QFont("Monaco", 9) );

    int maxLines = width() / 5;

    float totalMin = (float)m_mainWindow->duration() / 60000.0f;
    float N[] = { 60, 30, 15, 12, 6, 5, 4, 3, 2 };
    int Ncount = sizeof(N)/sizeof(N[0]);

    int i;
    for(i = 0; i<(Ncount-1) && ((totalMin * N[i]) > maxLines) ; ++i) {}
    float dx = 1.0f / (N[i] * totalMin);

    painter->setPen(QColor(50,60,50));
    for(int j = 0; j<maxLines; ++j) {
        float x = dx * j;

        int rem = j % (int)N[i];
        float y = 1.0 - (rem ? 0.5 * s_marginBottom : s_marginBottom);

        painter->drawLine(QPointF(x,y),QPointF(x,1.0));

        if (!rem) {
            QString text = QString("%1:00").arg( floor(j / N[i]) );

            double x_pix = x * width() + 3;
            double y_pix = y * height() - 2;

            painter->setMatrixEnabled(false);
            painter->drawText(QPointF(x_pix,y_pix), text);
            painter->setMatrixEnabled(true);
        }
    }

}

void StoryBoard::updateSnapshots()
{
    float tt = m_mainWindow->duration();

    foreach(QGraphicsItem * item, m_msToItem) {
        scene()->removeItem(item);
    }
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

        TimeLineItem * tli = new TimeLineItem(m, scene());
        tli->setEditModeOnly(false);

        QGraphicsItem * frameItem = scene()->addRect(
                QRectF(x - m_marginX,
                       y - s_marginY,
                       m_thumbWidth + 2.0*m_marginX,
                       m_thumbHeight + s_marginY*2.0),
                QPen(Qt::NoPen),QBrush(QColor(255,255,255,150)));
        frameItem->setParentItem(tli);

        QGraphicsPixmapItem * gpi = scene()->addPixmap( m->snapshot() );
        gpi->setPos(x,y);
        gpi->scale(m_thumbScale/(float)width(), m_thumbScale/(float)height());
        gpi->setParentItem(frameItem);

        m_msToItem[m->at()] = tli;
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

void StoryBoard::setCurrentTime(qint64 time)
{
    TimeLineWidget::setCurrentTime(time);

    if (m_msToItem.size() == 0)
        return; // no thumbs to select

    // find the last marker / item that we've passed
    QMap<qint64, QGraphicsItem *>::iterator iter = m_msToItem.upperBound(time);
    if (iter != m_msToItem.begin())
        iter--;
    QGraphicsItem * lastItem = iter.value();

    if (lastItem && lastItem != m_selectedThumb) {

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

void StoryBoard::deleteSynced(QGraphicsItem *, WTS::Synced *synced)
{
    MainWindow::Marker * m = dynamic_cast<MainWindow::Marker *>(synced);
    Q_ASSERT(m);
    m_mainWindow->removeMark(m);
}


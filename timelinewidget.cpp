#include "timelinewidget.h"
#include <QMouseEvent>

TimeLineWidget::TimeLineWidget(QWidget *parent) :
    QGraphicsView(parent), _seekOnDrag(false), m_currentTime(0)
{
    // find the mainWindow
    QObject * iter = parent;
    while(iter && !iter->inherits("MainWindow"))
        iter = iter->parent();
    if (!iter)
        qFatal("TimeLineWidget should be descendant of MainWindow in view hierarchy");
    mainWindow = qobject_cast<MainWindow *>(iter);

    connect(mainWindow,SIGNAL(storyBoardChanged()),SLOT(update()));
    connect(mainWindow,SIGNAL(samplerClock(qint64)),SLOT(setCurrentTime(qint64)));

    setScene(new QGraphicsScene(0.0,0.0,1.0,1.0,this));
}

void TimeLineWidget::resizeEvent ( QResizeEvent * /*event*/ )
{
    fitInView(0,0,1.0,1.0);
}

void TimeLineWidget::setCurrentTime(qint64 time)
{
    Phonon::MediaObject * mo = mainWindow->mediaObject();
    if (!mo) return;

    if (time != m_currentTime) {
        // erase old cursor
        qint64 x = m_currentTime * width() / mo->totalTime();
        update( (int) x - 3, 0, (int) x + 3, height() );

        m_currentTime = time;

        // draw new cursor
        x = m_currentTime * width() / mo->totalTime();
        update( (int)x - 1, 0, (int) x + 1, height() );
    }
}

void TimeLineWidget::paintRange(QPainter * painter, qreal x, qreal w, const QColor& c)
{
    QRectF r(x, 0, w, 1);
    painter->setPen(c);
    painter->drawRect( r );
    painter->fillRect( r, QBrush(c) );
}

void TimeLineWidget::drawBackground ( QPainter * painter, const QRectF & /*rect*/ )
{
    qreal total = mainWindow->mediaObject()->totalTime();
    qreal relX1 = 0.0f;

    QColor colors[] = {
        QColor(170,255,170),
        QColor(200,255,200),
    };
    int currentColor = 0;

    foreach(MainWindow::Marker scene, mainWindow->getMarkers(MainWindow::SCENE)) {
        qreal relX2 = (qreal)scene.m_ms / total;
        // draw a rectangle which as tall as a widget and runs from relX1 to relX2
        paintRange(painter, relX1, relX2-relX1, colors[currentColor]);
        currentColor = (currentColor+1) % 2;
        relX1 = relX2;
    }
    // the last one: form relX1 to 1.0
    paintRange(painter, relX1, 1.0f - relX1, colors[currentColor]);

    painter->setPen(QColor(0,0,0,100));
    foreach(MainWindow::Marker scene, mainWindow->getMarkers(MainWindow::EVENT)) {
        qreal relX2 = (qreal)scene.m_ms / total;
        painter->drawLine(QPointF(relX2,0.), QPointF(relX2,1.));
    }

}

void TimeLineWidget::drawForeground ( QPainter * painter, const QRectF & rect )
{
    float x = (float)m_currentTime / (float)mainWindow->mediaObject()->totalTime();

    if (x > rect.x() && x < rect.right()) {
        painter->setPen(QColor(0,0,255,100));
        painter->drawLine(QPointF(x,0),QPointF(x,1));
    }
}

void TimeLineWidget::seekTo(qint64 x) {
    mainWindow->seek(mainWindow->mediaObject()->totalTime() * x / (qint64)width());
}

void TimeLineWidget::mousePressEvent ( QMouseEvent * event )
{
    if (seekOnDrag()) {
        if (event->buttons() & Qt::LeftButton) {
            seekTo(event->x());
        }
    }
}

void TimeLineWidget::mouseMoveEvent ( QMouseEvent * event )
{
    if (seekOnDrag()) {
        if (event->buttons() & Qt::LeftButton) {
            seekTo(event->x());
        }
    }
}

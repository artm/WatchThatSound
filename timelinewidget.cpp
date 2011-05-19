#include "timelinewidget.h"
#include "ui_mainwindow.h"
#include <QMouseEvent>

#include "synced.h"

TimeLineWidget::TimeLineWidget(QWidget *parent)
    : QGraphicsView(parent)
    , m_seekOnDrag(false)
    , m_currentTime(0)
    , m_editMode(true)
    , m_deafToSeek(false)
{
    // find the mainWindow
    QObject * iter = parent;
    while(iter && !iter->inherits("MainWindow"))
        iter = iter->parent();
    if (!iter)
        qFatal("TimeLineWidget should be descendant of MainWindow in view hierarchy");
    m_mainWindow = qobject_cast<MainWindow *>(iter);

    connect(m_mainWindow,SIGNAL(storyBoardChanged()),SLOT(update()));
    connect(m_mainWindow,SIGNAL(samplerClock(qint64)),SLOT(setCurrentTime(qint64)));

    setScene(new QGraphicsScene(0.0,0.0,1.0,1.0,this));
    m_cursorLine = new QGraphicsLineItem(0,0,0,1.0,0,scene());
    m_cursorLine->setPen( QPen(Qt::blue) );
    m_cursorLine->setZValue(10);
    m_selectionRect = new QGraphicsRectItem(0,scene());
    m_selectionRect->setVisible(false);
    m_selectionRect->setZValue(5);
    m_selectionRect->setPen( Qt::NoPen );
    QColor selbg(Qt::blue);
    selbg = selbg.lighter();
    selbg.setAlpha(100);
    m_selectionRect->setBrush( QBrush( selbg ) );
    connect(scene(), SIGNAL(selectionChanged()), SLOT(updateSelection()));
}

void TimeLineWidget::resizeEvent ( QResizeEvent * /*event*/ )
{
    fitInView(0,0,1.0,1.0);
}

void TimeLineWidget::setCurrentTime(qint64 time)
{
    if (m_deafToSeek) return;

    Phonon::MediaObject * mo = m_mainWindow->mediaObject();
    if (!mo) return;

    if (time != m_currentTime) {
        m_currentTime = time;
        m_cursorLine->setX( (double)m_currentTime / mo->totalTime() );
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
    qreal total = m_mainWindow->mediaObject()->totalTime();
    qreal relX1 = 0.0f;

    QPainter::RenderHints oldHints = painter->renderHints();
    painter->setRenderHint(QPainter::Antialiasing, false);

    QColor colors[] = {
        QColor(170,255,170),
        QColor(200,255,200),
    };
    int currentColor = 0;

    foreach(MainWindow::Marker * marker, m_mainWindow->getMarkers(MainWindow::SCENE)) {
        qreal relX2 = (qreal)marker->at() / total;
        // draw a rectangle which as tall as a widget and runs from relX1 to relX2
        paintRange(painter, relX1, relX2-relX1, colors[currentColor]);
        currentColor = (currentColor+1) % 2;
        relX1 = relX2;
    }
    // the last one: form relX1 to 1.0
    paintRange(painter, relX1, 1.0f - relX1, colors[currentColor]);

    // draw tension curve antialised...
    //painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QColor(0,0,0,200));
    foreach(MainWindow::Marker * marker, m_mainWindow->getMarkers(MainWindow::EVENT)) {
        qreal relX2 = (qreal)marker->at() / total;
        painter->drawLine(QPointF(relX2,0.), QPointF(relX2,1.));
    }

    painter->setPen(QColor(255,100,100,127));
    painter->drawPath( m_mainWindow->tensionCurve() );

    painter->setRenderHints(oldHints, true);
}

void TimeLineWidget::seekTo(qint64 x) {
    m_deafToSeek = true;
    m_mainWindow->seek(m_mainWindow->mediaObject()->totalTime() * x / (qint64)width());
    m_cursorLine->setX( (double)x / width() );
    m_deafToSeek = false;
}

void TimeLineWidget::mousePressEvent ( QMouseEvent * event )
{
    QGraphicsView::mousePressEvent(event);

    // check if a draggable item was hit
    if (seekOnDrag()) {
        if (event->buttons() & Qt::LeftButton) {
            seekTo(event->x());
        }
    }
}

void TimeLineWidget::mouseMoveEvent ( QMouseEvent * event )
{
    QGraphicsView::mouseMoveEvent(event);

    if (seekOnDrag()) {
        if (event->buttons() & Qt::LeftButton) {
            seekTo(event->x());
        }
    }
}

void TimeLineWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
}

void TimeLineWidget::assignSynced(QGraphicsItem *item, WTS::Synced *synced)
{
    item->setData(SYNCED, QVariant::fromValue(synced));
}

QGraphicsItem * TimeLineWidget::findSynced(QGraphicsItem *item, WTS::Synced **synced)
{
    while( item && !item->data(SYNCED) .isValid() ) {
        item = item->parentItem();
    }
    if (item) {
        *synced = item->data(SYNCED).value<WTS::Synced*>();
        return item;
    } else {
        *synced = 0;
        return 0;
    }
}

void TimeLineWidget::updateSelection()
{
    QList<QGraphicsItem *> sel = scene()->selectedItems();
    if (sel.length() > 0) {
        if (sel[0] == m_selectionRect->parentItem())
            return;
        float dx = 3, dy = 3;
        QRectF r = sel[0]->boundingRect().adjusted(-dx,-dy,dx,dy);
        m_selectionRect->setRect( r );
        m_selectionRect->setParentItem( sel[0] );
        m_selectionRect->setVisible(true);
    } else {
        m_selectionRect->setVisible(false);
        m_selectionRect->setParentItem(0);
    }
}


#include "TimeLineWidget.h"
#include "ui_MainWindow.h"
#include <QMouseEvent>

#include "SharpLine.h"
#include "BufferItem.h"
#include "Synced.h"

using namespace WTS;

TimeLineWidget::TimeLineWidget(QWidget *parent)
    : QGraphicsView(parent)
    , m_seekOnDrag(false)
    , m_currentTime(0)
    , m_editMode(true)
    , m_deafToSeek(false)
{
    // find the mainWindow
    QObject * iter = parent;
    while(iter && !iter->inherits("WTS::MainWindow"))
        iter = iter->parent();
    if (!iter)
        qCritical("TimeLineWidget should be descendant of MainWindow in view hierarchy");
    m_mainWindow = qobject_cast<MainWindow *>(iter);

    connect(m_mainWindow,SIGNAL(storyBoardChanged()),SLOT(invalidateBackground()));
    connect(m_mainWindow,SIGNAL(samplerClock(qint64)),SLOT(setCurrentTime(qint64)));

    setScene(new QGraphicsScene(0.0,0.0,1.0,1.0,this));
    m_cursorLine = new SharpLine(scene());
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

    setLineWidth(0);
    setMidLineWidth(0);
    setCacheMode(QGraphicsView::CacheBackground);
}

void TimeLineWidget::resizeEvent ( QResizeEvent * /*event*/ )
{
    fitInView(0,0,1.0,1.0);
}

void TimeLineWidget::setCurrentTime(qint64 time)
{
    if (m_deafToSeek) return;

    if (time != m_currentTime) {
        m_currentTime = time;
        m_cursorLine->setX( (double)m_currentTime / m_mainWindow->duration() );
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
    qreal total = m_mainWindow->duration();
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

    painter->setPen(QColor(0,0,0,200));
    foreach(MainWindow::Marker * marker, m_mainWindow->getMarkers(MainWindow::EVENT)) {
        qreal relX2 = (qreal)marker->at() / total;
        painter->drawLine(QPointF(relX2,0.), QPointF(relX2,1.));
    }

    painter->setPen(QColor(255,100,100,127));
    painter->drawPath( m_mainWindow->tensionCurve() );

    if (m_editMode) {
        painter->setPen(QColor(255,100,100));
        painter->drawRect( QRectF(0,0,1,1) );
    }

    painter->setRenderHints(oldHints, true);
}

void TimeLineWidget::mousePressEvent ( QMouseEvent * event )
{
    QGraphicsView::mousePressEvent(event);

    doSeekOnDrag(event);
}

void TimeLineWidget::mouseMoveEvent ( QMouseEvent * event )
{
    QGraphicsView::mouseMoveEvent(event);
    doSeekOnDrag(event);
}

void TimeLineWidget::doSeekOnDrag( QMouseEvent * event )
{
    if (seekOnDrag()) {
        if (event->buttons() & Qt::LeftButton) {
            qint64 t = m_mainWindow->duration() * event->x() / (qint64)width();

            QGraphicsItem * dragged = scene()->mouseGrabberItem();
            BufferItem * dragged_b = dynamic_cast<BufferItem *>(dragged);

            if (dragged_b) {
                t = dragged_b->buffer()->rangeStartAt();
            } else if (dragged) {
                WTS::Synced * synced = 0;
                findSynced(dragged, &synced);
                if (synced) {
                    t = synced->at();
                }
            }

            m_deafToSeek = true;
            m_mainWindow->seek( t );
            m_cursorLine->setX( (double)t / m_mainWindow->duration() );
            m_deafToSeek = false;
        }
    }
}

void TimeLineWidget::assignSynced(QGraphicsItem *item, WTS::Synced *synced)
{
    item->setData(SYNCED, QVariant::fromValue(synced));
}

QGraphicsItem * TimeLineWidget::findSynced(QGraphicsItem *item, WTS::Synced **synced)
{
    while( item && !item->data(SYNCED).isValid() ) {
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
        //float dx = 3.0 / width(), dy = 3.0 / height();
        QRectF marg = sel[0]->mapRectFromScene(0, 0, 3.0 / width(), 3.0 / height());
        QRectF r = sel[0]->boundingRect().adjusted(-marg.width(),-marg.height(),marg.width(),marg.height());
        m_selectionRect->setRect( r );
        m_selectionRect->setParentItem( sel[0] );
        m_selectionRect->setVisible(true);
    } else {
        m_selectionRect->setVisible(false);
        m_selectionRect->setParentItem(0);
    }
}

void TimeLineWidget::setEditMode(bool on)
{
    m_editMode = on;
    invalidateBackground();
}

void TimeLineWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
        QList<QGraphicsItem *> selection = scene()->selectedItems();
        foreach(QGraphicsItem * i, selection) {
            scene()->removeItem(i);

            WTS::Synced * synced = 0;
            QGraphicsItem * syncedItem = findSynced(i,&synced);

            if (syncedItem) {
                deleteSynced(syncedItem, synced);
            }

            i->setSelected(false);
            delete i;

            m_mainWindow->saveData();
        }
    }
}


#include "scoresymbol.h"

#include <QtGui>

const float ScoreSymbol::s_maxThickness = 30;

ScoreSymbol::ScoreSymbol()
    : m_running(false)
{
    m_pen = QPen(QColor(0,0,127));
    m_brush = QBrush(QColor(64,64,255,200));
}

void ScoreSymbol::start(const QPointF &pos)
{
    m_shape = CIRCLE;
    m_thickness[0] = 5;
    m_thickness[1] = 0;
    m_pos = pos;
    m_length = 0;
    m_inkTimer.start(500);
    m_running = true;
    updateGraphics();
}

void ScoreSymbol::ink()
{
    // sanity check
    if (!m_running) {
        m_inkTimer.stop();
        return;
    }

    int target = 0;
    m_inkTimer.setInterval(50);

    switch(symbolShape()) {
    case CIRCLE:
        setShape(FILLED_CIRCLE);
        break;
    case DROPLET:
        target = 1;
    case FILLED_CIRCLE:
        m_thickness[target] = std::min(s_maxThickness, m_thickness[target]+1.0f);
        if (m_thickness[0] == m_thickness[1]) {
            m_inkTimer.setInterval(500);
        }
        break;
    case TAILED_CIRCLE:
        // ignore
        return;
    }

    updateGraphics();
}

void ScoreSymbol::pull(const QPointF &pos)
{
    if (!m_running) return;
    m_length = pos.x() - m_pos.x();

    float len = mapFromScene(m_pos + QPointF(m_length, 0)).x();

    switch(symbolShape()) {
    case CIRCLE:
        if (fabs(len) > m_thickness[0]) {
            setShape(TAILED_CIRCLE);
            m_inkTimer.stop(); // not interested in ink no more
        }
        break;
    case TAILED_CIRCLE:
        if (fabs(len) <= m_thickness[0]) {
            setShape(CIRCLE);
        }
        break;
    case FILLED_CIRCLE:
        if (fabs(len) > m_thickness[0]) {
            setShape(DROPLET);
        }
        break;
    case DROPLET:
        if (fabs(len) <= m_thickness[0]) {
            setShape(FILLED_CIRCLE);
        }
        if (m_thickness[1] > 0)
            m_inkTimer.stop(); // not interested in ink no more
        break;
    }

    if (m_inkTimer.isActive())
        m_inkTimer.start(500);

    updateGraphics();
}

void ScoreSymbol::finish()
{
    if (!m_running) return;

    m_running = false;
    m_inkTimer.stop();
    m_inkTimer.disconnect(SIGNAL(timeout()));
}

void ScoreSymbol::configure(QGraphicsScene *theScene, int width, int height)
{
    if (!scene())
        theScene->addItem(this);
    setTransform( QTransform::fromScale( 1.0/width, 1.0/height ) );
}

void ScoreSymbol::updateGraphics()
{
    if (!scene())
        return;

    foreach(QGraphicsItem * child, childItems()) {
        scene()->removeItem(child);
        delete child;
    }

    // length is in ms but item is in pixels...
    float len = mapFromScene(m_pos + QPointF(m_length, 0)).x();
    float d0 = m_thickness[0], d1 = m_thickness[1], r0 = d0/2.0, r1 = d1/2.0;
    switch(symbolShape()) {
    case DROPLET: {
        QPainterPath path;

        path.moveTo(0,-r0);
        path.arcTo(-r0,-r0,d0,d0,90,len > r0 ? 180 : -180);
        path.lineTo(len,r1);
        if (r1 > 0) {
            path.arcTo(len-r1,-r1,d1,d1, 270, len > r0 ? 180 : -180);
        }
        path.closeSubpath();

        QGraphicsPathItem * pathItem =
                new QGraphicsPathItem(path, this);
        pathItem->setPen(m_pen);
        pathItem->setBrush(m_brush);
    }
        break;
    case TAILED_CIRCLE:
        if (fabs(len) > r0) {
            QGraphicsLineItem * line =
                    new QGraphicsLineItem(QLineF(len > 0 ? r0 : - r0, 0, len,0),
                                          this);
            line->setPen(m_pen);
    }
    case CIRCLE: {
        QGraphicsEllipseItem * ellipse =
                new QGraphicsEllipseItem(QRectF(-r0,-r0,d0,d0),
                                         this);
        ellipse->setPen(m_pen);
        break;
    }
    case FILLED_CIRCLE: {
        QGraphicsEllipseItem * ellipse =
                new QGraphicsEllipseItem(QRectF(-r0,-r0,d0,d0),
                                         this);
        ellipse->setPen(m_pen);
        ellipse->setBrush(m_brush);
        break;
    }
    }

    setPos(m_pos);
}

void ScoreSymbol::setColors(const QPen &pen, const QBrush &brush)
{
    m_pen = pen;
    m_brush = brush;
}

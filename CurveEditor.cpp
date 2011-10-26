#include "CurveEditor.h"
#include <QMouseEvent>

using namespace WTS;

CurveEditor::CurveEditor(QWidget *parent)
    : TimeLineWidget(parent)
    , m_nodePixelSize(8)
    , m_curve(0)
    , m_dragItem(0)
{
    m_curve = scene()->addPath(QPainterPath(), QPen(Qt::red));
}

void CurveEditor::setCurve(const QPainterPath& curve)
{
    Q_ASSERT(m_curve);
    m_curve->setPath(curve);
    int cnt = m_curve->path().elementCount();

    foreach(QGraphicsItem * child, m_curve->childItems()) {
        delete child;
    }

    for(int i = 0; i<cnt; ++i) {
        const QPainterPath::Element& elt = m_curve->path().elementAt(i);
        QGraphicsRectItem * node = new QGraphicsRectItem(m_curve);
        node->setBrush(QBrush(QColor(255,100,100,127)));
        node->setData(0, i); // save element index into node
        node->setPos(elt.x, elt.y);
    }

    scaleNodes();
}

void CurveEditor::resizeEvent(QResizeEvent * event)
{
    TimeLineWidget::resizeEvent(event);

    scaleNodes();
}



void CurveEditor::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        m_dragLastP = mapToScene(event->pos());
        m_dragItem  = dynamic_cast<QGraphicsRectItem *>(itemAt( event->pos() ));
    }
    TimeLineWidget::mousePressEvent(event);
}

void CurveEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_dragItem) {
            emit dataChanged();
            m_dragItem = 0;
        }
    }
    TimeLineWidget::mouseReleaseEvent(event);
}

void CurveEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        if (m_dragItem) {
            QPointF newPos = mapToScene(event->pos());
            float level = std::min(1.0, std::max(0.0, newPos.y()));
            newPos.setY(level);
            QPointF d = newPos - m_dragLastP;
            m_dragLastP = newPos;
            m_dragItem->moveBy(0, d.y());
            QPainterPath path = m_curve->path();
            int nodeIndex = m_dragItem->data(0).toInt();
            path.setElementPositionAt(nodeIndex,
                                      m_dragItem->pos().x(),
                                      m_dragItem->pos().y());
            m_curve->setPath(path);
            emit updateLevel( nodeIndex, level );
        }
    }
    TimeLineWidget::mouseMoveEvent(event);
}

void CurveEditor::scaleNodes()
{
    if (m_curve) {
        foreach(QGraphicsItem * child, m_curve->childItems()) {
            QGraphicsRectItem * node = dynamic_cast<QGraphicsRectItem *>(child);
            if (!node)
                continue;

            QPointF size2(m_nodePixelSize / 2.0 / width(),
                          m_nodePixelSize / 2.0 / height() );

            node->setRect( QRectF(- size2, size2) );
        }
    }
}

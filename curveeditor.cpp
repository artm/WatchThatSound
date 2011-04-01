#include "curveeditor.h"

CurveEditor::CurveEditor(QWidget *parent)
    : TimeLineWidget(parent)
    , m_nodePixelSize(8)
    , m_curve(0)
    , m_dragItem(0)
    , m_edited(false)
{
    m_curve = scene()->addPath(QPainterPath(), QPen(Qt::red));
}

void CurveEditor::setCurve(const QPainterPath& curve)
{
    if (!m_curve)
        m_curve = scene()->addPath(curve, QPen(Qt::red));
    else {
        m_curve->setPath(curve);
    }

    int cnt = m_curve->path().elementCount();

    foreach(QGraphicsItem * child, m_curve->childItems()) {
        delete child;
    }

    for(int i = 0; i<cnt; ++i) {
        const QPainterPath::Element& elt = m_curve->path().elementAt(i);
        QGraphicsRectItem * node = new QGraphicsRectItem(m_curve);
        node->setBrush(QBrush(QColor(255,100,100,127)));
        node->setData(0, i); // save element into node
        node->setPos(elt.x, elt.y);
    }

    emit dataChanged();

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
}

void CurveEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragItem = 0;
    }
}

void CurveEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        if (m_dragItem) {
            QPointF newPos = mapToScene(event->pos());
            newPos.setY(std::min(1.0, std::max(0.0, newPos.y())));
            QPointF d = newPos - m_dragLastP;
            m_dragLastP = newPos;
            m_dragItem->moveBy(0, d.y());
            QPainterPath path = m_curve->path();
            path.setElementPositionAt(m_dragItem->data(0).toInt(),
                                      m_dragItem->pos().x(),
                                      m_dragItem->pos().y());
            m_curve->setPath(path);
            emit dataChanged();
        }
    }
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

            m_edited = true;
        }
    }
}

void CurveEditor::saveData(QXmlStreamWriter &xml)
{
    if (!m_curve)
        return;
    xml.writeAttribute("edited", QString("%1").arg(isEdited()));    

    foreach(QGraphicsItem * child, m_curve->childItems()) {
        QGraphicsRectItem * node = dynamic_cast<QGraphicsRectItem *>(child);
        if (!node)
            continue;

        xml.writeStartElement("node");
        xml.writeAttribute("ms", QString("%1").arg(node->pos().x()
                           * m_mainWindow->mediaObject()->totalTime()));
        xml.writeAttribute("level", QString("%1").arg(node->pos().y()));
        xml.writeEndElement();
    }
}

void CurveEditor::loadData(QXmlStreamReader &xml)
{
    m_edited = xml.attributes().value("edited").toString().toInt();

    QPainterPath curve;
    bool init = true;
    while(xml.readNextStartElement()) {
        float ms = xml.attributes().value("ms").toString().toFloat();
        float y = xml.attributes().value("level").toString().toFloat();
        float x = ms / m_mainWindow->mediaObject()->totalTime();
        if (init) {
            curve.moveTo(x,y);
            init = false;
        } else
            curve.lineTo(x,y);
        xml.readElementText();
    }
    setCurve(curve);
}


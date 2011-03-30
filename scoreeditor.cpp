#include "scoreeditor.h"

ScoreEditor::ScoreEditor(QWidget *parent)
    : TimeLineWidget(parent)
    , m_gridStep(5)
{
}

void ScoreEditor::drawBackground(QPainter *painter, const QRectF &rect)
{
    TimeLineWidget::drawBackground(painter, rect);
    painter->setPen(QColor(0,64,0,16));

    /* FIXME restrict to rect? */
    for(int i=0; i<width(); i+=m_gridStep) {
        float x = (float)i/width();
        painter->drawLine(QPointF(x,0.0),QPointF(x,1.0));
    }
    for(int j=0; j<height(); j+=m_gridStep) {
        float y = (float)j/height();
        painter->drawLine(QPointF(0.0,y),QPointF(1.0,y));
    }
}

void ScoreEditor::mouseReleaseEvent(QMouseEvent * /*event*/)
{
    m_newSymbol.finish();
}

void ScoreEditor::mousePressEvent(QMouseEvent * event)
{
    m_newSymbol.start(mapToScene(event->pos()));
}

void ScoreEditor::mouseMoveEvent(QMouseEvent * event)
{
    m_newSymbol.pull(mapToScene(event->pos()));
}


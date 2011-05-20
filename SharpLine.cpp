
#include <QPainter>
#include "SharpLine.h"

SharpLine::SharpLine(QGraphicsScene * scene)
    : QGraphicsLineItem(0., 0., 0., 1., 0, scene)
{

}

void SharpLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPainter::RenderHints hints = painter->renderHints();
    painter->setRenderHint(QPainter::Antialiasing, false);
    QGraphicsLineItem::paint(painter, option, widget);
    painter->setRenderHints(hints);
}

#ifndef SHARPLINE_H
#define SHARPLINE_H

#include <QGraphicsLineItem>

namespace WTS {

/* This is exactly like line widget except it switches off antialiasing before painting itself */
class SharpLine : public QGraphicsLineItem
{
public:
    SharpLine(QGraphicsScene * scene = 0);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

}

#endif // SHARPLINE_H

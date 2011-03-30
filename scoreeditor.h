#ifndef SCOREEDITOR_H
#define SCOREEDITOR_H

#include "timelinewidget.h"
#include "scoresymbol.h"

class ScoreEditor : public TimeLineWidget
{
    Q_OBJECT
public:
    explicit ScoreEditor(QWidget *parent = 0);

    void drawBackground(QPainter *painter, const QRectF &rect);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);


signals:

public slots:

protected:
    unsigned m_gridStep;
    ScoreSymbol m_newSymbol;
};

#endif // SCOREEDITOR_H

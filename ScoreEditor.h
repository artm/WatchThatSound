#ifndef SCOREEDITOR_H
#define SCOREEDITOR_H

#include "TimeLineWidget.h"
#include "ScoreSymbol.h"

namespace WTS {

class ScoreEditor : public TimeLineWidget
{
    Q_OBJECT
public:
    explicit ScoreEditor(QWidget *parent = 0);

    void drawBackground(QPainter *painter, const QRectF &rect);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);

signals:

public slots:
    void passInk() { m_newSymbol->ink(); }
    virtual void setProject(Project *project);
    void saveSection(QXmlStreamWriter& xml);
    bool loadSection(QXmlStreamReader& xml);
    QList<ScoreSymbol*> scoreSymbols();

protected:
    void selectPetal(QGraphicsItem * petal);
    void initNewSymbol();

    enum {
        PetalIndex = 100
    };

    // thin lines are # of lines in each strip cut by the thick line
    unsigned m_gridThinLines, m_gridThickLines, m_gridVerticalLines;
    ScoreSymbol * m_newSymbol;
    QGraphicsItemGroup * m_colorWheel;
    QGraphicsEllipseItem * m_colorSelCircle;

    static const int s_wheelColorCount;
    static const float s_wheelInnerRadius;
    static const float s_wheelOuterRadius;
};

}

#endif // SCOREEDITOR_H

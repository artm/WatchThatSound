#ifndef SCORESYMBOL_H
#define SCORESYMBOL_H

#include <QGraphicsItem>
#include <QTimer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QPen>
#include <QBrush>

namespace WTS {

class ScoreSymbol : public QGraphicsItem
{
public:
    enum Shape {
        CIRCLE,
        FILLED_CIRCLE,
        TAILED_CIRCLE,
        DROPLET
    };

    explicit ScoreSymbol();
    Shape symbolShape() const { return m_shape; }

    void configure(QGraphicsScene * scene, int width, int height);
    void setColors(const QPen& pen, const QBrush& brush);

    QTimer * inkTimer() { return &m_inkTimer; }
    void ink();
    void setShape(Shape shape) { m_shape = shape; }

    void start(const QPointF& pos);
    void pull(const QPointF& pos);
    bool finish();

    void saveData(QXmlStreamWriter& xml);
    void loadData(QXmlStreamReader& xml);

    void updateGraphics();

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

protected:

    Shape m_shape;
    float m_thickness[2];
    float m_length;
    bool m_running;
    QTimer m_inkTimer;

    QPen m_pen;
    QBrush m_brush;

    bool m_TMP_deleted;

    static const float s_maxThickness;
};

}

#endif // SCORESYMBOL_H

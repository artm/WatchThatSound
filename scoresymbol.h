#ifndef SCORESYMBOL_H
#define SCORESYMBOL_H

#include <QGraphicsItemGroup>
#include <QTimer>

class ScoreSymbol : public QGraphicsItemGroup
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
    void finish();

protected:
    void updateGraphics();

    Shape m_shape;
    float m_thickness[2];
    QPointF m_pos;
    float m_length;
    bool m_running;
    QTimer m_inkTimer;

    QPen m_pen;
    QBrush m_brush;

    static const float s_maxThickness;
};

#endif // SCORESYMBOL_H

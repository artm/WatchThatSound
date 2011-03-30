#ifndef SCORESYMBOL_H
#define SCORESYMBOL_H

#include <QObject>
#include <QTimer>

class ScoreSymbol : public QObject
{
    Q_OBJECT
    Q_ENUMS(Shape)
public:
    enum Shape {
        CIRCLE,
        FILLED_CIRCLE,
        TAILED_CIRCLE,
        DROPLET,
        POLY
    };

    explicit ScoreSymbol(QObject *parent = 0);
    Shape shape() const { return m_shape; }

signals:

public slots:
    void ink();
    void setShape(Shape shape) { m_shape = shape; }

    void start(const QPointF& pos);
    void pull(const QPointF& pos);
    void finish();

protected:
    Shape m_shape;
    int m_thickness[2];
    QPointF m_pos;
    float m_length;
    bool m_running;
    QTimer m_inkTimer;
};

#endif // SCORESYMBOL_H

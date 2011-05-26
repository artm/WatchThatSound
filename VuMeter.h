#ifndef VUMETER_H
#define VUMETER_H

#include <QWidget>

class VuMeter : public QWidget
{
    Q_OBJECT
public:
    explicit VuMeter(QWidget *parent = 0);

    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);

signals:

public slots:
    void setValue(float value);

protected:
    int m_ledCount;
    // geometry
    QSizeF m_ledSize;
    float m_gap;
    int m_horizMargin, m_vertMargin;


    float m_value;
};

#endif // VUMETER_H

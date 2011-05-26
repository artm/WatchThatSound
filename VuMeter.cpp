#include "VuMeter.h"

#include <QtGui>

using namespace WTS;

VuMeter::VuMeter(QWidget *parent)
    : QWidget(parent)
    , m_ledCount(16)
    , m_value(0)
{
    m_horizMargin = 5;
    m_vertMargin = 5;
}

void VuMeter::resizeEvent(QResizeEvent * /*event*/)
{
    // find geometry

    int w = width() - 2 * m_horizMargin, h = height() - 2 * m_vertMargin;
    m_gap = (float)h / (m_ledCount * 3 - 1);
    m_ledSize = QSizeF( w, m_gap * 2 );

    update();
}

void VuMeter::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.fillRect(0,0,width(),height(), QBrush(Qt::black));

    float logValue =  (10.0 * log10( m_value ) + 20.0)  / 23.0 * m_ledCount;

    for(int i = 0; i<m_ledCount; ++i) {
        QBrush brush(QColor(0,50,0));
        if (i==0) {
            brush = QBrush(QColor(50,0,0));
        } else if ( m_ledCount / i > 4) {
            brush = QBrush(QColor(50,50,0));
        }

        if (logValue > (m_ledCount - i)) {
            brush.setColor( brush.color().lighter( 200 ) );
        }

        painter.fillRect(QRect(QPoint(m_horizMargin, i * (m_gap + m_ledSize.height())),
                               m_ledSize.toSize()),
                         brush);
    }
}

void VuMeter::setValue(float value)
{
    m_value = value;
    update();
}

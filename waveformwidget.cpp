#include "waveformwidget.h"
#include <QtGui>

WaveformWidget::WaveformWidget(QWidget *parent) :
    QWidget(parent), m_scaleMax(m_minScaleMax), m_wasRecording(false), m_buffer(0)
{
}

void WaveformWidget::updateWaveform(WtsAudio::BufferAt * bufferAt, bool recording)
{
    m_buffer = bufferAt->buffer();

    QPainter painter(&m_img);
    painter.setPen(QColor(0,0,0,100));
    painter.fillRect(0,0,m_img.width(),m_img.height(), m_buffer->color() );
    int midY = m_img.height()/2;

    qint64 stride = 1;
    const float * val = m_buffer->floatAt(0);
    qint64 lineCount =
            std::min((qint64)m_img.width(),
                     recording ? m_buffer->m_writePos : m_buffer->sampleCount());
    QVector<float> Y(lineCount, 0.0);

    if (recording != m_wasRecording || !recording) {
        m_scaleMax = m_minScaleMax;
        m_wasRecording = recording;
    }

    if (recording) {
        val = m_buffer->floatAt(m_buffer->m_writePos - lineCount);
    } else {
        // show the whole thing
        stride = m_buffer->sampleCount() / lineCount;
    }

    for(int x = 0; x<lineCount; ++x) {
        Y[x] = val[x*stride];
        for(int i=1; i<stride; ++i) {
            Y[x] = std::max(Y[x], (float)fabs(val[x*stride+i]));
        }
        m_scaleMax = std::max(Y[x],m_scaleMax);
    }

    for(int x = 0; x<lineCount; ++x) {
        int y1 = midY + (0.45 * (Y[x]/m_scaleMax) * (float)m_img.height());
        int y2 = midY - (0.45 * (Y[x]/m_scaleMax) * (float)m_img.height());
        painter.drawLine(x, y1, x, y2);
    }

    painter.drawLine(lineCount, midY, m_img.width(), midY);

    update();
}

void WaveformWidget::paintEvent(QPaintEvent */*event*/)
{
    QPainter painter(this);
    painter.drawPixmap(0,0,m_img);

    if (m_buffer) {
        int x1 = m_buffer->rangeStart() * width() / m_buffer->duration();
        int x2 = m_buffer->rangeEnd() * width() / m_buffer->duration();
        painter.fillRect(QRect(0,0,x1,height()), QBrush(QColor(0,0,0,70)));
        painter.fillRect(QRect(x2,0,width(),height()), QBrush(QColor(0,0,0,70)));
    }
}

void WaveformWidget::resizeEvent(QResizeEvent *event)
{
    if (event->size() != event->oldSize()) {

        m_img = QPixmap(event->size());
        m_img.fill(QColor(200,255,200));
    }
}

void WaveformWidget::mousePressEvent(QMouseEvent * e)
{
    m_selStart = e->x() * m_buffer->duration() / width();
    m_buffer->setRange(m_selStart, m_selStart);
    update();
}

void WaveformWidget::mouseMoveEvent(QMouseEvent * e)
{
    float sampleNum = e->x() * m_buffer->duration() / width();
    if (sampleNum > m_selStart) {
        m_buffer->setRange(m_selStart, sampleNum);
    } else {
        m_buffer->setRange(sampleNum, m_selStart);
    }
    update();
}

void WaveformWidget::mouseReleaseEvent(QMouseEvent *)
{
    emit rangeChanged(m_buffer);
}

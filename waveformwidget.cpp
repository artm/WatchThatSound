#include "waveformwidget.h"
#include <QtGui>

WaveformWidget::WaveformWidget(QWidget *parent) :
    QWidget(parent), scaleMax(minScaleMax), wasRecording(false)
{
}

void WaveformWidget::updateWaveform(bool recording, qint64 /*at*/, const SoundBuffer& buffer)
{
    QPainter painter(&img);
    painter.setPen(QColor(0,0,0,100));
    painter.fillRect(0,0,img.width(),img.height(), buffer.color() );
    int midY = img.height()/2;

    qint64 stride = 1;
    const float * val = buffer.floatAt(0);
    qint64 lineCount = std::min((qint64)img.width(), buffer.m_writePos);
    QVector<float> Y(lineCount, 0.0);

    if (recording != wasRecording || !recording) {
        scaleMax = minScaleMax;
        wasRecording = recording;
    }

    if (recording) {
        qint64 offs = buffer.m_writePos - lineCount;
        val = buffer.floatAt(offs);
    } else if (buffer.m_writePos) {
        // show the whole thing
        stride = buffer.m_writePos / lineCount;
    }

    for(int x = 0; x<lineCount; ++x) {
        Y[x] = val[x*stride];
        for(int i=1; i<stride; ++i) {
            Y[x] = std::max(Y[x], (float)fabs(val[x*stride+i]));
        }
        scaleMax = std::max(Y[x],scaleMax);
    }

    for(int x = 0; x<lineCount; ++x) {
        int y1 = midY + (0.45 * (Y[x]/scaleMax) * (float)img.height());
        int y2 = midY - (0.45 * (Y[x]/scaleMax) * (float)img.height());
        painter.drawLine(x, y1, x, y2);
    }

    painter.drawLine(lineCount, midY, img.width(), midY);

    update();
}

void WaveformWidget::paintEvent(QPaintEvent */*event*/)
{
    QPainter painter(this);
    painter.drawPixmap(0,0,img);
}

void WaveformWidget::resizeEvent(QResizeEvent *event)
{
    if (event->size() != event->oldSize()) {

        img = QPixmap(event->size());
        img.fill(QColor(200,255,200));
    }
}

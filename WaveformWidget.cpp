#include "WaveformWidget.h"
#include <QtGui>

using namespace WTS;

WaveformWidget::WaveformWidget(QWidget *parent) :
    QWidget(parent), m_scaleMax(m_minScaleMax), m_curTime(-1), m_wasRecording(false), m_buffer(0)
{
}

void WaveformWidget::clearWaveform(WtsAudio::BufferAt * buffer)
{
    if (!buffer || m_buffer == buffer) {
        m_buffer = 0;
        // clean
        m_img.fill( QApplication::palette().color(QPalette::Dark) );
        update();
    }
}

void WaveformWidget::updateWaveform(WtsAudio::BufferAt * bufferAt, bool recording)
{
    emit enableWaveformControls(bufferAt != NULL);

    SoundBuffer * buffer = bufferAt->buffer();

    if (m_buffer != bufferAt) {
        m_buffer = bufferAt;

        float gain = buffer->gain();
        gain = sqrtf(sqrtf(gain)) * 100;

        emit adjustGainSlider((int)gain);
    }

    QPainter painter(&m_img);
    painter.setPen(QColor(0,0,0,100));
    painter.fillRect(0,0,m_img.width(),m_img.height(), buffer->color() );
    int midY = m_img.height()/2;

    qint64 stride = 1;
    const float * val = buffer->floatAt(0);
    qint64 lineCount =
            std::min((qint64)m_img.width(),
                     recording ? buffer->m_writePos : buffer->sampleCount());
    QVector<float> Y(lineCount, 0.0);

    if (recording != m_wasRecording || !recording) {
        m_scaleMax = m_minScaleMax;
        m_wasRecording = recording;
    }

    if (recording) {
        val = buffer->floatAt(buffer->m_writePos - lineCount);
    } else {
        // show the whole thing
        stride = buffer->sampleCount() / lineCount;
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

void WaveformWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.drawPixmap(0,0,m_img);

    if (m_buffer) {
        SoundBuffer * buffer = m_buffer->buffer();

        painter.setPen(QColor(0,0,0,255));

        qint64 cursor = m_curTime - m_buffer->at();

        if (cursor > 0 && cursor < buffer->duration()) {
            int x = cursor * width() / buffer->duration();
            painter.fillRect( x, 0, 1, height(), Qt::blue);
        }

        int x1 = buffer->rangeStart() * width() / buffer->sampleCount();
        int x2 = buffer->rangeEnd() * width() / buffer->sampleCount();
        painter.fillRect(QRect(0,0,x1,height()), QBrush(QColor(0,0,0,70)));
        painter.fillRect(QRect(x2,0,width(),height()), QBrush(QColor(0,0,0,70)));
    }
}

void WaveformWidget::resizeEvent(QResizeEvent *event)
{
    if (event->size() != event->oldSize()) {
        m_img = QPixmap(event->size());
        clearWaveform();
    }
}

void WaveformWidget::mousePressEvent(QMouseEvent * e)
{
    if (!m_buffer)
        return;

    SoundBuffer * buffer = m_buffer->buffer();
    m_selStart = e->x() * buffer->sampleCount() / width();
    buffer->setRange(m_selStart, m_selStart);
    update();
}

void WaveformWidget::mouseMoveEvent(QMouseEvent * e)
{
    SoundBuffer * buffer = m_buffer->buffer();

    float selEnd = e->x() * buffer->sampleCount() / width();
    if (selEnd > m_selStart) {
        buffer->setRange(m_selStart, selEnd);
    } else {
        buffer->setRange(selEnd, m_selStart);
    }
    update();
}

void WaveformWidget::mouseReleaseEvent(QMouseEvent *)
{
    emit rangeChanged(m_buffer->buffer());
}

void WaveformWidget::setGain(int volX)
{
    if (m_buffer) {

        // exponential approximation from
        // http://www.dr-lex.be/info-stuff/volumecontrols.html

        float gain = 0.01 * volX;
        gain *= gain;
        gain *= gain;
        m_buffer->buffer()->setGain( gain );
    }
}

void WTS::WaveformWidget::openInExternalApp()
{
    if (m_buffer)
        emit openInExternalApp(m_buffer->buffer());

}

void WTS::WaveformWidget::setProject(WTS::Project *project)
{
    m_project = project;
    connect(this, SIGNAL(openInExternalApp(SoundBuffer*)), m_project, SLOT(openInExternalApp(SoundBuffer*)));
}

void WTS::WaveformWidget::tick(qint64 ms)
{
    m_curTime = ms;
    // TODO check if update is really necessary
    update();
}

#include "WaveformWidget.h"
#include <QtGui>

using namespace WTS;

WaveformWidget::WaveformWidget(QWidget *parent) :
    QWidget(parent), m_curTime(-1), m_wasRecording(false), m_buffer(0)
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
    if (m_buffer != bufferAt || recording || recording != m_wasRecording) {
        // really changing
        if ((m_buffer = bufferAt) != NULL) {
            // to something...
            SoundBuffer * buffer = m_buffer->buffer();

            float gain = buffer->gain();
            gain = sqrtf(sqrtf(gain)) * 100;
            emit adjustGainSlider((int)gain);

            if (recording != m_wasRecording || !recording)
                m_scaleMax = SoundBuffer::s_minScaleMax;

            if (recording)
                m_scaleMax = buffer->draw(m_img, true, m_scaleMax);
            else
                buffer->draw(m_img);
        } else {
            // became none
            clearWaveform();
        }
    }

    emit enableWaveformControls(m_buffer != NULL);

    update();

    m_wasRecording = recording;
}

void WaveformWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.drawPixmap(0,0,m_img);

    if (m_buffer) {
        SoundBuffer * buffer = m_buffer->buffer();

        painter.setPen(QColor(0,0,0,255));

        qint64 cursor = m_curTime - m_buffer->at();
        if (!m_wasRecording && cursor > 0 && cursor < buffer->duration()) {
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
    connect(this, SIGNAL(rangeChanged(SoundBuffer*)), project, SLOT(save()));
}

void WTS::WaveformWidget::tick(qint64 ms)
{
    m_curTime = ms;
    // TODO check if update is really necessary
    update();
}

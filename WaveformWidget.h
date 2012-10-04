#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

#include <QWidget>
#include "MainWindow.h"
#include "Project.h"

namespace WTS {

class WaveformWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WaveformWidget(QWidget *parent = 0);

    SoundBuffer * soundBuffer();

    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

signals:
    void rangeChanged(SoundBuffer * buffer = 0);
    void adjustGainSlider(int volX);
    void enableWaveformControls(bool enable);
    void openInExternalApp(SoundBuffer *);

public slots:
    void updateWaveform(WtsAudio::BufferAt * bufferAt, bool recording = false);
    void setGain(int volX);
    void clearWaveform(WtsAudio::BufferAt * buffer = 0);
    void openInExternalApp();
    void setProject(Project * project);

    void tick(qint64 ms);

protected:
    QPixmap m_img;
    float m_scaleMax;
    qint64 m_curTime;
    bool m_wasRecording;

    WtsAudio::BufferAt * m_buffer;
    qint64 m_selStart;
    Project * m_project;
};

}

#endif // WAVEFORMWIDGET_H

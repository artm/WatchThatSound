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
    void clearWaveform(SoundBuffer * buffer = 0);
    void openInExternalApp();
    void setProject(Project * project);

protected:
    QPixmap m_img;
    float m_scaleMax;
    static const float m_minScaleMax = 0.1;
    bool m_wasRecording;

    SoundBuffer * m_buffer;
    qint64 m_selStart;
    Project * m_project;
};

}

#endif // WAVEFORMWIDGET_H

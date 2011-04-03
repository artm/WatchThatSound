#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

#include <QWidget>
#include "mainwindow.h"

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
    void rangeChanged();

public slots:
    void updateWaveform(WtsAudio::BufferAt * bufferAt, bool recording = false);

protected:
    QPixmap m_img;
    float m_scaleMax;
    static const float m_minScaleMax = 0.1;
    bool m_wasRecording;

    SoundBuffer * m_buffer;
    qint64 m_selStart;
};

#endif // WAVEFORMWIDGET_H

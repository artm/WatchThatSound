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

signals:

public slots:
    void updateWaveform(bool recording, qint64 at, const SoundBuffer& buffer);

protected:
    QPixmap img;
    float scaleMax;
    static const float minScaleMax = 0.1;
    bool wasRecording;
};

#endif // WAVEFORMWIDGET_H

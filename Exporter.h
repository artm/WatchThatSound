#ifndef EXPORTTHREAD_H
#define EXPORTTHREAD_H

#include "WtsAudio.h"
#include "Project.h"
#include "TimeLineController.hpp"

#include <QByteArray>
#include <QVector>
#include <QProgressDialog>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace WTS {

class VideoFile;

class Exporter : public TimeLineController
{
    Q_OBJECT
public:
    explicit Exporter(QObject *parent = 0);
    void run();
    void configure(const QString& fname,
                   VideoFile * vfile,
                   Project * project,
                   WtsAudio * audio,
                   QProgressDialog * progress);

signals:

public slots:

protected:
    void initExport();
    void initAudioStream(CodecID codec_id);
    void initVideoStream();
    void performExport();
    void finishUp();

    QByteArray m_filename;
    VideoFile * m_originalVideoFile;
    WtsAudio * m_audio;
    QProgressDialog * m_progress;

    AVFormatContext * m_container;
    AVStream * m_videoStream;
    AVStream * m_audioStream;

};

}

#endif // EXPORTTHREAD_H

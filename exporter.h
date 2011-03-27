#ifndef EXPORTTHREAD_H
#define EXPORTTHREAD_H

#include "wtsaudio.h"

#include <QByteArray>
#include <QVector>
#include <QProgressDialog>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

class VideoFile;

class Exporter : public QObject
{
    Q_OBJECT
public:
    explicit Exporter(QObject *parent = 0);
    void run();
    void configure(const QString& fname,
                   VideoFile * vfile,
                   const QList<WtsAudio::BufferAt *>& sequence,
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
    QList<WtsAudio::BufferAt *> m_sequence;
    WtsAudio * m_audio;
    QProgressDialog * m_progress;

    AVFormatContext * m_container;
    AVStream * m_videoStream;
    AVStream * m_audioStream;

};

#endif // EXPORTTHREAD_H

#ifndef EXPORTTHREAD_H
#define EXPORTTHREAD_H

#include "wtsaudio.h"

#include <QThread>
#include <QByteArray>
#include <QVector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

class VideoFile;

class ExportThread : public QThread
{
    Q_OBJECT
public:
    explicit ExportThread(QObject *parent = 0);
    void run();
    void configure(const QString& fname,
                   VideoFile * vfile,
                   const QList<WtsAudio::BufferAt *>& sequence,
                   WtsAudio * audio);

signals:
    void exportProgress(int percent);

public slots:

protected:
    AVStream *add_audio_stream(AVFormatContext *oc, enum CodecID codec_id);
    void open_audio(AVFormatContext *oc, AVStream *st);
    void close_audio(AVFormatContext *oc, AVStream *st);
    AVStream *add_video_stream(AVFormatContext *oc);
    void open_video(AVFormatContext *oc, AVStream *st);
    void close_video(AVFormatContext *oc, AVStream *st);

    QByteArray m_filename;
    VideoFile * m_originalVideoFile;
    QList<WtsAudio::BufferAt *> m_sequence;
    WtsAudio * m_audio;

    QVector<int16_t> m_samples;
    QVector<uint8_t> m_audioOutbuf;
};

#endif // EXPORTTHREAD_H

#ifndef EXPORTTHREAD_H
#define EXPORTTHREAD_H

#include <QThread>
#include <QString>

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
    void configure(const QString& fname, VideoFile * vfile);

signals:
    void exportProgress(int percent);

public slots:

protected:
    AVStream *add_audio_stream(AVFormatContext *oc, enum CodecID codec_id);
    void open_audio(AVFormatContext *oc, AVStream *st);
    void close_audio(AVFormatContext *oc, AVStream *st);
    AVStream *add_video_stream(AVFormatContext *oc, enum CodecID codec_id);
    void open_video(AVFormatContext *oc, AVStream *st);
    void close_video(AVFormatContext *oc, AVStream *st);

    QByteArray m_filename;
    VideoFile * m_videoFile;

    int16_t *m_samples;
    uint8_t *m_audio_outbuf;
    int m_audio_outbuf_size;
    int m_audio_input_frame_size;

    AVFrame *m_picture, *m_tmp_picture;
    uint8_t *m_video_outbuf;
    int m_frame_count, m_video_outbuf_size;
};

#endif // EXPORTTHREAD_H

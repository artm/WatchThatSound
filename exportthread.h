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
    void initExport();
    void initAudioStream(CodecID codec_id);
    void initVideoStream();
    void performExport();
    void finishUp();

    QByteArray m_filename;
    VideoFile * m_originalVideoFile;
    QList<WtsAudio::BufferAt *> m_sequence;
    WtsAudio * m_audio;

    AVFormatContext * m_container;
    AVStream * m_videoStream;
    AVStream * m_audioStream;

};

#endif // EXPORTTHREAD_H

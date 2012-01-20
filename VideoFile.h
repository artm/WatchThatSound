#ifndef VIDEOFILE_H
#define VIDEOFILE_H

#include <QObject>
#include <QImage>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

namespace WTS {

class VideoFile : public QObject
{
    Q_OBJECT
public:
    explicit VideoFile(QObject *parent = 0);
    explicit VideoFile(QString path, QObject *parent = 0);
    virtual ~VideoFile();

    void open(QString path);
    int width() const;
    int height() const;
    QImage frame();

    bool nextPacket(AVPacket& packet);

    CodecID codecId() const;
    const AVCodecContext * codec() const;
    const AVStream * stream() const;

    qint64 duration() const;
    QString path() const { return m_path; }
signals:

public slots:
    void seek(qint64 ms);

protected:
    AVFormatContext *m_formatContext;
    AVCodecContext *m_codecContext;
    AVCodec *m_codec;
    AVFrame *m_frame;
    AVFrame *m_frameRGB;
    QByteArray m_frameBytes;
    int m_streamIndex;
    SwsContext * m_convertContext;
    QString m_path;

    static bool s_ffInited;
};

}

#endif // VIDEOFILE_H

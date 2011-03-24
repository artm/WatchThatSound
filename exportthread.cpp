#include "exportthread.h"
#include "videofile.h"

#include <QtDebug>

/* 5 seconds stream duration */
#define STREAM_DURATION   5.0
#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_NB_FRAMES  ((int)(STREAM_DURATION * STREAM_FRAME_RATE))
#define STREAM_PIX_FMT PIX_FMT_YUV420P /* default pix_fmt */

ExportThread::ExportThread(QObject *parent) :
    QThread(parent)
{
}

void ExportThread::configure(const QString& fname, VideoFile * vfile)
{
    m_filename = fname.toLocal8Bit();
    m_videoFile = vfile;
}

void ExportThread::run()
{
    int job = 30;
    emit exportProgress(0);

    const char * filename = m_filename.constData();

    /* auto detect the output format from the name. default is
       mpeg. */
    AVOutputFormat * fmt = av_guess_format(NULL, filename, NULL);
    if (!fmt) {
        qWarning() << "Could not deduce output format from file extension: using AVI.";
        fmt = av_guess_format("avi", NULL, NULL);
    }
    if (!fmt) {
        qCritical() << "Could not find suitable output format";
        exit(1);
    }

    qDebug() << "Export file:" << filename << "format:" << fmt->name;

    AVFormatContext * oc = avformat_alloc_context();
    if (!oc) {
        qCritical() << "Memory error in ffmpeg";
        exit(1);
    }
    oc->oformat = fmt;
    // apparently this is the normal ffmpeg way to set the filename
    snprintf(oc->filename, sizeof(oc->filename), "%s", filename);

    AVStream * video_st = NULL;
    AVStream * audio_st = NULL;
    if (fmt->video_codec != CODEC_ID_NONE) {
        video_st = add_video_stream(oc, fmt->video_codec);
    }
    if (fmt->audio_codec != CODEC_ID_NONE) {
        audio_st = add_audio_stream(oc, fmt->audio_codec);
    }

    // set the output parameters (must be done even if no parameters)
    if (av_set_parameters(oc, NULL) < 0) {
        qCritical() << "Invalid output format parameters";
        exit(1);
    }

    av_dump_format(oc, 0, filename, 1);

    /* now that all the parameters are set, we can open the audio and
       video codecs and allocate the necessary encode buffers */
    if (video_st)
        open_video(oc, video_st);
    if (audio_st)
        open_audio(oc, audio_st);

    if (avio_open(&oc->pb, filename, URL_WRONLY) < 0) {
        qCritical() << QString("Could not open '%1'").arg(filename);
        exit(1);
    }

    /* write the stream header, if any */
    av_write_header(oc);


    for(int i=0; i<job; i++) {
        usleep(5000);
        emit exportProgress(100 * i / job);
    }

    /* close each codec */
    if (video_st)
        close_video(oc, video_st);
    if (audio_st)
        close_audio(oc, audio_st);

    /* free the resources */
    for(unsigned i = 0; i < oc->nb_streams; i++) {
        av_freep(&oc->streams[i]->codec);
        av_freep(&oc->streams[i]);
    }

    avio_close(oc->pb);
    av_free(oc);
}


AVStream * ExportThread::add_audio_stream(AVFormatContext *oc, enum CodecID codec_id)
{
    AVCodecContext *c;
    AVStream *st;

    st = av_new_stream(oc, 1);
    if (!st) {
        qCritical() << "Could not alloc stream";
        exit(1);
    }

    c = st->codec;
    c->codec_id = codec_id;
    c->codec_type = AVMEDIA_TYPE_AUDIO;

    /* put sample parameters */
    c->sample_fmt = AV_SAMPLE_FMT_S16;
    c->bit_rate = 64000;
    c->sample_rate = 44100;
    c->channels = 2;

    // some formats want stream headers to be separate
    if(oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

    return st;
}

void ExportThread::open_audio(AVFormatContext */*oc*/, AVStream *st)
{
    AVCodecContext *c;
    AVCodec *codec;

    c = st->codec;

    /* find the audio encoder */
    codec = avcodec_find_encoder(c->codec_id);
    if (!codec) {
        qCritical() << "codec not found";
        exit(1);
    }

    /* open it */
    if (avcodec_open(c, codec) < 0) {
        qCritical() << "could not open codec";
        exit(1);
    }

    m_audio_outbuf_size = 10000;
    m_audio_outbuf = (uint8_t*) av_malloc(m_audio_outbuf_size);

    /* ugly hack for PCM codecs (will be removed ASAP with new PCM
       support to compute the input frame size in samples */
    if (c->frame_size <= 1) {
        m_audio_input_frame_size = m_audio_outbuf_size / c->channels;
        switch(st->codec->codec_id) {
        case CODEC_ID_PCM_S16LE:
        case CODEC_ID_PCM_S16BE:
        case CODEC_ID_PCM_U16LE:
        case CODEC_ID_PCM_U16BE:
            m_audio_input_frame_size >>= 1;
            break;
        default:
            break;
        }
    } else {
        m_audio_input_frame_size = c->frame_size;
    }
    m_samples = (int16_t*) av_malloc(m_audio_input_frame_size * 2 * c->channels);
}

void ExportThread::close_audio(AVFormatContext */*oc*/, AVStream *st)
{
    avcodec_close(st->codec);
    av_free(m_samples);
    av_free(m_audio_outbuf);
}

/* add a video output stream */

AVStream * ExportThread::add_video_stream(AVFormatContext *oc, enum CodecID codec_id)
{
    AVCodecContext *c;
    AVStream *st;

    qWarning() << "FIXME: video stream should be a copy of the input stream!";

    st = av_new_stream(oc, 0);
    if (!st) {
        qCritical() << "Could not alloc stream";
        exit(1);
    }

    c = st->codec;
    c->codec_id = codec_id;
    c->codec_type = AVMEDIA_TYPE_VIDEO;

    /* put sample parameters */
    c->bit_rate = 400000;
    /* resolution must be a multiple of two */
    c->width = 352;
    c->height = 288;
    /* time base: this is the fundamental unit of time (in seconds) in terms
       of which frame timestamps are represented. for fixed-fps content,
       timebase should be 1/framerate and timestamp increments should be
       identically 1. */
    c->time_base.den = STREAM_FRAME_RATE;
    c->time_base.num = 1;
    c->gop_size = 12; /* emit one intra frame every twelve frames at most */
    c->pix_fmt = STREAM_PIX_FMT;
    if (c->codec_id == CODEC_ID_MPEG2VIDEO) {
        /* just for testing, we also add B frames */
        c->max_b_frames = 2;
    }
    if (c->codec_id == CODEC_ID_MPEG1VIDEO){
        /* Needed to avoid using macroblocks in which some coeffs overflow.
           This does not happen with normal video, it just happens here as
           the motion of the chroma plane does not match the luma plane. */
        c->mb_decision=2;
    }
    // some formats want stream headers to be separate
    if(oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

    return st;
}

void ExportThread::open_video(AVFormatContext *oc, AVStream *st)
{
    AVCodec *codec;
    AVCodecContext *c;

    c = st->codec;

    /* find the video encoder */
    codec = avcodec_find_encoder(c->codec_id);
    if (!codec) {
        qCritical() << "codec not found";
        exit(1);
    }

    /* open the codec */
    if (avcodec_open(c, codec) < 0) {
        qCritical() << "could not open codec";
        exit(1);
    }

    m_video_outbuf = NULL;
    if (!(oc->oformat->flags & AVFMT_RAWPICTURE)) {
        /* allocate output buffer */
        /* XXX: API change will be done */
        /* buffers passed into lav* can be allocated any way you prefer,
           as long as they're aligned enough for the architecture, and
           they're freed appropriately (such as using av_free for buffers
           allocated with av_malloc) */
        m_video_outbuf_size = 200000;
        m_video_outbuf = (uint8_t*)av_malloc(m_video_outbuf_size);
    }

    /* allocate the encoded raw picture */
    //picture = alloc_picture(c->pix_fmt, c->width, c->height);
    if (!m_picture) {
        qCritical() << "Could not allocate picture";
        exit(1);
    }

    /* if the output format is not YUV420P, then a temporary YUV420P
       picture is needed too. It is then converted to the required
       output format */
    m_tmp_picture = NULL;
    if (c->pix_fmt != PIX_FMT_YUV420P) {
        //tmp_picture = alloc_picture(PIX_FMT_YUV420P, c->width, c->height);
        if (!m_tmp_picture) {
            qCritical() << "Could not allocate temporary picture";
            exit(1);
        }
    }
}

void ExportThread::close_video(AVFormatContext */*oc*/, AVStream *st)
{
    avcodec_close(st->codec);
    av_free(m_picture->data[0]);
    av_free(m_picture);
    if (m_tmp_picture) {
        av_free(m_tmp_picture->data[0]);
        av_free(m_tmp_picture);
    }
    av_free(m_video_outbuf);
}

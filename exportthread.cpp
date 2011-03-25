#include "exportthread.h"
#include "videofile.h"

#include <QtDebug>

class AssertFailed {
public:
    AssertFailed(const QString& cond, const QString& file, int line,
                 QString extra = QString())
    {
        m_message = QString("%1:%2: assertion '%3' failed")
                .arg(file).arg(line).arg(cond);

        if (extra.size())
            m_message += ". " + extra + ".";
    }
    QString message() const { return m_message; }
    const char * cMessage() const { return m_message.toLocal8Bit().constData(); }
protected:
    QString m_message;
};

#define TRY_ASSERT(cond) \
    do { if (!cond) { throw AssertFailed(#cond, __FILE__, __LINE__); } } while (0)
#define TRY_ASSERT_X(cond, message) \
    do { if (!cond) { throw AssertFailed(#cond, __FILE__, __LINE__, message); \
    } } while (0)

ExportThread::ExportThread(QObject *parent)
    : QThread(parent)
    , m_originalVideoFile(0)
{
}

void ExportThread::configure(const QString& fname, VideoFile * vfile)
{
    m_filename = fname.toLocal8Bit();
    m_originalVideoFile = vfile;
}

void ExportThread::run()
{
    try {
        emit exportProgress(0);

        const char * filename = m_filename.constData();

        AVOutputFormat * fmt = av_guess_format(NULL, filename, NULL);
        if (!fmt) {
            qWarning() << "Could not deduce output format "
                          "from file extension: using AVI.";
            fmt = av_guess_format("avi", NULL, NULL);
        }

        TRY_ASSERT_X(fmt, "Could not find suitable output format");

        AVFormatContext * container = avformat_alloc_context();
        TRY_ASSERT_X(container, "Memory error in ffmpeg");
        container->oformat = fmt;
        // apparently this is the normal ffmpeg way to set the filename
        snprintf(container->filename, sizeof(container->filename), "%s", filename);

        AVStream * video_st = NULL;
        AVStream * audio_st = NULL;
        if (fmt->video_codec != CODEC_ID_NONE) {
            video_st = add_video_stream(container);
        }
        if (fmt->audio_codec != CODEC_ID_NONE) {
            audio_st = add_audio_stream(container, fmt->audio_codec);
        }

        // set the output parameters (must be done even if no parameters)
        TRY_ASSERT_X((av_set_parameters(container, NULL) >= 0),
                     "Invalid output format parameters");

        av_dump_format(container, 0, filename, 1);

        /* now that all the parameters are set, we can open the audio and
       video codecs and allocate the necessary encode buffers */
        if (audio_st)
            open_audio(container, audio_st);
        if (video_st)
            open_video(container, video_st);

        TRY_ASSERT_X((avio_open(&container->pb, filename, URL_WRONLY) >= 0),
                     QString("Could not open '%1'").arg(filename));

        av_write_header(container);
        m_originalVideoFile->seek(0);

        double duration = (double)video_st->duration
                * video_st->time_base.num
                / video_st->time_base.den;
        bool moreVideo = true, moreAudio = true;
        while(moreVideo || moreAudio) {
            double audio_pts = (double)audio_st->pts.val
                    * audio_st->time_base.num
                    / audio_st->time_base.den;
            double video_pts = (double)video_st->pts.val
                    * video_st->time_base.num
                    / video_st->time_base.den;

            moreAudio = audio_pts < duration;

            double pts;

            AVPacket packet;
            if ( moreVideo
                    && (!moreAudio || (video_pts < audio_pts))
                    && (moreVideo = m_originalVideoFile->nextPacket(packet)) ) {
                packet.stream_index = video_st->index;
                av_interleaved_write_frame(container, &packet);
                pts = video_pts;
            } else if (moreAudio) {

                av_init_packet(&packet);

                qDebug() << "FIXME get audio mix";

                packet.size = avcodec_encode_audio(audio_st->codec,
                                                   m_audioOutbuf.data(),
                                                   m_audioOutbuf.size(),
                                                   m_samples.data());
                if (audio_st->codec->coded_frame
                        && ( (uint64_t) audio_st->codec->coded_frame->pts
                            != AV_NOPTS_VALUE) )
                    packet.pts= av_rescale_q(audio_st->codec->coded_frame->pts,
                                             audio_st->codec->time_base,
                                             audio_st->time_base);
                packet.flags |= AV_PKT_FLAG_KEY;
                packet.stream_index= audio_st->index;
                packet.data= m_audioOutbuf.data();
                av_interleaved_write_frame(container, &packet);

                pts = audio_pts;
            }

            emit exportProgress(100 * pts / duration);
        }
        av_write_trailer(container);

        /* close each codec */
        if (video_st)
            close_video(container, video_st);
        if (audio_st)
            close_audio(container, audio_st);

        /* free the resources */
        for(unsigned i = 0; i < container->nb_streams; i++) {
            av_freep(&container->streams[i]->codec);
            av_freep(&container->streams[i]);
        }

        avio_close(container->pb);
        av_free(container);

    } catch (const AssertFailed& e) {
        qCritical() << e.cMessage();
    }
}

AVStream * ExportThread::add_audio_stream(AVFormatContext *oc, enum CodecID codec_id)
{
    AVCodecContext *c;
    AVStream *st;

    st = av_new_stream(oc, 1);
    TRY_ASSERT_X(st, "Could not alloc stream");

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

    codec = avcodec_find_encoder(c->codec_id);
    TRY_ASSERT_X(codec, "Could not find audio encoder");
    TRY_ASSERT_X((avcodec_open(c, codec) >= 0), "Could not open audio encoder");

    m_audioOutbuf.resize(10000);

    /* ugly hack for PCM codecs (will be removed ASAP with new PCM
       support to compute the input frame size in samples */
    int inputFrameSize = c->frame_size;
    if (c->frame_size <= 1) {
        inputFrameSize = m_audioOutbuf.size() / c->channels;
        switch(st->codec->codec_id) {
        case CODEC_ID_PCM_S16LE:
        case CODEC_ID_PCM_S16BE:
        case CODEC_ID_PCM_U16LE:
        case CODEC_ID_PCM_U16BE:
            inputFrameSize >>= 1;
            break;
        default:
            break;
        }
    }

    m_samples.resize(inputFrameSize * c->channels);
}

void ExportThread::close_audio(AVFormatContext */*oc*/, AVStream *st)
{
    avcodec_close(st->codec);
    m_samples.resize(0);
    m_audioOutbuf.resize(0);
}

AVStream * ExportThread::add_video_stream(AVFormatContext * container)
{
    // Video output stream is pretty much a copy of the input video
    AVStream * newStream = av_new_stream(container, 0);
    TRY_ASSERT_X(newStream, "Could not allocate memory");
    // this technique is copied from glorious FFmpeg
    //memcpy(newStream, m_videoFile->stream(), sizeof(AVStream));

    const AVStream * oldStream = m_originalVideoFile->stream();
    // we copy some fields but leave other as they were initialized by
    // av_new_stream() above. this way we're sure that no pointers to
    // malloced memory get copied and later freed twice.
    newStream->r_frame_rate = oldStream->r_frame_rate;
    newStream->first_dts = oldStream->first_dts;
    newStream->pts = oldStream->pts;
    newStream->time_base = oldStream->time_base;
    newStream->pts_wrap_bits = oldStream->pts_wrap_bits;
    newStream->discard = oldStream->discard;
    newStream->quality = oldStream->quality;
    newStream->start_time = oldStream->start_time;
    newStream->duration = oldStream->duration;
    newStream->nb_frames = oldStream->nb_frames;
    newStream->disposition = oldStream->disposition;
    newStream->sample_aspect_ratio = oldStream->sample_aspect_ratio;
    newStream->avg_frame_rate = oldStream->avg_frame_rate;
    // we're not sure this makes any difference but just to be nice
    newStream->stream_copy = 1;

    // copy codec from input file
    newStream->codec = avcodec_alloc_context();
    TRY_ASSERT_X(newStream->codec, "Could not allocate memory");
    avcodec_copy_context(newStream->codec, m_originalVideoFile->codec());
    // can't copy mjpeg without this
    newStream->codec->strict_std_compliance = FF_COMPLIANCE_UNOFFICIAL;
    newStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

    return newStream;
}

void ExportThread::open_video(AVFormatContext * /*container*/, AVStream *stream)
{
    AVCodecContext * codecContext = stream->codec;

    /* find the video encoder */
    AVCodec * codec = avcodec_find_encoder(codecContext->codec_id);
    TRY_ASSERT_X(codec, "Video codec not found");
    TRY_ASSERT_X((avcodec_open(codecContext, codec) >= 0),
                 "Could not open video codec");

}

void ExportThread::close_video(AVFormatContext */*oc*/, AVStream *st)
{
    avcodec_close(st->codec);
}

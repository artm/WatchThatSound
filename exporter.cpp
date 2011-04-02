#include "exporter.h"
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

Exporter::Exporter(QObject *parent)
    : QObject(parent)
    , m_originalVideoFile(0)
    , m_audio(0)
    , m_container(0)
    , m_videoStream(0)
    , m_audioStream(0)
{
}

void Exporter::configure(const QString& fname,
                             VideoFile * vfile,
                             const QList<WtsAudio::BufferAt *>& sequence,
                             WtsAudio * audio,
                             QProgressDialog * progress)
{
    m_filename = fname.toLocal8Bit();
    m_originalVideoFile = vfile;
    m_sequence = sequence;
    m_audio = audio;
    m_progress = progress;
}

void Exporter::initExport()
{
    AVOutputFormat * format = av_guess_format(NULL, m_filename.constData(), NULL);
    if (!format) {
        qWarning() << "Could not deduce output format "
                      "from file extension: using MOV (QuickTime).";
        format = av_guess_format("mov", NULL, NULL);
    }

    TRY_ASSERT_X(format, "Could not find suitable output format");

    m_container = avformat_alloc_context();
    TRY_ASSERT_X(m_container, "Memory error in ffmpeg");
    m_container->oformat = format;
    // apparently this is the normal ffmpeg way to set the filename
    snprintf(m_container->filename,
             sizeof(m_container->filename),
             "%s",
             m_filename.constData());

    if (format->audio_codec != CODEC_ID_NONE)
        initAudioStream(format->audio_codec);
    if (format->video_codec != CODEC_ID_NONE)
        initVideoStream();


    // set the output parameters (must be done even if no parameters)
    TRY_ASSERT_X((av_set_parameters(m_container, NULL) >= 0),
                 "Invalid output format parameters");

    av_dump_format(m_container, 0, m_filename.constData(), 1);

    AVCodec * audioCodec = avcodec_find_encoder(m_audioStream->codec->codec_id);
    TRY_ASSERT_X(audioCodec, "Could not find audio encoder");
    TRY_ASSERT_X((avcodec_open(m_audioStream->codec, audioCodec) >= 0),
                 "Could not open audio encoder");

    AVCodec * videoCodec = avcodec_find_encoder(m_videoStream->codec->codec_id);
    TRY_ASSERT_X(videoCodec, "Video codec not found");
    TRY_ASSERT_X((avcodec_open(m_videoStream->codec, videoCodec) >= 0),
                 "Could not open video codec");

    TRY_ASSERT_X( ( avio_open(&m_container->pb,
                              m_filename.constData(),
                              URL_WRONLY) >= 0 ),
                 QString("Could not open " + m_filename));

}

void Exporter::performExport()
{
    QVector<uint8_t> m_encodedAudio(10000);

    // this is copied from ffmpeg's output-example
    // original comment states that it's an "ugly hack for PCM codecs"
    int mixBufferSize = m_audioStream->codec->frame_size;
    if (m_audioStream->codec->frame_size <= 1) {
        mixBufferSize = m_encodedAudio.size() / m_audioStream->codec->channels;
        switch(m_audioStream->codec->codec_id) {
        case CODEC_ID_PCM_S16LE:
        case CODEC_ID_PCM_S16BE:
        case CODEC_ID_PCM_U16LE:
        case CODEC_ID_PCM_U16BE:
            mixBufferSize >>= 1;
            break;
        default:
            break;
        }
    }

    QVector<int16_t> m_mixBuffer(mixBufferSize * m_audioStream->codec->channels);

    av_write_header(m_container);

    m_originalVideoFile->seek(0);
    m_audio->samplerClear();
    qSort(m_sequence.begin(), m_sequence.end(), WtsAudio::startsBefore);
    QList<WtsAudio::BufferAt *>::iterator sequenceCursor = m_sequence.begin();

    double duration = (double)m_videoStream->duration
            * m_videoStream->time_base.num
            / m_videoStream->time_base.den;
    bool moreVideo = true, moreAudio = true;
    while(moreVideo || moreAudio) {
        double audio_pts = (double)m_audioStream->pts.val
                * m_audioStream->time_base.num
                / m_audioStream->time_base.den;
        double video_pts = (double)m_videoStream->pts.val
                * m_videoStream->time_base.num
                / m_videoStream->time_base.den;

        moreAudio = audio_pts < duration;

        double pts;

        AVPacket packet;
        if ( moreVideo
                && (!moreAudio || (video_pts < audio_pts))
                && (moreVideo = m_originalVideoFile->nextPacket(packet)) ) {
            packet.stream_index = m_videoStream->index;
            av_interleaved_write_frame(m_container, &packet);
            pts = video_pts;
        } else if (moreAudio) {

            av_init_packet(&packet);

            qint64 ms = (qint64)(audio_pts * 1000.0);

            while( sequenceCursor != m_sequence.end()
                  && (*sequenceCursor)->at() <= ms ) {
                m_audio->samplerSchedule(*sequenceCursor);
                sequenceCursor++;
            }

            m_audio->samplerMix( ms, m_mixBuffer);

            packet.size = avcodec_encode_audio(m_audioStream->codec,
                                               m_encodedAudio.data(),
                                               m_encodedAudio.size(),
                                               m_mixBuffer.data());
            if (m_audioStream->codec->coded_frame
                    && ( (uint64_t) m_audioStream->codec->coded_frame->pts
                        != AV_NOPTS_VALUE) )
                packet.pts= av_rescale_q(m_audioStream->codec->coded_frame->pts,
                                         m_audioStream->codec->time_base,
                                         m_audioStream->time_base);
            packet.flags |= AV_PKT_FLAG_KEY;
            packet.stream_index= m_audioStream->index;
            packet.data= m_encodedAudio.data();
            av_interleaved_write_frame(m_container, &packet);

            pts = audio_pts;
        }

        m_progress->setValue(100 * pts / duration);
    }

    av_write_trailer(m_container);
}

void Exporter::run()
{
    try {
        m_progress->setValue(0);

        initExport();
        performExport();
        finishUp();

        m_progress->setValue(100);
    } catch (const AssertFailed& e) {
        qCritical() << e.cMessage();
    }
}

void Exporter::finishUp()
{
    avcodec_close(m_videoStream->codec);
    avcodec_close(m_audioStream->codec);

    /* free the resources */
    for(unsigned i = 0; i < m_container->nb_streams; i++) {
        av_freep(&m_container->streams[i]->codec);
        av_freep(&m_container->streams[i]);
    }

    avio_close(m_container->pb);
    av_free(m_container);
}

void Exporter::initAudioStream(CodecID codec_id)
{
    AVCodecContext * codecContext;

    m_audioStream = av_new_stream(m_container, 1);
    TRY_ASSERT_X(m_audioStream, "Could not alloc stream");

    codecContext = m_audioStream->codec;
    codecContext->codec_id = codec_id;
    codecContext->codec_type = AVMEDIA_TYPE_AUDIO;

    /* put sample parameters */
    codecContext->sample_fmt = AV_SAMPLE_FMT_S16;
    codecContext->bit_rate = 64000;
    codecContext->sample_rate = 44100;
    codecContext->channels = 1;

    // some formats want stream headers to be separate
    if(m_container->oformat->flags & AVFMT_GLOBALHEADER)
        codecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
}

void Exporter::initVideoStream()
{
    // Video output stream is pretty much a copy of the input video
    m_videoStream = av_new_stream(m_container, 0);
    TRY_ASSERT_X(m_videoStream, "Could not allocate memory");

    const AVStream * oldStream = m_originalVideoFile->stream();
    // we copy some fields but leave other as they were initialized by
    // av_new_stream() above. this way we're sure that no pointers to
    // malloced memory get copied and later freed twice.
    m_videoStream->r_frame_rate = oldStream->r_frame_rate;
    m_videoStream->first_dts = oldStream->first_dts;
    m_videoStream->pts = oldStream->pts;
    m_videoStream->time_base = oldStream->time_base;
    m_videoStream->pts_wrap_bits = oldStream->pts_wrap_bits;
    m_videoStream->discard = oldStream->discard;
    m_videoStream->quality = oldStream->quality;
    m_videoStream->start_time = oldStream->start_time;
    m_videoStream->duration = oldStream->duration;
    m_videoStream->nb_frames = oldStream->nb_frames;
    m_videoStream->disposition = oldStream->disposition;
    m_videoStream->sample_aspect_ratio = oldStream->sample_aspect_ratio;
    m_videoStream->avg_frame_rate = oldStream->avg_frame_rate;
    // we're not sure this makes any difference but just to be nice
    m_videoStream->stream_copy = 1;

    // copy codec from input file
    m_videoStream->codec = avcodec_alloc_context();
    TRY_ASSERT_X(m_videoStream->codec, "Could not allocate memory");
    avcodec_copy_context(m_videoStream->codec, m_originalVideoFile->codec());
    // can't copy mjpeg without this
    m_videoStream->codec->strict_std_compliance = FF_COMPLIANCE_UNOFFICIAL;
    m_videoStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

    // otherwise we crash on non-square pixels (first world trouble)
    m_videoStream->codec->sample_aspect_ratio = m_videoStream->sample_aspect_ratio;

}

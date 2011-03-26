#include "wtsaudio.h"
#include "soundbuffer.h"

#include <QDebug>

struct PortAudioLogger {
    bool operator<<(PaError error) {
        if (error != paNoError) {
            qCritical() << Pa_GetErrorText(error);
            return false;
        }
        return true;
    }
};

template<typename T> T audioCast(float v) { return (T)v; }
template<> int16_t audioCast<int16_t>(float v) { return  (int16_t) (v * 32768.0); }

WtsAudio::WtsAudio(QObject *parent) :
    QObject(parent), m_stream(0)
{
    Pa_Initialize();

    PaStreamParameters iParams, oParams;

    iParams.device = Pa_GetDefaultInputDevice();
    iParams.channelCount = 1;
    iParams.sampleFormat = paFloat32;
    iParams.suggestedLatency = 0;

    oParams.device = Pa_GetDefaultOutputDevice();
    oParams.channelCount = 1;
    oParams.sampleFormat = paFloat32;
    oParams.suggestedLatency = 0;

    PortAudioLogger() << Pa_OpenStream( &m_stream, &iParams, &oParams, 44100, 2048, paClipOff , 0, 0);
}

WtsAudio::~WtsAudio()
{
    if (m_stream) {
        Pa_CloseStream(m_stream);
        m_stream = 0;
    }
    Pa_Terminate();
}

void WtsAudio::start()
{
    Q_ASSERT(m_stream);
    PortAudioLogger() << Pa_StartStream(m_stream);
}

void WtsAudio::stop()
{
    Q_ASSERT(m_stream);
    PortAudioLogger() << Pa_StopStream(m_stream);
}

qint64 WtsAudio::capture(SoundBuffer * buffer)
{
    Q_ASSERT(m_stream);
    qint64 availSamples = Pa_GetStreamReadAvailable(m_stream);
    if (availSamples) {
        float * chunk = buffer->chunkToWrite(availSamples);
        Pa_ReadStream(m_stream, chunk, availSamples);
    }
    return availSamples;
}

void WtsAudio::samplerClock(qint64 ms)
{
    QVector<float> mix(Pa_GetStreamWriteAvailable(m_stream));
    samplerMix (ms, mix);
    Pa_WriteStream(m_stream, mix.data(), mix.size());
}

void WtsAudio::samplerMix(qint64 ms, QVector<float>& mix)
{
    m_clock = ms;
    mix.fill(0);

    QLinkedList< WtsAudio::BufferAt * >::iterator
            nextBufferIt = m_activeBuffers.begin();
    while( nextBufferIt != m_activeBuffers.end() ) {
        WtsAudio::BufferAt * buffer = *nextBufferIt;
        qint64 startRead = buffer->playOffset();
        qint64 startWrite = 0;
        if (startRead < 0) {
            startWrite -= startRead;
            startRead = 0;
        }

        qint64 count = std::min(mix.size() - startWrite,
                                buffer->buffer()->sampleCount() - startRead);
        float * in = buffer->buffer()->floatAt( startRead );
        for(int i = 0; i<count; ++i) {
            mix[ startWrite+i ] += in[i];
        }
        buffer->setPlayOffset(startRead + count);

        if (buffer->playOffset() == buffer->buffer()->sampleCount())
            // deactivate buffer...
            nextBufferIt = m_activeBuffers.erase(nextBufferIt);
        else
            ++nextBufferIt;
    }
}

void WtsAudio::samplerMix(qint64 ms, QVector<int16_t>& mix)
{
    QVector<float> fmix(mix.size());
    samplerMix(ms, fmix);
    for(int i=0; i<fmix.size(); ++i) {
        mix[i] = audioCast<int16_t>(fmix[i]);
    }
}

void WtsAudio::samplerSchedule(WtsAudio::BufferAt * buffer)
{
    qint64 t = currentSampleOffset();
    qint64 bt = msToSampleCount(buffer->at());

    // ignore if end time before current time
    if (bt + buffer->buffer()->sampleCount() < t)
        return;

    // position buffer's playback offset
    buffer->setPlayOffset(t - bt);

    // add buffer to the list...
    m_activeBuffers << buffer;
}

void WtsAudio::samplerClear()
{
    m_activeBuffers.clear();
    m_clock = 0;
}

qint64 WtsAudio::currentSampleOffset() const
{
    if (m_activeBuffers.empty()) {
        return msToSampleCount(m_clock);
    } else {
        WtsAudio::BufferAt * buffer = m_activeBuffers.first();
        return msToSampleCount(buffer->at()) + buffer->playOffset();
    }
}

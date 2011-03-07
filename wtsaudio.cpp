#include "wtsaudio.h"
#include "soundbuffer.h"

#include <QDebug>

struct PortAudioLogger {
    bool operator<<(PaError error) {
        if (error != paNoError) {
            qDebug() << Pa_GetErrorText(error);
            return false;
        }
        return true;
    }
};

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
    m_clock = ms;
    qint64 mixSize = Pa_GetStreamWriteAvailable(m_stream);
    float mix[ mixSize ];
    std::fill(mix, mix+mixSize, 0.0);

    QLinkedList< WtsAudio::BufferAt * >::iterator nextBufferIt = m_activeBuffers.begin();
    while( nextBufferIt != m_activeBuffers.end() ) {
        WtsAudio::BufferAt * buffer = *nextBufferIt;
        qint64 startRead = buffer->playOffset();
        qint64 startWrite = 0;
        if (startRead < 0) {
            startWrite -= startRead;
            startRead = 0;
        }

        qint64 count = std::min(mixSize - startWrite, buffer->buffer()->sampleCount() - startRead);
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

    Pa_WriteStream(m_stream, mix, mixSize);
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

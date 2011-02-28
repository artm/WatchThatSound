#ifndef WTSAUDIO_H
#define WTSAUDIO_H

#include <portaudio.h>
#include <QObject>
#include <QLinkedList>

class SoundBuffer;

class WtsAudio : public QObject
{
    Q_OBJECT
public:
    struct BufferAt {
        SoundBuffer * m_buffer;
        qint64 m_at;
        qint64 m_playOffset;

        BufferAt() : m_buffer(0), m_at(0), m_playOffset(0) {}
        BufferAt(SoundBuffer * buffer, qint64 at) : m_buffer(buffer), m_at(at), m_playOffset(0) {}

        bool operator<(const BufferAt& other) const { return m_at < other.m_at; }
    };

    static bool startsBefore( const BufferAt * a, const BufferAt * b ) { return *a < *b; }

    explicit WtsAudio(QObject *parent = 0);
    virtual ~WtsAudio();

    qint64 capture(SoundBuffer * buffer);
    qint64 currentSampleOffset() const;

    static qint64 samplingRate() { return 44100; }
    static int channelCount() { return 1; }
    static qint64 msToSampleCount(qint64 ms) { return samplingRate() * channelCount() * ms / 1000; }
    static qint64 sampleCountToMs(qint64 count) { return count * 1000 / (samplingRate() * channelCount()); }
    static qint64 byteToSampleCount(qint64 byteCount) { return byteCount / sizeof(float); }
signals:

public slots:
    void start();
    void stop();

    void samplerClock(qint64 ms);
    void samplerSchedule(WtsAudio::BufferAt * buffer);
    void samplerClear();


protected:
    PaStream * m_stream;
    qint64 m_clock;
    QLinkedList< WtsAudio::BufferAt * > m_activeBuffers;
};

#endif // WTSAUDIO_H

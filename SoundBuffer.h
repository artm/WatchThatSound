#ifndef SOUNDBUFFER_H
#define SOUNDBUFFER_H

#include <QVector>
#include <QFile>
#include <QtMultimedia>
#include <QIODevice>
#include <QColor>

class SoundBuffer
{
public:
    SoundBuffer();
    SoundBuffer(qint64 sampleCount);
    SoundBuffer(const QString& name, const SoundBuffer& other, qint64 sampleCount = 0);
    SoundBuffer& operator= (const SoundBuffer& other);

    // in ms
    qint64 duration() const;
    // in samples
    qint64 sampleCount() const { return m_data.size(); }

    // float * by number ...
    float* floatAt(qint64 pos);
    const float* floatAt(qint64 pos) const;
    float* floatAtWritePos() { return floatAt(m_writePos); }
    float* chunkToWrite(qint64& size);

    qint64 freeToWrite() const { return m_data.size() - m_writePos; }
    void setWritePos(qint64 pos) { m_writePos = std::min(pos, (qint64)m_data.size()); }

    void paste(const SoundBuffer * other);
    QString name() const { return m_name; }

    void save(QFile& file);
    void load(QFile& file);

    QColor color() const { return m_color; }
    void setColor(QColor color) { m_color = color; }

    void setRange(qint64 from, qint64 to) { setRangeStart(from); setRangeEnd(to); }
    void setRangeStart(qint64 from) { m_range[0] = std::max(from, 0ll); }
    void setRangeEnd(qint64 to) { m_range[1] = std::min(to, sampleCount()); }

    qint64 rangeStart() const { return m_range[0]; }
    qint64 rangeEnd() const { return m_range[1]; }
    qint64 rangeLength() const { return m_range[1]-m_range[0]; }

    void setGain(float gain) { m_gain = gain; }
    float gain() const { return m_gain; }
    float normGain() const { return m_normGain; }
    void initGains();

protected:
    QString m_name;
    bool m_saved;

    QVector<float> m_data;
    QColor m_color;

    qint64 m_range[2];

public:
    // fixme
    qint64 m_writePos, m_readPos;

protected:
    float m_normGain, m_gain;
};

#endif // SOUNDBUFFER_H

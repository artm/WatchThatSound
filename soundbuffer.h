#ifndef SOUNDBUFFER_H
#define SOUNDBUFFER_H

#include <QByteArray>
#include <QFile>
#include <QtMultimedia>
#include <QIODevice>
#include <QColor>

#include "wtsaudio.h"

class SoundBuffer
{
public:
    SoundBuffer();
    SoundBuffer(qint64 sampleCount);
    SoundBuffer(const QString& name, const SoundBuffer& other, qint64 sampleCount = 0);
    SoundBuffer& operator= (const SoundBuffer& other);

    // in ms
    qint64 duration() const { return WtsAudio::sampleCountToMs(m_sampleCount); }
    // in samples
    qint64 sampleCount() const { return m_sampleCount; }

    // float * by number ...
    float* floatAt(qint64 pos);
    const float* floatAt(qint64 pos) const;
    float* floatAtWritePos() { return floatAt(m_writePos); }
    float* chunkToWrite(qint64& size);

    qint64 freeToWrite() const { return m_sampleCount - m_writePos; }
    void setWritePos(qint64 pos) { m_writePos = std::min(pos, m_sampleCount); }

    void paste(const SoundBuffer * other);
    QString name() const { return m_name; }

    void save(QFile& file);
    void load(QFile& file);

    QColor color() const { return m_color; }
    void setColor(QColor color) { m_color = color; }

protected:
    QString m_name;
    bool m_saved;

    QByteArray m_data;
    qint64 m_sampleCount;
    QColor m_color;

public:
    // fixme
    qint64 m_writePos, m_readPos;
};

#endif // SOUNDBUFFER_H

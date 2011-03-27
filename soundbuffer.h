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

protected:
    QString m_name;
    bool m_saved;

    QVector<float> m_data;
    QColor m_color;

public:
    // fixme
    qint64 m_writePos, m_readPos;
};

#endif // SOUNDBUFFER_H

#ifndef SOUNDBUFFER_H
#define SOUNDBUFFER_H

#include "stable.h"
#include "Exception.h"

namespace WTS {

class SoundBuffer
{
public:
    SoundBuffer(const QString& name = QString("scratch"));
    SoundBuffer(qint64 sampleCount, const QString& name = QString("scratch"));
    SoundBuffer(const QString& name, const SoundBuffer& other, qint64 sampleCount = 0);
    SoundBuffer& operator= (const SoundBuffer& other);

    class FileNotFoundError : public Exception {
    public:
        FileNotFoundError(const QString& path)
            : Exception( QString("File not found: %0").arg(path) ) {}

        static void test(const QString& path) {
            if (!QFile(path).exists())
                throw FileNotFoundError(path);
        }
        static void test(const QFile& file) {
            if (!file.exists())
                throw FileNotFoundError(file.fileName());
        }
    };

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
    QString savedAs() const { return m_savedAs; }

    static QString makeFileName(const QString& name);
    void save( const QDir& dir );
    void load( const QDir& dir );
    // return true if sample has changed
    bool maybeReload( const QDir& dir );

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

    static const float s_minScaleMax = 0.1;
    float draw(QPixmap& surface, bool recording = false, float scaleMax = s_minScaleMax);

protected:
    QString m_name, m_savedAs;
    bool m_saved;

    QVector<float> m_data;
    QColor m_color;

    qint64 m_range[2];

    QDateTime m_timestamp;

public:
    // fixme
    qint64 m_writePos, m_readPos;

protected:
    float m_normGain, m_gain;
};

}

#endif // SOUNDBUFFER_H

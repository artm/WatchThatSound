#include "SoundBuffer.h"
#include "WtsAudio.h"

#include <cmath>
#include <QFileInfo>

#include <sndfile.hh>

using namespace WTS;

SoundBuffer::SoundBuffer()
    : m_name("scratch")
    , m_saved(true)
    , m_writePos(0)
    , m_readPos(0)
    , m_normGain(1)
    , m_gain(1)
{
    setRange(0,0);
}

SoundBuffer::SoundBuffer(qint64 sampleCount)
    : m_name("scratch")
    , m_saved(true)
    , m_data(sampleCount, 0)
    , m_writePos(0)
    , m_readPos(0)
    , m_normGain(1)
    , m_gain(1)
{
    setRange(0,sampleCount);
}

SoundBuffer::SoundBuffer(const QString& name, const SoundBuffer& other, qint64 sampleCount)
    : m_name(name)
    , m_saved(false)
    , m_data(other.m_data)
    , m_writePos(0)
    , m_readPos(0)
    , m_normGain(1)
    , m_gain(1)
{
    if (sampleCount && sampleCount != other.sampleCount())
        m_data.resize(sampleCount);
    setRange(0,sampleCount);
}

SoundBuffer& SoundBuffer::operator= (const SoundBuffer& other)
{
    m_data = other.m_data;
    m_name = other.m_name;
    m_saved = other.m_saved;
    m_writePos = 0;
    m_readPos = 0;
    m_range[0] = other.m_range[0];
    m_range[1] = other.m_range[1];
    m_normGain = other.m_normGain;
    m_gain = other.m_gain;
    return *this;
}

void SoundBuffer::save(const QString& path)
{
    if (m_saved)
        return;

    qDebug() << "Save sample to" << path;
    { // block to ensure file is closed by the moment we ask its modification time
        SndfileHandle snd(qPrintable(path),SFM_WRITE,
                SF_FORMAT_WAV | SF_FORMAT_FLOAT,
                1, // channel
                WtsAudio::samplingRate());

        snd.writef(m_data.data(),m_data.size());
    }
    m_timestamp = QFileInfo(path).lastModified();
    m_saved = true;
}

void SoundBuffer::load(const QString& path)
{
    // load either wav or raw
    if (path.contains(QRegExp("\\.wav$")) && QFile(path).exists()) {
        // load wav
        SndfileHandle snd(qPrintable(path));
        m_data.resize(snd.frames());
        snd.readf(m_data.data(),m_data.size());
        // only samples read from wav files are considered saved
        m_saved = true;
    } else {
        // load raw
        QString rawpath(path);
        rawpath.replace(QRegExp("[^\\.]+$"),"raw");
        qDebug() << "Upgrading sample from " << rawpath;
        QFile file(rawpath);

        FileNotFoundError::test(file);

        file.open(QFile::ReadOnly);
        int64_t sampleCount = file.size() / sizeof(float);
        m_data.resize(sampleCount);
        file.read(reinterpret_cast<char *>(m_data.data()),
                sampleCount*sizeof(float));
        file.close();
    }

    m_readPos = 0;
    m_writePos = 0;
    m_name = QFileInfo(path).fileName();
    m_timestamp = QFileInfo(path).lastModified();
}

bool SoundBuffer::maybeReload(const QString& path)
{
    if (QFileInfo(path).lastModified() > m_timestamp) {
        qDebug() << "Sample " << m_name << " has changed on disk";

        qint64 old_length = m_data.size();

        load(path);
        /*
         * if length has changed - select all
         */
        if (m_data.size() != old_length) {
            m_range[0] = 0;
            m_range[1] = m_data.size()-1;
        }
        return true;
    } else
        return false;
}


float* SoundBuffer::floatAt(qint64 pos)
{
    return m_data.data() + pos;
}

const float* SoundBuffer::floatAt(qint64 pos) const
{
    return m_data.constData() + pos;
}

void SoundBuffer::paste(const SoundBuffer * other)
{
    qint64 pasteSize = std::min(sampleCount() - m_writePos, other->sampleCount());
    qCopy( other->m_data.begin(), other->m_data.begin() + pasteSize, m_data.begin() );
    m_writePos += pasteSize;
}

float* SoundBuffer::chunkToWrite(qint64& size) {
    size = std::min(size, sampleCount() - m_writePos);
    float * chunk = floatAt(m_writePos);
    m_writePos += size;
    return chunk;
}

qint64 SoundBuffer::duration() const
{
    return WtsAudio::sampleCountToMs(m_data.size());
}

void SoundBuffer::initGains()
{
    float peak = 0.0;
    foreach(float v, m_data) {
        if (fabs(v) > peak) {
            peak = v;
        }
    }
    m_normGain = 1.0 / peak;
    m_gain = peak;
    // What?
}

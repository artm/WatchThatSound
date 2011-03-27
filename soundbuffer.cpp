#include <QtCore>
#include <algorithm>
#include "soundbuffer.h"
#include "wtsaudio.h"

SoundBuffer::SoundBuffer()
    : m_name("scratch")
    , m_saved(true)
    , m_writePos(0)
    , m_readPos(0)
{
}

SoundBuffer::SoundBuffer(qint64 sampleCount)
    : m_name("scratch")
    , m_saved(true)
    , m_data(sampleCount, 0)
    , m_writePos(0)
    , m_readPos(0)
{
}

SoundBuffer::SoundBuffer(const QString& name, const SoundBuffer& other, qint64 sampleCount)
    : m_name(name)
    , m_saved(false)
    , m_data(other.m_data)
    , m_writePos(0)
    , m_readPos(0)
{
    if (sampleCount && sampleCount != other.sampleCount())
        m_data.resize(sampleCount);
}

SoundBuffer& SoundBuffer::operator= (const SoundBuffer& other)
{
    m_data = other.m_data;
    m_name = other.m_name;
    m_saved = other.m_saved;
    m_writePos = 0;
    m_readPos = 0;
    return *this;
}

void SoundBuffer::save(QFile& file)
{
    if (m_saved)
        return;
    file.open(QFile::WriteOnly);
    file.write( reinterpret_cast<const char *>(m_data.constData()),
               m_data.size()*sizeof(float) );
    m_saved = true;
    file.close();
}

void SoundBuffer::load(QFile& file)
{
    file.open(QFile::ReadOnly);
    int64_t sampleCount = file.size() / sizeof(float);
    m_data.resize(sampleCount);
    file.read(reinterpret_cast<char *>(m_data.data()),
              sampleCount*sizeof(float));
    m_readPos = 0;
    m_writePos = 0;
    m_name = QFileInfo(file.fileName()).fileName();
    m_saved = true;
    file.close();
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

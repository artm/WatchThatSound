#include <QtCore>
#include <algorithm>
#include "soundbuffer.h"

SoundBuffer::SoundBuffer()
    : m_name("scratch")
    , m_saved(true)
    , m_data()
    , m_sampleCount(0)
    , m_writePos(0)
    , m_readPos(0)
{
}

SoundBuffer::SoundBuffer(qint64 sampleCount)
    : m_name("scratch")
    , m_saved(true)
    , m_data(sampleCount * sizeof(float), 0)
    , m_sampleCount(sampleCount)
    , m_writePos(0)
    , m_readPos(0)
{
}

SoundBuffer::SoundBuffer(const QString& name, const SoundBuffer& other, qint64 sampleCount) :
        m_name(name),
        m_saved(false),
        m_data(other.m_data),
        m_sampleCount(sampleCount ? sampleCount : other.m_sampleCount),
        m_writePos(0), m_readPos(0)
{
    if (m_sampleCount != other.m_sampleCount)
        m_data.resize(m_sampleCount * sizeof(float));
}

SoundBuffer& SoundBuffer::operator= (const SoundBuffer& other)
{
    m_sampleCount = other.m_sampleCount;
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
    file.write(m_data);
    m_saved = true;
    file.close();
}

void SoundBuffer::load(QFile& file)
{
    file.open(QFile::ReadOnly);
    m_data = file.readAll();
    m_sampleCount = WtsAudio::byteToSampleCount(m_data.size());
    m_readPos = 0;
    m_writePos = 0;
    m_name = QFileInfo(file.fileName()).fileName();
    m_saved = true;
    file.close();
}


float* SoundBuffer::floatAt(qint64 pos)
{
    return ((float*)m_data.data()) + pos;
}

const float* SoundBuffer::floatAt(qint64 pos) const
{
    return ((float*)m_data.data()) + pos;
}

void SoundBuffer::paste(const SoundBuffer * other)
{
    qint64 pasteSize = std::min(m_sampleCount - m_writePos, other->m_sampleCount);
    m_data.replace(m_writePos * sizeof(float), pasteSize * sizeof(float), other->m_data);
    m_writePos += pasteSize;
}

float* SoundBuffer::chunkToWrite(qint64& size) {
    size = std::min(size, m_sampleCount - m_writePos);
    float * chunk = floatAt(m_writePos);
    m_writePos += size;
    return chunk;
}


#include "stable.h"

#include "SoundBuffer.h"
#include "WtsAudio.h"

#include <cmath>

#include <sndfile.hh>

using namespace WTS;

SoundBuffer::SoundBuffer(const QString& name)
    : m_name(name)
    , m_saved(true)
    , m_writePos(0)
    , m_readPos(0)
    , m_normGain(1)
    , m_gain(1)
{
    setRange(0,0);
}

SoundBuffer::SoundBuffer(qint64 sampleCount, const QString& name)
    : m_name(name)
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

void SoundBuffer::save( const QDir& dir )
{
    QString path = dir.filePath( makeFileName( m_name ) );

    if (m_savedAs != path) {
        QFileInfo fi(m_savedAs);
        if (fi.exists()) {
            if (m_saved && fi.dir().rename( m_savedAs, path )) {
                qDebug() << "Renamed" << m_savedAs << "to" << path;
                m_savedAs = path;
                return;
            } else
                fi.dir().remove( fi.fileName() );
        }
    }

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
    m_savedAs = path;
}

void SoundBuffer::load( const QDir& dir )
{
    QString path = dir.filePath( makeFileName( m_name ) );

    qDebug() << "loading sample from" << path;

    if (QFile(path).exists()) {
        SndfileHandle snd(qPrintable(path));
        m_data.resize(snd.frames());
        snd.readf(m_data.data(),m_data.size());
        m_saved = true;
        m_savedAs = path;
    }

    m_readPos = 0;
    m_writePos = 0;
    m_timestamp = QFileInfo(path).lastModified();
}

bool SoundBuffer::maybeReload( const QDir& dir )
{
    QString path = dir.filePath( makeFileName( m_name ) );

    if (QFileInfo(path).lastModified() > m_timestamp) {
        qDebug() << "Sample " << m_name << " has changed on disk";

        qint64 old_length = m_data.size();

        load(dir);
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

float WTS::SoundBuffer::draw(QPixmap& surface, bool recording, float scaleMax)
{
    QPainter painter(&surface);
    painter.setPen(QColor(0,0,0,100));
    painter.fillRect(0,0,surface.width(),surface.height(), color() );
    int midY = surface.height()/2;

    qint64 stride = 1;
    const float * val = floatAt(0);
    qint64 lineCount =
            std::min((qint64)surface.width(),
                     recording ? m_writePos : sampleCount());
    QVector<float> Y(lineCount, 0.0);

    if (recording) {
        val = floatAt(m_writePos - lineCount);
    } else {
        // show the whole thing
        stride = sampleCount() / lineCount;
    }

    for(int x = 0; x<lineCount; ++x) {
        Y[x] = val[x*stride];
        for(int i=1; i<stride; ++i) {
            Y[x] = std::max(Y[x], (float)fabs(val[x*stride+i]));
        }
        scaleMax = std::max(Y[x],scaleMax);
    }

    for(int x = 0; x<lineCount; ++x) {
        int y1 = midY + (0.45 * (Y[x]/scaleMax) * (float)surface.height());
        int y2 = midY - (0.45 * (Y[x]/scaleMax) * (float)surface.height());
        painter.drawLine(x, y1, x, y2);
    }

    painter.drawLine(lineCount, midY, surface.width(), midY);
    return scaleMax;
}

QString WTS::SoundBuffer::makeFileName(const QString &name)
{
    QString badsyms = "[^a-z0-9_]+";
    QRegExp lead( "^"+ badsyms, Qt::CaseInsensitive );
    QRegExp tail( badsyms + "$", Qt::CaseInsensitive );
    QRegExp  mid( badsyms, Qt::CaseInsensitive );
    QString result = name;
    result.remove(lead).remove(tail).replace(mid,"_").append(".wav");
    return result;
}

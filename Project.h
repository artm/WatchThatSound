#pragma once
#include "Synced.h"
#include "VideoFile.h"
#include "WtsAudio.h"

namespace WTS {

/** Project data container.
 * Eventually all project specific data should end up here.
 */
class Project : public QObject {
    Q_OBJECT
public:
    enum MarkerType {
        NONE,
        EVENT,
        SCENE,
        ANY,
    };

    class Marker : public WTS::Synced {
    protected:
        MarkerType m_type;
        QPixmap m_snapshot;

        float m_tension;

    public:
        Marker(MarkerType type, qint64 ms, QObject * parent)
            : WTS::Synced(ms, parent)
            , m_type(type)
            , m_tension(0.5)
        {}

        MarkerType type() const { return m_type; }
        const QPixmap& snapshot() const { return m_snapshot; }
        void setSnapshot(const QPixmap& snapshot) { m_snapshot = snapshot; }

        float tension() const { return m_tension; }
        void setTension(float value) { m_tension = value; }
    };

    Project(const QString& path, QObject * parent = 0);

    double finalTension() const { return m_finalTension; }
    void addMarker(MarkerType type, qint64 when, float tension);
    void removeMarkerAt(quint64 at);
    QList<Marker *> getMarkers(MarkerType type = ANY, bool forward = true) const;
    QPainterPath tensionCurve(float width);
    qint64 duration() const { return m_videoFile->duration(); }
    int videoWidth() const { return m_videoFile->width(); }
    int videoHeight() const { return m_videoFile->height(); }
    VideoFile * videoFile() { return m_videoFile; }
    const VideoFile * videoFile() const { return m_videoFile; }
    static QDir movDir();
    QDir dataDir() const { return m_dataDir; }
    QString moviePath() const { return videoFile()->path(); }
    QString movieFilename() const { return QFileInfo(moviePath()).fileName(); }
    void addBufferAt(WtsAudio::BufferAt * newBuff);
    void copyScratch(WtsAudio::BufferAt * newBuff);
    void removeBufferAt(WtsAudio::BufferAt * newBuff);
    QString samplePath(const QString& sampleName);
    QString samplePath(SoundBuffer *);

    // TODO split this off into a controller
    void seek(qint64 ms);
    void start();
    void advanceSequenceCursor(qint64 ms);
    QList<WtsAudio::BufferAt *>::iterator beginCursor();
    QList<WtsAudio::BufferAt *>::iterator endCursor() { return m_sequence.end(); }

public slots:
    // see if samples changed on disk and reload if so
    void reScanSamples();
    void openInExternalApp(SoundBuffer*);
    void setFinalTension(double value) { m_finalTension = value; }
    void setMarkerTension(int markerIndex, float tension);

    void save();
    void load();



signals:
    void samplerSchedule(WtsAudio::BufferAt * buffer);
    void newBufferAt(WtsAudio::BufferAt * bufferAt);

    void saveSection(QXmlStreamWriter& writer);
    void loadSection(QXmlStreamReader& reader);

    // changes
    void sampleChanged(SoundBuffer * sample);
    void tensionChanged();

protected slots:
    void saveStoryboard(QXmlStreamWriter&);
    void saveSequence(QXmlStreamWriter&);
    bool loadStoryboard(QXmlStreamReader&);
    bool loadSequence(QXmlStreamReader&);

protected:
    QString makeSampleName();

    bool m_loading;
    double m_finalTension;
    QMap<qint64, Project::Marker *> m_markers;
    VideoFile * m_videoFile;
    QDir m_dataDir;
    QList<WtsAudio::BufferAt *> m_sequence;
    int m_lastSampleNameNum;
    // TODO the cursor should be a part of controller (playback / export) not
    // model
    QList<WtsAudio::BufferAt *>::iterator m_sequenceCursor;

    static QDir s_movDir;
    static bool s_movDirFound;
};

}

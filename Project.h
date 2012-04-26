#pragma once
#include "Synced.h"
#include "VideoFile.h"
#include "WtsAudio.h"


namespace WTS {

class ScoreSymbol;

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

    // exceptions
    struct InvalidProject {};
    struct CantChangeDuration {};

    // testing helpers
    explicit Project(QObject * parent = 0);
    void setDuration(qint64 duration);
    bool isValid() const;

    // normal api
    Project(const QString& path, QObject * parent = 0);
    double finalTension() const { return m_finalTension; }
    void addMarker(MarkerType type, qint64 when, float tension);
    QList<Marker *> getMarkers(MarkerType type = ANY, bool forward = true) const;
    QPainterPath tensionCurve(float width, float height = 1.0f, Marker * from = 0, Marker * to = 0);
    qint64 duration() const {
        throwIfInvalid();
        return m_videoFile ? m_videoFile->duration() : m_duration; }
    int videoWidth() const {
        throwIfInvalid();
        return m_videoFile->width();
    }
    int videoHeight() const {
        throwIfInvalid();
        return m_videoFile->height();
    }
    VideoFile * videoFile() {
        return m_videoFile;
    }
    const VideoFile * videoFile() const { return m_videoFile; }
    static QDir movDir();
    QDir dataDir() const { return m_dataDir; }
    QString moviePath() const { return videoFile()->path(); }
    QString movieFilename() const { return QFileInfo(moviePath()).fileName(); }
    void addBufferAt(WtsAudio::BufferAt * newBuff);
    WtsAudio::BufferAt *copyScratch(WtsAudio::BufferAt * newBuff);
    QString samplePath(const QString& sampleName);
    QString samplePath(SoundBuffer *);
    QString pdfPath();

    QList<WtsAudio::BufferAt *> getSequence() const { return m_sequence; }

public slots:
    // see if samples changed on disk and reload if so
    void reScanSamples();
    void openInExternalApp(SoundBuffer*);
    void setFinalTension(double value) { m_finalTension = value; }
    void setMarkerTension(int markerIndex, float tension);
    void removalRequested(WTS::Synced* synced);

    void save();
    void load();
    void print(QPrinter&, const QList<WTS::ScoreSymbol *>&);

signals:

    void saveSection(QXmlStreamWriter& writer);
    void loadSection(QXmlStreamReader& reader);

    // changes
    void sampleChanged(SoundBuffer * sample);
    void newBufferAt(WtsAudio::BufferAt * bufferAt);
    void syncedItemRemoved(WTS::Synced * synced);
    void tensionChanged();
    void storyBoardChanged();

protected slots:
    void saveStoryboard(QXmlStreamWriter&);
    void saveSequence(QXmlStreamWriter&);
    bool loadStoryboard(QXmlStreamReader&);
    bool loadSequence(QXmlStreamReader&);

protected:
    QString makeSampleName();
    void removeMarkerAt(quint64 at);
    void removeBufferAt(WtsAudio::BufferAt * newBuff);

    void drawTimeScale( qint64 start, qint64 end, QPainter& painter, const QRect& target );
    void drawSceneThumbs( Marker * sceneMarker, qint64 start, qint64 end, QPainter& painter, const QRect& target );
    void drawScore( const QList<WTS::ScoreSymbol *>& score, qint64 start, qint64 end, QPainter& painter, const QRect& target );

    bool m_loading;
    double m_finalTension;
    QMap<qint64, Project::Marker *> m_markers;
    VideoFile * m_videoFile;
    QDir m_dataDir;
    QList<WtsAudio::BufferAt *> m_sequence;
    int m_lastSampleNameNum;
    static QDir s_movDir;
    static bool s_movDirFound;

    // print style
    QPen m_markerPen;

private:
    void setup();
    void throwIfInvalid() const;
    qint64 m_duration;
};

}

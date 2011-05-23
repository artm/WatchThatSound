#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <Phonon>

#include "wtsaudio.h"
#include "soundbuffer.h"
#include "videofile.h"
#include "synced.h"
#include "Preferences.h"

namespace Ui
{
    class MainWindow;
}

class Exporter;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum MarkerType {
        NONE,
        EVENT,
        SCENE,
        ANY,
    };

    class Marker : public WTS::Synced
    {
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

    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QList<Marker *> getMarkers(MarkerType type = ANY, bool forward = true) const;
    void addMarker(MarkerType type, qint64 when = -1, float tension = 0.5);
    Phonon::MediaObject * mediaObject();
    QState * addPage(const QString& name, QList<QWidget*> widgets, QList<QAction*> actions = QList<QAction*>());

    void buildMovieSelector();

    QPainterPath tensionCurve() const;
    void removeMark(Marker * m);
    void removeBuffer(WtsAudio::BufferAt * bufferAt);

public slots:
    void setFullscreen(bool fs);
    void onPlay(bool play);
    void onRecord(bool record);
    void tick(qint64 ms);
    void seek(qint64 ms);

    void onMovieFinished();

    void addSceneMark(){ addMarker(SCENE); }
    void addEventMark(){ addMarker(EVENT); }

    void loadMovie(const QString& path);

    void resetData();
    void saveData();
    void loadData();

    void exportMovie();

    void refreshTension();
    void updateMarkerTension(int markerIndex, float tension);
    void setMuteOnRecord(bool on) { m_muteOnRecord = on; }

signals:
    void storyBoardChanged();
    void newBufferAt(WtsAudio::BufferAt * bufferAt);
    void scratchUpdated(WtsAudio::BufferAt * bufferAt, bool recording);

    void samplerClock(qint64 ms);
    void samplerSchedule(WtsAudio::BufferAt * buffer);
    void samplerClear();

    void loaded();
    void stopped();

protected:
    QString makeSampleName();
    void constructStateMachine();

    WtsAudio m_audio;
    Ui::MainWindow *ui;
    QFileInfo m_movInfo;
    QDir m_dataDir;

    WtsAudio::BufferAt m_scratch;

    QList<WtsAudio::BufferAt *> m_sequence;
    QList<WtsAudio::BufferAt *>::iterator m_sequenceCursor;
    QMap<qint64, Marker *> m_markers;
    int m_lastSampleNameNum;
    VideoFile * m_videoFile;
    bool m_loading;
    Exporter * m_exporter;
    QStateMachine m_machine;
    QState * m_workshop;
    QActionGroup * m_tabActions;

    double m_finalTension;
    bool m_muteOnRecord;

    Preferences * m_preferences;
    QSettings m_settings;
};

#endif // MAINWINDOW_H

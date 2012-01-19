#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <Phonon>
#include <QStateMachine>
#include <QActionGroup>

#include "Project.h"
#include "WtsAudio.h"
#include "SoundBuffer.h"
#include "VideoFile.h"
#include "Preferences.h"

namespace Ui
{
    class MainWindow;
}

namespace WTS {

class Exporter;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void addMarker(Project::MarkerType type, qint64 when = -1, float tension = 0.5);
    Phonon::MediaObject * mediaObject();
    QState * addPage(const QString& name, QList<QWidget*> widgets, QList<QAction*> actions = QList<QAction*>());

    void buildMovieSelector();

    QPainterPath tensionCurve() const;
    void removeMark(Project::Marker * m);
    void removeBuffer(WtsAudio::BufferAt * bufferAt);

    qint64 duration() { return m_videoFile->duration(); }
    QDir movDir();
    Project * project() { return m_project; }

public slots:
    void setFullscreen(bool fs);
    void onPlay(bool play);
    void onRecord(bool record);
    void tick(qint64 ms);
    void seek(qint64 ms);

    void onMovieFinished();

    void addSceneMark(){ addMarker(Project::SCENE); }
    void addEventMark(){ addMarker(Project::EVENT); }

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

    Project * m_project;

    QDir m_movDir;
    bool m_movDirFound;

    WtsAudio m_audio;
    Ui::MainWindow *ui;
    QFileInfo m_movInfo;
    QDir m_dataDir;

    WtsAudio::BufferAt m_scratch;

    QList<WtsAudio::BufferAt *> m_sequence;
    QList<WtsAudio::BufferAt *>::iterator m_sequenceCursor;
    int m_lastSampleNameNum;
    VideoFile * m_videoFile;
    bool m_loading;
    Exporter * m_exporter;
    QStateMachine m_machine;
    QState * m_workshop;
    QActionGroup * m_tabActions;

    bool m_muteOnRecord;

    Preferences * m_preferences;
    QSettings m_settings;
};

}

#endif // MAINWINDOW_H

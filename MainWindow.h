#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <Phonon>
#include <QStateMachine>
#include <QActionGroup>

#include "Project.h"
#include "WtsAudio.h"
#include "SoundBuffer.h"
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

    void exportMovie();

    void refreshTension();
    void setMuteOnRecord(bool on) { m_muteOnRecord = on; }
    void startSolo( WtsAudio::BufferAt * );

    void onEndOfSample( WtsAudio::BufferAt * );

signals:
    void storyBoardChanged();
    void scratchUpdated(WtsAudio::BufferAt * bufferAt, bool recording);

    void samplerClock(qint64 ms);
    void samplerClear();

    void loaded();
    void stopped();

    void projectChanged(Project *);

protected:
    void constructStateMachine();
    bool eventFilter( QObject * watched, QEvent * event );

    Project * m_project;

    WtsAudio m_audio;
    Ui::MainWindow *ui;

    WtsAudio::BufferAt m_scratch;

    Exporter * m_exporter;
    QStateMachine m_machine;
    QState * m_workshop;
    QActionGroup * m_tabActions;

    bool m_muteOnRecord;

    Preferences * m_preferences;
    QSettings m_settings;

    WtsAudio::BufferAt * m_soloBuffer;
};

}

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QDir>
#include <Phonon>

#include "wtsaudio.h"
#include "soundbuffer.h"
#include "videofile.h"

namespace Ui
{
    class MainWindow;
}

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

    struct Marker {
        MarkerType m_type;
        qint64 m_ms;
        QPixmap m_snapshot;

        Marker() : m_type(NONE), m_ms(0) {}
        Marker(MarkerType type, qint64 ms) : m_type(type), m_ms(ms) {}
        bool operator<(const Marker& other) const { return m_ms < other.m_ms; }
    };

    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QList<Marker> getMarkers(MarkerType type = ANY, bool forward = true) const;
    Phonon::MediaObject * mediaObject();
    void addMarker(MarkerType type, qint64 when = -1);

public slots:
    void setFullscreen(bool fs);
    void onPlay(bool play);
    void onRecord(bool record);
    void tick(qint64 ms);
    void seek(qint64 ms);

    void addSceneMark(){ addMarker(SCENE); }
    void addEventMark(){ addMarker(EVENT); }

    void loadMovie(const QString& path);
    void loadToScratch(WtsAudio::BufferAt * bufferAt);

    void resetData();
    void saveData();
    void loadData();

signals:
    void storyBoardChanged();
    void newBufferAt(WtsAudio::BufferAt * bufferAt);
    void scratchUpdated(bool on, qint64 at, const SoundBuffer& scratch);

    void samplerClock(qint64 ms);
    void samplerSchedule(WtsAudio::BufferAt * buffer);
    void samplerClear();

protected:
    QString makeSampleName();

    WtsAudio m_audio;
    Ui::MainWindow *ui;
    QDir m_dataDir;
    qint64 m_scratchInsertTime;
    SoundBuffer m_scratch;
    QList<WtsAudio::BufferAt *> m_sequence;
    QList<WtsAudio::BufferAt *>::iterator m_sequenceCursor;
    QMap<qint64, Marker> m_markers;
    int m_lastSampleNameNum;
    VideoFile * m_videoFile;
    bool m_loading;
};

#endif // MAINWINDOW_H

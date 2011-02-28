#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QDir>
#include <Phonon>

#include "wtsaudio.h"
#include "soundbuffer.h"

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
        MarkerType type;
        qint64 ms;
        Marker() : type(NONE), ms(0) {}
        Marker(MarkerType type, qint64 ms) : type(type), ms(ms) {}
    };

    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QList<Marker> getMarkers(MarkerType type = ANY) const;
    Phonon::MediaObject * mediaObject();

public slots:
    void setFullscreen(bool fs);
    void onPlay(bool play);
    void onRecord(bool record);
    void tick(qint64 ms);
    void seek(qint64 ms);

    void addScene();
    void addMarker();
    void loadMovie(const QString& path);
    void loadToScratch(WtsAudio::BufferAt * bufferAt);

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
    qint64 m_sampleSyncMs;
    SoundBuffer m_scratch;
    QList<WtsAudio::BufferAt *> m_sequence;
    QList<WtsAudio::BufferAt *>::iterator m_sequenceCursor;
    QMap<qint64, Marker> m_markers;
    int m_lastSampleNameNum;
};

#endif // MAINWINDOW_H

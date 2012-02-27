#ifndef TIMELINECONTROLLER_HPP
#define TIMELINECONTROLLER_HPP

#include "WtsAudio.h"

#include <QObject>
#include <QPointer>

namespace WTS {

class Project;

/*
 * TimeLineController maintains a current position in the Project.
 * This allows playback, export, and editing.
 */
class TimeLineController : public QObject
{
    Q_OBJECT
public:
    explicit TimeLineController(QObject *parent = 0);
    
    // sequence iteration...
    void seek(qint64 ms);
    void start();
    void advanceSequenceCursor(qint64 ms);
    QList<WtsAudio::BufferAt *>::iterator beginCursor();
    QList<WtsAudio::BufferAt *>::iterator endCursor();

    qint64 cursorTime() const { return m_cursorTime; }

signals:
    void samplerSchedule(WtsAudio::BufferAt * buffer);

public slots:
    void setProject(Project * project);
    
protected:
    QPointer<Project> m_project;

    // sequence iteration...
    //   this is a copy of the sequence
    QList<WtsAudio::BufferAt *> m_sequence;
    QList<WtsAudio::BufferAt *>::iterator m_sequenceCursor;

    qint64 m_cursorTime;
};

}

#endif // TIMELINECONTROLLER_HPP

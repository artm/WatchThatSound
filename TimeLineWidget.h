#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QtCore>
#include <QGraphicsView>
#include <QGraphicsLineItem>
#include "MainWindow.h"

namespace WTS {

class Synced;
class Project;

class TimeLineWidget : public QGraphicsView
{
    Q_OBJECT
    Q_PROPERTY(bool seekOnDrag READ seekOnDrag WRITE setSeekOnDrag)
    Q_PROPERTY(bool editMode READ editMode WRITE setEditMode)
public:

    explicit TimeLineWidget(QWidget *parent = 0);
    bool seekOnDrag() const { return m_seekOnDrag; }
    bool editMode() const { return m_editMode; }

    enum {
        SYNCED = 1000
    };

    static void assignSynced(QGraphicsItem * item, WTS::Synced * synced);
    // find synced associated with given item or its ancestor
    // returns the item which has the synced associated
    static QGraphicsItem * findSynced(QGraphicsItem * item, WTS::Synced ** synced);
    virtual void deleteSynced(QGraphicsItem *, WTS::Synced *) {}

    Project * project() { return m_project; }

signals:
    void dataChanged();

public slots:
    virtual void setCurrentTime(qint64 time);
    void setSeekOnDrag(bool on) { m_seekOnDrag = on; }
    void setEditMode(bool on);
    virtual void updateSelection();
    void invalidateBackground() {
        invalidateScene(
                    sceneRect(),
                    QGraphicsScene::BackgroundLayer);
    }
    virtual void setProject(Project * project);

protected:
    // FIXME this one should be eventually phased out
    MainWindow * m_mainWindow;
    QPointer<Project> m_project;
    bool m_seekOnDrag;
    qint64 m_currentTime;

    QGraphicsLineItem * m_cursorLine;
    QGraphicsRectItem * m_selectionRect;

    void paintRange(QPainter * painter, qreal x, qreal w, const QColor& c);

    virtual void drawBackground ( QPainter * painter, const QRectF & rect );

    virtual void resizeEvent ( QResizeEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseMoveEvent ( QMouseEvent * event );
    virtual void keyPressEvent(QKeyEvent *event);

    void doSeekOnDrag( QMouseEvent * event );

private:
    bool m_editMode, m_deafToSeek;
};

}

#endif // TIMELINEWIDGET_H

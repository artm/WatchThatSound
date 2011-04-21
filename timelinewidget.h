#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QtCore>
#include <QGraphicsView>
#include <QGraphicsLineItem>
#include "mainwindow.h"

namespace WTS { class Synced; }

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
        SYNCED = 1000,
        DRAG_OPTIONS,
    };
    enum DragFlags {
        DRAG_X = 1,
        DRAG_Y = 2,
        DRAG_XY = DRAG_X|DRAG_Y,
    };

    void assignSynced(QGraphicsItem * item, WTS::Synced * synced);
    // find synced associated with given item or its ancestor
    // returns the item which has the synced associated
    QGraphicsItem * findSynced(QGraphicsItem * item, WTS::Synced ** synced);

    static void setItemDragFlags(QGraphicsItem * item, int options);
    static int itemDragFlags(QGraphicsItem * item);

signals:
    void dataChanged();

public slots:
    virtual void setCurrentTime(qint64 time);
    void setSeekOnDrag(bool on) { m_seekOnDrag = on; }
    void setEditMode(bool on) { m_editMode = on; }

protected:
    MainWindow * m_mainWindow;
    bool m_seekOnDrag;
    qint64 m_currentTime;
    bool m_editMode;

    QGraphicsItem * m_draggedItem;
    QPointF m_lastDragPos;
    int m_draggedItemFlags;

    void seekTo(qint64 x);
    void paintRange(QPainter * painter, qreal x, qreal w, const QColor& c);

    virtual void drawBackground ( QPainter * painter, const QRectF & rect );
    virtual void drawForeground ( QPainter * painter, const QRectF & rect );
    virtual void resizeEvent ( QResizeEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseMoveEvent ( QMouseEvent * event );
    virtual void mouseReleaseEvent(QMouseEvent *event);
};

Q_DECLARE_METATYPE(TimeLineWidget::DragFlags);

#endif // TIMELINEWIDGET_H

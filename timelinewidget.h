#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QtCore>
#include <QGraphicsView>
#include <QGraphicsLineItem>
#include "mainwindow.h"

class TimeLineWidget : public QGraphicsView
{
    Q_OBJECT
public:
    Q_PROPERTY(bool seekOnDrag READ seekOnDrag WRITE setSeekOnDrag)
    Q_PROPERTY(bool editMode READ editMode WRITE setEditMode)

    explicit TimeLineWidget(QWidget *parent = 0);
    bool seekOnDrag() const { return m_seekOnDrag; }
    bool editMode() const { return m_editMode; }

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

    void seekTo(qint64 x);
    void paintRange(QPainter * painter, qreal x, qreal w, const QColor& c);

    virtual void drawBackground ( QPainter * painter, const QRectF & rect );
    virtual void drawForeground ( QPainter * painter, const QRectF & rect );
    virtual void resizeEvent ( QResizeEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseMoveEvent ( QMouseEvent * event );
};

#endif // TIMELINEWIDGET_H

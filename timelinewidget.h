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

    explicit TimeLineWidget(QWidget *parent = 0);
    bool seekOnDrag() { return _seekOnDrag; }

signals:

public slots:
    virtual void setCurrentTime(qint64 time);
    void setSeekOnDrag(bool on) { _seekOnDrag = on; }

protected:
    MainWindow * m_mainWindow;
    bool _seekOnDrag;
    qint64 m_currentTime;

    void seekTo(qint64 x);
    void paintRange(QPainter * painter, qreal x, qreal w, const QColor& c);

    virtual void drawBackground ( QPainter * painter, const QRectF & rect );
    virtual void drawForeground ( QPainter * painter, const QRectF & rect );
    virtual void resizeEvent ( QResizeEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseMoveEvent ( QMouseEvent * event );
};

#endif // TIMELINEWIDGET_H

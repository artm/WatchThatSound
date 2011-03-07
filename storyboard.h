#ifndef STORYBOARD_H
#define STORYBOARD_H

#include "timelinewidget.h"

class StoryBoard : public TimeLineWidget
{
    Q_OBJECT
public:
    explicit StoryBoard(QWidget *parent = 0);

public slots:
    void updateSnapshots();
    virtual void resizeEvent ( QResizeEvent * event );
    void setVideoSize(int w, int h) { m_videoWidth = w; m_videoHeight = h; }
    virtual void setCurrentTime(qint64 time);

protected:
    float m_thumbWidth, m_thumbHeight;
    int m_videoWidth, m_videoHeight;
    float m_thumbScale;
    float m_marginX;

    QHash<QGraphicsItem *, MainWindow::Marker *> m_itemToMarker;
    QMap<qint64, QGraphicsItem *> m_msToItem;
    QGraphicsItem * m_selectedThumb;

    // drag drop
    QGraphicsItem * m_dragItem;
    QPointF m_dragLastP;

    QGraphicsProxyWidget * m_itemPopup;
    QGraphicsItem * m_popupsItem;

    static const int s_levelCount = 5;
    static const float s_marginY = 0.05, s_marginBottom = 0.1;

    virtual void mousePressEvent ( QMouseEvent * event );
    void drawBackground( QPainter * painter, const QRectF & rect ) ;

    void showItemPopup( QGraphicsItem * item);
};

#endif // STORYBOARD_H

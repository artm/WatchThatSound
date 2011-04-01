#ifndef CURVEEDITOR_H
#define CURVEEDITOR_H

#include "timelinewidget.h"

class CurveEditor : public TimeLineWidget
{
    Q_OBJECT
public:
    explicit CurveEditor(QWidget *parent = 0);

    void setCurve(const QPainterPath& curve);
    void scaleNodes();
    bool isEdited() const { return m_edited; }

    void saveData(QXmlStreamWriter& xml);
    void loadData(QXmlStreamReader& xml);

    QPainterPath curve() const { return m_curve ? m_curve->path() : QPainterPath(); }
signals:

public slots:

protected:
    virtual void resizeEvent ( QResizeEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseReleaseEvent ( QMouseEvent * event );
    virtual void mouseMoveEvent ( QMouseEvent * event );

    float m_nodePixelSize;
    QGraphicsPathItem * m_curve;

    QGraphicsRectItem * m_dragItem;
    QPointF m_dragLastP;
    bool m_edited;
};

#endif // CURVEEDITOR_H

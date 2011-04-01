#include "scoreeditor.h"
#include "rainbow.h"

const int ScoreEditor::s_wheelColorCount = 8;
const float ScoreEditor::s_wheelInnerRadius = 7;
const float ScoreEditor::s_wheelOuterRadius = 18;

ScoreEditor::ScoreEditor(QWidget *parent)
    : TimeLineWidget(parent)
    , m_gridStep(5)
    , m_newSymbol(0)
    , m_colorSelCircle(0)
{
    initNewSymbol();
    setRenderHints(QPainter::Antialiasing);

    // make color wheel
    m_colorWheel = new QGraphicsItemGroup(0,scene());

    float arcRun = 360.0 / s_wheelColorCount;

    QRectF outerRect( - s_wheelOuterRadius, - s_wheelOuterRadius, 2.0*s_wheelOuterRadius, 2.0*s_wheelOuterRadius);
    QRectF innerRect( - s_wheelInnerRadius, - s_wheelInnerRadius, 2.0*s_wheelInnerRadius, 2.0*s_wheelInnerRadius);

    for(int i = 0; i<s_wheelColorCount; ++i) {
        float startAngle = arcRun * i;
        float a0 = startAngle / 180.0 * M_PI;
        float a1 = (startAngle + arcRun) / 180.0 * M_PI;

        QPainterPath petalShape;

        // is Qt inconsistent in the orientation of Y and angles?
        petalShape.moveTo( s_wheelInnerRadius * cos(a1), -s_wheelInnerRadius * sin(a1) );
        petalShape.arcTo(  innerRect, startAngle+arcRun, -arcRun );
        petalShape.lineTo( s_wheelOuterRadius * cos(a0), -s_wheelOuterRadius * sin(a0));
        petalShape.arcTo(  outerRect, startAngle, arcRun );
        petalShape.closeSubpath();

        QGraphicsPathItem * petal = new QGraphicsPathItem(petalShape, m_colorWheel);
        petal->setPen(QPen(Rainbow::getColor(i,255,80)));
        petal->setBrush(QBrush(Rainbow::getColor(i,200)));

        petal->setData(PetalIndex, i);

    }
    m_colorSelCircle = new QGraphicsEllipseItem(innerRect,m_colorWheel);
    selectPetal( m_colorWheel->childItems()[0] );
}

void ScoreEditor::drawBackground(QPainter *painter, const QRectF &rect)
{
    TimeLineWidget::drawBackground(painter, rect);
    painter->setPen(QColor(0,64,0,16));

    /* FIXME restrict to rect? */
    for(int i=0; i<width(); i+=m_gridStep) {
        float x = (float)i/width();
        painter->drawLine(QPointF(x,0.0),QPointF(x,1.0));
    }
    for(int j=0; j<height(); j+=m_gridStep) {
        float y = (float)j/height();
        painter->drawLine(QPointF(0.0,y),QPointF(1.0,y));
    }
}

void ScoreEditor::mouseReleaseEvent(QMouseEvent * /*event*/)
{
    m_newSymbol->finish();
    m_symbols.append( m_newSymbol );
    initNewSymbol();
    emit dataChanged();
}

void ScoreEditor::mousePressEvent(QMouseEvent * event)
{
    /* first see if we hit something */
    if (event->buttons() & Qt::LeftButton) {
        QGraphicsItem * hitItem = itemAt( event->pos() );

        if (hitItem) {

            // is this a petal?
            QVariant vpindex = hitItem->data(PetalIndex);
            if (vpindex.isValid() && !vpindex.isNull()) {
                // yes, a petal, use its color
                selectPetal(hitItem);
            }

        } else
            m_newSymbol->start(mapToScene(event->pos()));
    }
}

void ScoreEditor::mouseMoveEvent(QMouseEvent * event)
{
    m_newSymbol->pull(mapToScene(event->pos()));
}

void ScoreEditor::resizeEvent(QResizeEvent *event)
{
    TimeLineWidget::resizeEvent(event);
    m_newSymbol->configure(scene(), width(), height());
    foreach(ScoreSymbol * sym, m_symbols) {
        sym->configure(scene(), width(), height());
        sym->updateGraphics();
    }

    m_colorWheel->setTransform( QTransform::fromScale( 1.0/width(), 1.0/height() ) );
    m_colorWheel->setTransform( QTransform::fromTranslate(
                                   s_wheelOuterRadius,
                                   s_wheelOuterRadius ),
                               true);
}

void ScoreEditor::selectPetal(QGraphicsItem * item)
{
    QGraphicsPathItem * petal =
            dynamic_cast<QGraphicsPathItem *>(item);

    if (!petal) return;

    m_colorSelCircle->setPen(petal->pen());
    m_colorSelCircle->setBrush(petal->brush());
    m_newSymbol->setColors(petal->pen(), petal->brush());
}

void ScoreEditor::initNewSymbol()
{
    m_newSymbol = new ScoreSymbol();
    m_newSymbol->configure(scene(), width(), height());
    if (m_colorSelCircle)
        m_newSymbol->setColors( m_colorSelCircle->pen(),
                               m_colorSelCircle->brush() );
    connect(m_newSymbol->inkTimer(), SIGNAL(timeout()), SLOT(passInk()));
}

void ScoreEditor::saveData(QXmlStreamWriter &xml)
{
    foreach(ScoreSymbol * symbol, m_symbols) {
        symbol->saveData(xml);
    }
}

void ScoreEditor::loadData(QXmlStreamReader &xml)
{
    while(xml.readNextStartElement()) {
        ScoreSymbol * symbol = new ScoreSymbol();
        symbol->configure(scene(), width(), height());
        symbol->loadData(xml);
        m_symbols << symbol;
        xml.readElementText();
    }
}


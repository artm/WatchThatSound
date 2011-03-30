#include "scoresymbol.h"

ScoreSymbol::ScoreSymbol(QObject *parent)
    : QObject(parent)
    , m_running(false)
{
}

void ScoreSymbol::ink()
{
    switch(shape()) {
    case CIRCLE:
        setShape(FILLED_CIRCLE);
        m_inkTimer.setInterval(20);
    case FILLED_CIRCLE:
        m_thickness[0]++;
        break;
    case DROPLET:
        setShape(POLY);
        m_inkTimer.setInterval(20);
    case POLY:
        m_thickness[1]++;
        break;
    case TAILED_CIRCLE:
        // ignore
        break;
    }
}

void ScoreSymbol::start(const QPointF &pos)
{
    m_running = true;
    m_shape = CIRCLE;
    m_thickness[0] = 0;
    m_thickness[1] = 0;
    m_pos = pos;
    m_inkTimer.start(500);
}

void ScoreSymbol::pull(const QPointF &pos)
{
    Q_ASSERT(m_running);
    m_length = pos.x() - m_pos.x();

    switch(shape()) {
    case CIRCLE:
        setShape(TAILED_CIRCLE);
        m_inkTimer.stop(); // not interested in ink no more
        break;
    case FILLED_CIRCLE:
        setShape(DROPLET);
        break;
    }

    if (m_inkTimer.isActive())
        m_inkTimer.start(500);
}

void ScoreSymbol::finish()
{
    Q_ASSERT(m_running);
    m_inkTimer.stop();
}

#include "EditController.hpp"

WTS::EditController::EditController(QObject *parent) :
    TimeLineController(parent)
{
}

void WTS::EditController::addMarkerAtCursor(Project::MarkerType type)
{
    qDebug() << "adding a marker";
    m_project->addMarker(type, cursorTime(), 0.5f);
}

#include "exportthread.h"

#include <QtDebug>

ExportThread::ExportThread(QObject *parent) :
    QThread(parent)
{
}

void ExportThread::run()
{
    int job = 30;
    emit exportProgress(0);
    for(int i=0; i<job; i++) {
        usleep(50000);
        emit exportProgress(100 * i / job);
    }
}

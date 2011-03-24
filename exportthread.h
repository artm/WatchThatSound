#ifndef EXPORTTHREAD_H
#define EXPORTTHREAD_H

#include <QThread>

class ExportThread : public QThread
{
    Q_OBJECT
public:
    explicit ExportThread(QObject *parent = 0);
    void run();

signals:
    void exportProgress(int percent);

public slots:

};

#endif // EXPORTTHREAD_H

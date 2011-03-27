#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Watch That Sound");
    a.setQuitOnLastWindowClosed(true);

    MainWindow w;
    w.setFullscreen(true);

    return a.exec();
}

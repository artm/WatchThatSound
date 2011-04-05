#include <QApplication>
#include "mainwindow.h"

#ifdef Q_WS_MAC
#include "CocoaInitializer.h"
#include "SparkleAutoUpdater.h"
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Watch That Sound");
    a.setQuitOnLastWindowClosed(true);

    MainWindow w;

    AutoUpdater* updater = 0;
#ifdef Q_WS_MAC
    CocoaInitializer initializer;
    updater = new SparkleAutoUpdater("https://ftp.v2.nl/~artm/WTS3/appcast.xml");
#endif
    if (updater) {
            updater->checkForUpdates();
    }


    w.setFullscreen(true);
    return a.exec();
}

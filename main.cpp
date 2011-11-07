#include <QApplication>
#include "MainWindow.h"

#ifdef Q_WS_MAC
#include "CocoaInitializer.h"
#include "SparkleAutoUpdater.h"
#endif

using namespace WTS;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Watch That Sound");
    a.setQuitOnLastWindowClosed(true);

#ifdef Q_WS_WIN
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath()+"\\plugins" );
#endif // Q_WS_WIN

    MainWindow w;

#ifdef Q_WS_MAC
    AutoUpdater* updater = 0;
    CocoaInitializer initializer;
    updater = new SparkleAutoUpdater("https://ftp.v2.nl/~artm/WTS3/appcast.xml");
    if (updater) {
            updater->checkForUpdates();
    }
#endif

    w.setFullscreen(true);
    exit(a.exec());
}

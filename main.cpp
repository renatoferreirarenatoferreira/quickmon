#include "mainwindow.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(APPLICATION_NAME);
    a.setApplicationVersion(APPLICATION_VERSION);

    MainWindow w;
    w.show();

    return a.exec();
}

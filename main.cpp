#include "mainwindow.h"
#include <QApplication>
#include <QObject>
#include <QPixmap>
#include <QSplashScreen>
#include <QThread>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QPixmap splashPixmap(":/Icons/playerSplash.png");
    QSplashScreen splash(splashPixmap);
    splash.show();


    MainWindow w;

    QThread::sleep(2);

    w.show();
    splash.finish(&w);

    return a.exec();
}

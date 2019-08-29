#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow *m = new MainWindow;

    m->showMaximized();

    return a.exec();
}

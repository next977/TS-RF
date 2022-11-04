#include <QFontDatabase>
#include <QFont>
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    int font = QFontDatabase::addApplicationFont(":/Fonts/starmedfont.ttf");
    MainWindow w;
    w.setIDstarmed(font);
    w.show();

    return a.exec();
}

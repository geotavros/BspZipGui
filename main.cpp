#include <QApplication>

#include "bspzipgui.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BspZipGui w;
    w.show();
    return a.exec();
}

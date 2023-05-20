#include "MainWidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Widget w;
    if (w.property("isClient").toBool())
        return 0;
    w.show();
    QApplication::setQuitOnLastWindowClosed(false);

    return a.exec();
}

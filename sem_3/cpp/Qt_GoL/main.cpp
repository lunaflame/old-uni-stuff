#include "Qt_GoL.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Qt_GoL w;
    w.show();

    return a.exec();
}

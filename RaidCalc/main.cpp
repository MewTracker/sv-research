#include "RaidCalc.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RaidCalc w;
    w.show();
    return a.exec();
}

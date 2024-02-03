#include "VolumeSHController.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VolumeSHController w;
    w.show();
    return a.exec();
}

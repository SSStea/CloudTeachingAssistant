#include "ECloudAssistant.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ECloudAssistant w;
    w.show();
    return a.exec();
}

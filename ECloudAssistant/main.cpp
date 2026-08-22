#include "ECloudAssistant.h"
#include "RtmpPushManager.h"
#include "PullerWidget.h"
#include "AVPlayer.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //ECloudAssistant w;
    //w.show();

    // CRtmpPushManager manager;
    // manager.bOpen("rtmp://172.20.108.206:1935/live/01");

    // CPullerWidget w;
    // w.show();

    CPullerWidget player;
    player.show();
    player.Open("rtmp://172.20.108.206:1935/live/01");

    return a.exec();
}

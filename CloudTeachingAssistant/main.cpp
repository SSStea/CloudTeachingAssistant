#include <iostream>
#include "EventLoop.h"
#include "TcpServer.h"

int main()
{
    uint32_t nCnt = std::thread::hardware_concurrency();
    CEventLoop eventLoop(nCnt);
    CTcpServer* server = new CTcpServer(&eventLoop);

    server->bStart("172.20.108.206", 9527);
    std::cout << "server start" << std::endl;

    getchar();

    server->Stop();

    std::cout << "server terminal" << std::endl;
    return 0;
}
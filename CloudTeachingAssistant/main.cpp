#include <iostream>
#include "EventLoop.h"
#include "TcpServer.h"
#include "RtmpServer.h"

void TcpTest()
{
	uint32_t nCnt = std::thread::hardware_concurrency();
	CEventLoop eventLoop(nCnt);
	CTcpServer* server = new CTcpServer(&eventLoop);

	server->bStart("172.20.108.206", 9527);
	std::cout << "server start" << std::endl;

	getchar();

	server->Stop();

	std::cout << "server terminal" << std::endl;
}

void RtmpSeverTest()
{
	uint32_t nCnt = std::thread::hardware_concurrency();
	CEventLoop eventLoop(nCnt);
	auto rtmpServer = CRtmpServer::pCreate(&eventLoop);
	rtmpServer->SetChunkSize(60000);
	rtmpServer->SetEventCallback([](std::string strType, std::string strStreamPath) {
		std::cout << "[Event]: " << strType << ", [stream path]: " << strStreamPath << std::endl;
		});
	if (rtmpServer->bStart("172.20.108.206", 1935))
	{
		std::cout << "rtmp server start success" << std::endl;
	}
	else
	{
		std::cout << "rtmp server start fail" << std::endl;
	}
	getchar();
}

int main()
{
	RtmpSeverTest();
    return 0;
}

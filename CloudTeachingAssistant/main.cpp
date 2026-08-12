#include <iostream>
#include "EventLoop.h"
#include "TcpServer.h"
#include "RtmpServer.h"
//#include "SigServer.h"
#include "LoginServer.h"
#include "LoadBalanceServer.h"

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

//void SigeServerTest()
//{
//	uint32_t nCnt = std::thread::hardware_concurrency();
//	CEventLoop eventLoop(nCnt);
//	auto sigServer = CSigServer::pCreate(&eventLoop);
//	if (sigServer->bStart("172.20.108.206", 6539))
//	{
//		std::cout << "sig server start success" << std::endl;
//	}
//	else
//	{
//		std::cout << "sig server start fail" << std::endl;
//	}
//	getchar();
//}

void LoginTest()
{
	uint32_t nCnt = std::thread::hardware_concurrency();
	std::shared_ptr<CLoginServer> loginServer = nullptr;
	CEventLoop eventLoop(nCnt);
	auto loadServer = CLoadBalanceServer::pCreate(&eventLoop);

	if (loadServer->bStart("172.20.108.206", 8523))
	{
		loginServer = CLoginServer::pCreate(&eventLoop);
		if (loginServer->bStart("172.20.108.206", 9867))
		{
			std::cout << "server start success\n";
		}
		else
		{
			std::cout << "server start fail\n";
		}
	}
	getchar();
}

int main()
{
	LoginTest();
    return 0;
}

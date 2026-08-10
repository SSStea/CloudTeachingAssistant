#pragma once
#include "TcpServer.h"
#include "SigConnection.h"
#include "EventLoop.h"

class CSigServer : public CTcpServer
{
public:
	static std::shared_ptr<CSigServer> pCreate(CEventLoop* eventloop);  //设单例
	~CSigServer();
private:
	CEventLoop* m_pEventLoop;
	CSigServer(CEventLoop* eventloop);
	virtual CTcpConnection::ptr onConnect(int nSocket);
};


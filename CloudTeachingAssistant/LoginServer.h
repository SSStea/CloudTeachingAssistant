#pragma once
#include "TcpServer.h"
#include "TcpClient.h"

class CLoginServer : public CTcpServer
{
public:
	static std::shared_ptr<CLoginServer> pCreate(CEventLoop* eventloop);
	~CLoginServer();
private:
	TimerId m_ID;
	CEventLoop* m_pEventLoop;
	std::unique_ptr<CTcpClient> m_pClient;

	CLoginServer(CEventLoop* eventloop);
	virtual CTcpConnection::ptr onConnect(int nSocket);
};

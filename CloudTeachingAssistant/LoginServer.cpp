#include "LoginServer.h"
#include "EventLoop.h"
#include "LoginConnection.h"

CLoginServer::CLoginServer(CEventLoop* eventloop)
	: CTcpServer(eventloop), m_pEventLoop(eventloop), m_pClient(nullptr)
{
	m_pClient.reset(new CTcpClient());
	m_pClient->Create();
	if (m_pClient->bConnect("172.20.108.206", 8523))
	{
		m_ID = m_pEventLoop->AddTimer([this]() {
			m_pClient->GetMonitorInfo();
			return true;
			}, 1000);
	}
}

CLoginServer::~CLoginServer()
{

}

std::shared_ptr<CLoginServer> CLoginServer::pCreate(CEventLoop* eventloop)
{
	std::shared_ptr<CLoginServer> server(new CLoginServer(eventloop));

	return server;
}

CTcpConnection::ptr CLoginServer::onConnect(int nSocket)
{
	return std::make_shared<CLoginConnection>(m_pEventLoop->GetReactor().get(), nSocket);
}
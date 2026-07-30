#include "TcpServer.h"
#include <iostream>

CTcpServer::CTcpServer(CEventLoop* eventLoop)
	:m_eventLoop(eventLoop), m_nPort(0), m_accpetor(new CAcceptor(eventLoop))
	, m_bIsStarted(false)
{
	m_accpetor->SetNewConnectCallback([this](int nFd) {
		CTcpConnection::ptr conn = this->onConnect(nFd);
		if (conn)
		{
			this->addConnection(nFd, conn);
			conn->SetDisconnectCallback([this](CTcpConnection::ptr conn) {
				int nFd = conn->nGetSocket();
				this->removeConnection(nFd);
				});
		}
		});
}

CTcpServer::~CTcpServer()
{
	this->Stop();
}

bool CTcpServer::bStart(std::string strIP, uint16_t nPort)
{
	this->Stop();

	if (!m_bIsStarted)
	{
		if (m_accpetor->nListen(strIP, nPort) < 0)
		{
			std::cout << "Tcp Server Start fail: Listen fail" << std::endl;
			return false;
		}

		m_nPort = nPort;
		m_strIP = strIP;
		m_bIsStarted = true;
	}

	return false;
}

void CTcpServer::Stop()
{
	if (m_bIsStarted)
	{
		for (auto it : m_mapConnections)
		{
			it.second->disConnect();
		}

		m_accpetor->Close();
		m_bIsStarted = false;
	}
}

CTcpConnection::ptr CTcpServer::onConnect(int nFd)
{
	return std::make_shared<CTcpConnection>(m_eventLoop->GetReactor().get(), nFd);
}

void CTcpServer::addConnection(int nFd, CTcpConnection::ptr conn)
{
	m_mapConnections.emplace(nFd, conn);
}

void CTcpServer::removeConnection(int nFd)
{
	m_mapConnections.erase(nFd);
}

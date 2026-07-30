#include "Acceptor.h"
#include <iostream>

CAcceptor::CAcceptor(CEventLoop* eventLoop)
	: m_eventLoop(eventLoop), m_tcpSocket(new CTcpSocket())
{

}

CAcceptor::~CAcceptor()
{
}

int CAcceptor::nListen(std::string strIP, uint16_t nPort)
{
	if (m_tcpSocket->nGetSocket() > 0)
	{
		m_tcpSocket->Close();
	}

	int nFd = m_tcpSocket->nCreate();
	m_channelPtr.reset(new CChannel(nFd));
	CSocketUtil::SetNonBlock(nFd);
	CSocketUtil::SetReuseAddr(nFd);
	CSocketUtil::SetReusePort(nFd);

	if (!m_tcpSocket->bBind(strIP, nPort))
	{
		std::cout << "Acceptor bind fail" << std::endl;
		return -1;
	}

	if (!m_tcpSocket->bListen(1024))
	{
		std::cout << "Acceptor listen fail" << std::endl;
		return -2;
	}

	//设置通道的读回调，这样才能通过通道读取新来的连接请求
	m_channelPtr->SetReadCallback([this]() {this->onAccept(); });
	m_channelPtr->EnableReading();

	m_eventLoop->UpdateChannel(m_channelPtr);

	return 0;
}


void CAcceptor::Close()
{
	if (m_tcpSocket->nGetSocket() > 0)
	{
		m_eventLoop->RemoveChannel(m_channelPtr);
		m_tcpSocket->Close();
	}
}

void CAcceptor::onAccept()
{
	int nFd = m_tcpSocket->nAccept();
	if (nFd > 0)
	{
		if (m_newConnectCb)
		{
			m_newConnectCb(nFd);
		}
	}
}

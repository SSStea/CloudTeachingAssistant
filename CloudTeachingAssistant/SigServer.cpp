#include "SigServer.h"

CSigServer::CSigServer(CEventLoop* eventloop)
	: CTcpServer(eventloop), m_pEventLoop(eventloop)
{

}

CSigServer::~CSigServer()
{
}

std::shared_ptr<CSigServer> CSigServer::pCreate(CEventLoop* eventloop)
{
	std::shared_ptr<CSigServer> server(new CSigServer(eventloop));
	return server;
}

CTcpConnection::ptr CSigServer::onConnect(int nSocket)
{
	return std::make_shared<CSigConnection>(m_pEventLoop->GetReactor().get(), nSocket) ;
}
#pragma once
#include <memory>
#include <unordered_map>
#include "TcpSocket.h"
#include "TcpConnection.h"
#include "Acceptor.h"

class CEventLoop;
class CAcceptor;

class CTcpServer
{
public:
	CTcpServer(CEventLoop* eventLoop);
	~CTcpServer();

	virtual bool bStart(std::string strIP, uint16_t nPort);
	virtual void Stop();

	inline std::string strGetIPAddress() const { return m_strIP; }
	inline uint16_t nGetPort() const { return m_nPort; }

protected:
	//通过套接字获取到已有的TCP连接的指针
	virtual CTcpConnection::ptr onConnect(int nFd);
	//将TCP连接放进map表中方便管理
	virtual void addConnection(int nFd, CTcpConnection::ptr conn);
	//从表中删除TCP链接
	virtual void removeConnection(int nFd);

private:
	CEventLoop* m_eventLoop;
	uint16_t m_nPort;
	std::string m_strIP;
	std::unique_ptr<CAcceptor> m_accpetor;
	bool m_bIsStarted = false;
	//已有连接的map表
	std::unordered_map<int, CTcpConnection::ptr> m_mapConnections;
};


#pragma once
#include "TcpConnection.h"
#include "LoadBalanceServer.h"
#include <iostream>
#include <chrono>

class CLoadBalanceConnection : public CTcpConnection
{
public:
	CLoadBalanceConnection(std::shared_ptr<CLoadBalanceServer> loadBalanceServer,
		CReactorBase* reactor, int nSocket);
	~CLoadBalanceConnection();

protected:
	void Disonnection();
	bool bOnRead(CBufferReader& buffer);
	bool bIsTimeout(uint64_t nTimestamp);
	void HandleMessage(CBufferReader& buffer);
	void HandleLogin(CBufferReader& buffer);
	void HandleMonitorInfo(CBufferReader& buffer);

private:
	int m_nSocket;
	std::weak_ptr<CLoadBalanceServer> m_loadBalanceServer;
};


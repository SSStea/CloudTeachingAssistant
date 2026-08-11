#pragma once
#include "TcpServer.h"
#include "loaddefine.h"

class CLoadBalanceServer : public CTcpServer, public std::enable_shared_from_this<CLoadBalanceServer>
{
public:
	static std::shared_ptr<CLoadBalanceServer> pCreate(CEventLoop* eventloop);
	~CLoadBalanceServer();
private:
	friend class CLoadBalanceConnection;
	CLoadBalanceServer(CEventLoop* eventloop);
	virtual CTcpConnection::ptr onConnect(int nSocket);
	void UpdateMonitor(const int nFD, MonitorBody* info);
	MonitorBody* GetMonitorInfo();
private:
	CEventLoop* m_pEventLoop;
	std::mutex m_mutex;
	std::map<int, MonitorBody*> m_mapMonitorInfos;
};


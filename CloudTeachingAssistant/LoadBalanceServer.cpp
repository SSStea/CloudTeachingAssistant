#include "LoadBalanceServer.h"
#include "LoadBalanceConnection.h"
#include <vector>
#include <algorithm>

std::shared_ptr<CLoadBalanceServer> CLoadBalanceServer::pCreate(CEventLoop* eventloop)
{
	std::shared_ptr<CLoadBalanceServer> server(new CLoadBalanceServer(eventloop));

	return server;
}

CLoadBalanceServer::CLoadBalanceServer(CEventLoop* eventloop)
	: CTcpServer(eventloop), m_pEventLoop(eventloop)
{

}

CLoadBalanceServer::~CLoadBalanceServer()
{
	for (auto iter : m_mapMonitorInfos)
	{
		if (iter.second)
		{
			delete iter.second;
			iter.second = nullptr;
		}
	}
}

CTcpConnection::ptr CLoadBalanceServer::onConnect(int nSocket)
{
	return std::make_shared<CLoadBalanceConnection>(shared_from_this(), m_pEventLoop->GetReactor().get(), nSocket);
}

void CLoadBalanceServer::UpdateMonitor(const int nFD, MonitorBody* info)
{
	//更新资源时加锁
	std::lock_guard<std::mutex> lock(m_mutex);

	//更新资源
	m_mapMonitorInfos[nFD] = info;
}

MonitorBody* CLoadBalanceServer::GetMonitorInfo()
{
	std::lock_guard<std::mutex> lock(m_mutex);

	//获取的时候先排序
	//先将这个map中元素转到vector再来排序
	std::vector<MinotorPair> vecMonitor(m_mapMonitorInfos.begin(), m_mapMonitorInfos.end());
	//就会通过这个CmpByValue结构体来去排序
	sort(vecMonitor.begin(), vecMonitor.end(), CmpByValue());

	//因为是从小到大排序，使用这个容器第一个就是最优的服务
	return vecMonitor[0].second;
}

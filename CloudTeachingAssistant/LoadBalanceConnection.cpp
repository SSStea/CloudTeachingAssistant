#include "LoadBalanceConnection.h"

#define TIMEOUT 60

CLoadBalanceConnection::CLoadBalanceConnection(std::shared_ptr<CLoadBalanceServer> loadBalanceServer, CReactorBase* reactor, int nSocket)
	:CTcpConnection(reactor, nSocket), m_nSocket(nSocket), m_loadBalanceServer(loadBalanceServer)
{
	this->setReadCallback([this](std::shared_ptr<CTcpConnection>, CBufferReader& buffer) {
		return this->bOnRead(buffer);
		});

	this->SetDisconnectCallback([this](std::shared_ptr<CTcpConnection> conn) {
		this->Disonnection();
		});
}

CLoadBalanceConnection::~CLoadBalanceConnection()
{
}

void CLoadBalanceConnection::Disonnection()
{
}

bool CLoadBalanceConnection::bOnRead(CBufferReader& buffer)
{
	if (buffer.nReadableBytes() > 0)
	{
		HandleMessage(buffer);
	}

	return true;
}

bool CLoadBalanceConnection::bIsTimeout(uint64_t nTimestamp)
{
	//获取当前时间
	auto now = std::chrono::system_clock::now();
	auto nowTimestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

	//计算差值
	uint64_t nTime = nowTimestamp - nTimestamp;

	return nTime > TIMEOUT;
}

void CLoadBalanceConnection::HandleMessage(CBufferReader& buffer)
{
	if (buffer.nReadableBytes() < sizeof(PacketHead))
	{
		return;
	}

	PacketHead* head = (PacketHead*)buffer.pPeek();
	if (buffer.nReadableBytes() < head->nLen)
	{
		return;
	}

	switch (head->nCmd)
	{
	case Login:
		HandleLogin(buffer);
		break;
	case Monitor:
		HandleMonitorInfo(buffer);
		break;
	default:
		std::cout << "cmd error" << std::endl;
		break;
	}

	buffer.Retrieve(head->nLen);
}

void CLoadBalanceConnection::HandleLogin(CBufferReader& buffer)
{
	LoginReply reply;
	LoginInfo* info = (LoginInfo*)buffer.pPeek();
	if (bIsTimeout(info->nTimestamp))
	{
		std::cout << "is timeout" << std::endl;
		reply.nCmd = ERROR;
	}
	else
	{
		//获取ip和端口
		auto server = m_loadBalanceServer.lock();
		if (server)
		{
			MonitorBody* monitor = server->GetMonitorInfo();//在GetMonitorInfo里面做一个资源算法来分配
			reply.arrIP = monitor->arrIP;
			reply.nPort = monitor->nPort;
		}
		else
		{
			std::cout << "no server" << std::endl;
			reply.nCmd = ERROR;
		}
	}
	Send((const char*)&reply, reply.nLen);
}

void CLoadBalanceConnection::HandleMonitorInfo(CBufferReader& buffer)
{
	//处理心跳
	//获取info
	MonitorBody* monitor = (MonitorBody*)buffer.pPeek();
	auto server = m_loadBalanceServer.lock();
	if (server)
	{
		server->UpdateMonitor(m_nSocket, monitor);
	}
}

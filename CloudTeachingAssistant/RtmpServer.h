#pragma once
#include <mutex>
#include "TcpServer.h"
#include "Rtmp.h"
#include "RtmpSession.h"
#include "EventLoop.h"
#include "RtmpConnection.h"

class CRtmpServer : public CTcpServer, public CRtmp, public std::enable_shared_from_this<CRtmpServer>
{
public:
	using eventCallback = std::function<void(std::string strType, std::string strStreamPath)>;

	//创建服务器
	static std::shared_ptr<CRtmpServer> pCreate(CEventLoop* eventLoop);
	~CRtmpServer();
	void SetEventCallback(const eventCallback& cb);

private:
	friend class CRtmpConnection;

	//设单例
	CRtmpServer(CEventLoop* eventLoop);
	//通过流路径管理区分session
	void AddSession(std::string strStreamPath);
	void RemoveSession(std::string strStreamPath);

	CRtmpSession::Ptr pGetSession(std::string strStreamPath);
	bool bHasPublisher(std::string strStreamPath);
	bool bHasSession(std::string strStreamPath);
	//打印日志
	void NotifyEvent(std::string strType, std::string strStreamPath);

	virtual CTcpConnection::ptr onConnect(int nSocket);

	CEventLoop* m_pEventLoop;
	std::mutex m_mutex;
	std::unordered_map<std::string, CRtmpSession::Ptr> m_mapRtmpSessions;
	std::vector<eventCallback> m_vecEventCallbacks;
};


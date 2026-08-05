#include "RtmpServer.h"

std::shared_ptr<CRtmpServer> CRtmpServer::pCreate(CEventLoop* eventLoop)
{
    std::shared_ptr<CRtmpServer> server(new CRtmpServer(eventLoop));
    return server;
    //可以new，但是不能直接make_shared，因为pCreate有权调用私有构造函数，而make_shared
    //会进入c++标准库内部无权调用
        //std::make_shared<CRtmpServer>(eventLoop);
}

CRtmpServer::CRtmpServer(CEventLoop* eventLoop)
    : CTcpServer(eventLoop), m_pEventLoop(eventLoop), m_vecEventCallbacks(10)
{
    //定时更新session
    m_pEventLoop->AddTimer([this]() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto it = m_mapRtmpSessions.begin(); it != m_mapRtmpSessions.end();)
        {
            if (it->second->nGetClients() == 0)
            {
                m_mapRtmpSessions.erase(it++);
            }
            else
            {
                it++;
            }
        }
        return true;
        }, 3000);//3s执行一次
}

CRtmpServer::~CRtmpServer()
{
}

void CRtmpServer::SetEventCallback(const eventCallback& cb)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_vecEventCallbacks.push_back(cb);
}

void CRtmpServer::AddSession(std::string strStreamPath)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_mapRtmpSessions.find(strStreamPath) == m_mapRtmpSessions.end())
    {
        m_mapRtmpSessions[strStreamPath] = std::make_shared<CRtmpSession>();
    }
}

void CRtmpServer::RemoveSession(std::string strStreamPath)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_mapRtmpSessions.find(strStreamPath) != m_mapRtmpSessions.end())
    {
        m_mapRtmpSessions.erase(strStreamPath);
    }
}

CRtmpSession::Ptr CRtmpServer::pGetSession(std::string strStreamPath)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    //没有就添加一个
	if (m_mapRtmpSessions.find(strStreamPath) == m_mapRtmpSessions.end())
	{
		m_mapRtmpSessions[strStreamPath] = std::make_shared<CRtmpSession>();
	}

    return m_mapRtmpSessions[strStreamPath];
}

bool CRtmpServer::bHasPublisher(std::string strStreamPath)
{
    auto session = pGetSession(strStreamPath);
    if (!session)
    {
        std::cout << "not has publisher" << std::endl;
        return false;
    }

    return (session->pGetPublisher() != nullptr);
}

bool CRtmpServer::bHasSession(std::string strStreamPath)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    return (m_mapRtmpSessions.find(strStreamPath) != m_mapRtmpSessions.end());
}

void CRtmpServer::NotifyEvent(std::string strType, std::string strStreamPath)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto eventCb : m_vecEventCallbacks)
    {
        if (eventCb)
        {
            eventCb(strType, strStreamPath);
        }
    }
}

CTcpConnection::ptr CRtmpServer::onConnect(int nSocket)
{
    return std::make_shared<CRtmpConnection>(shared_from_this(), m_pEventLoop->GetReactor().get(), nSocket);
}
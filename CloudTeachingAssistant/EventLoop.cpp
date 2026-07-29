#include "EventLoop.h"

CEventLoop::CEventLoop(uint32_t nThreadNums)
    :m_nReactorIndex(1), m_nThreadNums(nThreadNums)
{
    this->Loop();//启动循环
}

CEventLoop::~CEventLoop()
{
    this->Quit();
}

std::shared_ptr<CReactorBase> CEventLoop::GetReactor()
{
    if (m_vecReactors.size() == 1)//如果只有一个reactor
    {
        return m_vecReactors.at(0);//直接返回vector容器中的第一个
    }
    else
    {
        auto reactor = m_vecReactors.at(m_nReactorIndex);//按索引取出一个reactor
        m_nReactorIndex++;
        if (m_nReactorIndex >= m_vecReactors.size())
        {
            m_nReactorIndex = 0;
        }
        return reactor;
    }
    return nullptr;
}

TimerId CEventLoop::AddTimer(const TimerEvent& event, uint32_t nMsec)
{
    if (m_vecReactors.size() > 0)
    {
        //因为0号是主Reactor，所以这些操作都用0号Reactor做
        return m_vecReactors[0]->AddTimer(event, nMsec);
    }

    return 0;
}

void CEventLoop::RemoveTimer(TimerId timerId)
{
	if (m_vecReactors.size() > 0)
	{
		m_vecReactors[0]->RemoveTimer(timerId);
	}
}

void CEventLoop::UpdateChannel(ChannelPtr channel)
{
    if (m_vecReactors.size() > 0)
    {
        m_vecReactors[0]->UpdateChannel(channel);
    }
}

void CEventLoop::RemoveChannel(ChannelPtr& channel)
{
	if (m_vecReactors.size() > 0)
	{
		m_vecReactors[0]->RemoveChannel(channel);
	}
}

void CEventLoop::Loop()
{
    if (!m_vecReactors.empty())
    {
        return;
    }

    for (uint32_t i = 0; i < m_nThreadNums; i++)
    {
        //创建反应器
        std::shared_ptr<CReactorBase> reactorPtr(new CReactorEpoll(i));
        m_vecReactors.push_back(reactorPtr);
        //创建线程，在每个线程中都启动反应器
        std::shared_ptr<std::thread> threadPtr(new std::thread(&CReactorBase::Start,
            reactorPtr.get()));
        m_vecThreads.push_back(threadPtr);
    }
}

void CEventLoop::Quit()
{
    for (auto it : m_vecReactors)
    {
        it->Stop();
    }

    for (auto it : m_vecThreads)
    {
        if (it->joinable())
        {
            it->join();
        }
    }

    m_vecReactors.clear();
    m_vecThreads.clear();
}

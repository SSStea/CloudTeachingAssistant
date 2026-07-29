#include "ReactorEpoll.h"


CReactorEpoll::CReactorEpoll(int nID)
	: CReactorBase(nID)
{
	//创建Epoll
	m_nEpollFd = epoll_create(1024);
}

CReactorEpoll::~CReactorEpoll()
{
}

void CReactorEpoll::UpdateChannel(ChannelPtr channel)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	int nFd = channel->GetSocket();//获取通道管理的套接字

	if (m_mapChannels.find(nFd) != m_mapChannels.end())//查看这个通道是否存在
	{
		if (channel->IsNoneEvent())//如果这个通道不关心任何事
		{
			Update(EPOLL_CTL_DEL, channel);//把epoll监听中通道删掉
			m_mapChannels.erase(nFd);
		}
		else
		{
			Update(EPOLL_CTL_MOD, channel);//修改epoll的状态
		}
	}
	else//不存在
	{
		if (!channel->IsNoneEvent())//如果这个通道关心事件
		{
			m_mapChannels.emplace(nFd, channel);//加入epoll
			Update(EPOLL_CTL_ADD, channel);
		}
	}
}

void CReactorEpoll::RemoveChannel(ChannelPtr& channel)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	int nFd = channel->GetSocket();//获取通道管理的套接字

	if (m_mapChannels.find(nFd) != m_mapChannels.end())
	{
		Update(EPOLL_CTL_DEL, channel);
		m_mapChannels.erase(nFd);
	}
}

bool CReactorEpoll::bHandleEvent()
{
	struct epoll_event events[512] = { 0 };
	int nNumEvents = -1;

	nNumEvents = epoll_wait(m_nEpollFd, events, 512, 0);
	if (nNumEvents < 0)
	{
		if (errno != EINTR)
		{
			return false;
		}
	}

	for (int i = 0; i < nNumEvents; i++)
	{
		if (events[i].data.ptr)//如果这个事件有通道的话
		{
			((CChannel*)events[i].data.ptr)->HandleEvent(events[i].events);
		}
	}

	return true;
}

void CReactorEpoll::Update(int nOpt, ChannelPtr& channel)
{
	struct epoll_event event = { 0 };

	if (nOpt != EPOLL_CTL_DEL)
	{
		event.data.ptr = channel.get();
		event.events = channel->GetEvents();
	}

	if (::epoll_ctl(m_nEpollFd, nOpt, channel->GetSocket(), &event) < 0)
	{
		std::cout << "修改epoll事件失败" << std::endl;
	}
}

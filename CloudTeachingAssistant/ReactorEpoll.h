#pragma once
#include "ReactorBase.h"
#include <unordered_map>
#include <sys/epoll.h>
#include <errno.h>
#include <iostream>

class CReactorEpoll : public CReactorBase
{
public:
	CReactorEpoll(int nID = 0);
	virtual ~CReactorEpoll();

	void UpdateChannel(ChannelPtr channel);
	void RemoveChannel(ChannelPtr& channel);
	bool bHandleEvent();

protected:
	void Update(int nOpt, ChannelPtr& channel);

private:
	int m_nEpollFd = -1;
	std::mutex m_mutex;
	std::unordered_map<int, ChannelPtr> m_mapChannels;
};


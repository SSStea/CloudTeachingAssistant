#pragma once
#include <cstdint>
#include "Timer.h"
#include "Channel.h"
#include <atomic>
#include <mutex>

class CReactorBase//Reactor基类（TaskScheduler）
{
public:
	CReactorBase(int nId = 1);//id是reactor反应器的唯一id
	virtual ~CReactorBase();

	void Start();
	void Stop();

	TimerId AddTimer(const TimerEvent& event, uint32_t nMsec);//添加定时器，参数为事件、触发时间
	void RemoveTimer(TimerId timerId);

	virtual void UpdateChannel(ChannelPtr channel) {};
	virtual void RemoveChannel(ChannelPtr& channel) {};
	virtual bool bHandleEvent() { return false; }

	inline int nGetID() const { return m_nID; }

private:
	int					m_nID = 0;
	std::atomic_bool	m_bIsShutdown;
	std::mutex			m_mutex;
	CTimerQueue			m_timerQueue;
};


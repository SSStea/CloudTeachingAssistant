#include "Timer.h"

CTimer::CTimer(const TimerEvent& event, uint32_t nMsec)
	: eventCallback(event), m_nInterval(nMsec)
{
}

void CTimer::Sleep(uint32_t nMsec)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(nMsec));
}

void CTimer::SetNextTimeout(uint64_t nTimeNow)
{
	m_nNextTimeout = nTimeNow + m_nInterval;
}

int64_t CTimer::getNextTimeout()
{
	return m_nNextTimeout;
}

TimerId CTimerQueue::AddTimer(const TimerEvent& event, uint32_t nMsec)
{
	int64_t nTimePoint = nGetTimeNow(); //获取当前时间
	TimerId nTimerId = ++nLastTimeId;	//设置id

	auto timer = std::make_shared<CTimer>(event, nMsec); //创建定时任务，智能指针

	timer->SetNextTimeout(nTimePoint);
	m_mapTimers.emplace(nTimerId, timer);
	m_mapEvents.emplace(std::pair<int64_t, TimerId>(nTimePoint + nMsec, nTimerId), timer);

	return nTimerId;
}

void CTimerQueue::RemoveTimer(TimerId nID)
{
	auto it = m_mapTimers.find(nID);
	if (it != m_mapTimers.end())
	{
		int64_t nTimeout = it->second->getNextTimeout();
		m_mapEvents.erase(std::pair<int64_t, TimerId>(nTimeout, nID));
		m_mapTimers.erase(nID);
	}
}

void CTimerQueue::HandleTimerEvent()
{
	if (!m_mapTimers.empty())
	{
		int64_t nTimePoint = nGetTimeNow(); //获取当前时间
		while (!m_mapTimers.empty() &&
			m_mapEvents.begin()->first.first <= nTimePoint)
			//要处理的这个任务的超时时间不能超过当前时间
		{
			if(m_mapEvents.begin()->first.second)
			{
				TimerId timerId = m_mapEvents.begin()->first.second;
				//获取这个事件是一次性还是反复执行
				bool bFlag = m_mapEvents.begin()->second->eventCallback();
				if (bFlag)
				{
					m_mapEvents.begin()->second->SetNextTimeout(nTimePoint);
					//直接把这个事件移动过来，因为事件是智能指针，避免复制带来的指针加减一
					auto timerPtr = std::move(m_mapEvents.begin()->second);

					m_mapEvents.erase(m_mapEvents.begin());
					m_mapEvents.emplace(std::pair<int64_t, TimerId>(timerPtr->getNextTimeout(), timerId), timerPtr);
				}
				else
				{
					m_mapEvents.erase(m_mapEvents.begin());
					m_mapTimers.erase(timerId);
				}
			}
		}
	}
}

int64_t CTimerQueue::nGetTimeNow()
{
	auto timeNow = std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(timeNow.time_since_epoch()).count();
}

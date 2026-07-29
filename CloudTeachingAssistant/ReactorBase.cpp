#include "ReactorBase.h"

CReactorBase::CReactorBase(int nId) : m_nID(nId), m_bIsShutdown(false)
{
}

CReactorBase::~CReactorBase()
{
}

void CReactorBase::Start()
{
	m_bIsShutdown = false;

	while(!m_bIsShutdown)
	{
		//处理定时事件
		this->m_timerQueue.HandleTimerEvent();
		//处理IO事件
		this->bHandleEvent();
	}
}

void CReactorBase::Stop()
{
	m_bIsShutdown = true;
}

TimerId CReactorBase::AddTimer(const TimerEvent& event, uint32_t nMsec)
{
	return m_timerQueue.AddTimer(event, nMsec);
}

void CReactorBase::RemoveTimer(TimerId timerId)
{
	m_timerQueue.RemoveTimer(timerId);
}
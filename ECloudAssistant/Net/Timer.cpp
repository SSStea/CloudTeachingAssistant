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

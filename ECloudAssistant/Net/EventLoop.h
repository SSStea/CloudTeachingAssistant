#pragma once
#include "ReactorSelect.h"
#include <vector>

class CEventLoop
{
public:
	CEventLoop(uint32_t nThreadNums = -1);
	~CEventLoop();

	CEventLoop(const CEventLoop&) = delete;
	CEventLoop& operator=(const CEventLoop&) = delete;

	std::shared_ptr<CReactorBase> GetReactor();
	TimerId AddTimer(const TimerEvent& event, uint32_t nMsec);//添加定时器，参数为事件、触发时间
	void RemoveTimer(TimerId timerId);

	void UpdateChannel(ChannelPtr channel);
	void RemoveChannel(ChannelPtr& channel);

	void Loop();
	void Quit();

private:
	uint32_t m_nReactorIndex = 1;//reactor索引
	uint32_t m_nThreadNums = -1; //线程数量
	std::vector<std::shared_ptr<CReactorBase>> m_vecReactors; //存放reactor的vector容器
	std::vector<std::shared_ptr<std::thread>> m_vecThreads; //存放线程
};

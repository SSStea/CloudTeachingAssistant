#pragma once
#include <map>
#include <unordered_map>
#include <cstdint>
#include <functional>
#include <chrono>
#include <memory>
#include <thread>

//定时实践的回调函数，返回true则是循环执行，false则为一次性事件
typedef std::function<bool(void)> TimerEvent;
typedef uint32_t TimerId;

class CTimer//负责定时事件处理
{
public:
	CTimer(const TimerEvent& event, uint32_t nMsec);
	~CTimer() {}

	static void Sleep(uint32_t nMsec);

private:
	void SetNextTimeout(uint64_t nTimeNow); //设置下次任务的超时时间
	int64_t getNextTimeout();   //获取下次任务的超时时间

private:
	friend class CTimerQueue;

	TimerEvent eventCallback = [] {return false; };//回调函数默认返回false

	uint32_t m_nInterval = 0; //任务的执行时间
	uint64_t m_nNextTimeout = 0;//下次任务的超时时间

};


class CTimerQueue//负责CTimer对象的管理
{
public:
	CTimerQueue() {}
	~CTimerQueue() {}

public:
	TimerId AddTimer(const TimerEvent& event, uint32_t nMsec);//添加定时事件
	void RemoveTimer(TimerId nID);//删除
	void HandleTimerEvent(); //处理

protected:
	int64_t nGetTimeNow(); //获取当前时间

private:
	uint32_t nLastTimeId = 0;//最后一个定时事件的id

	//定时任务表，用定时事件的id排序，存放所有定时事件
	std::unordered_map<TimerId, std::shared_ptr<CTimer>> m_mapTimers;
	//定时事件表，用pair<超时事件，定时事件id>排序，存放所有定时事件
	std::map<std::pair<int64_t, TimerId>, std::shared_ptr<CTimer>> m_mapEvents;
};

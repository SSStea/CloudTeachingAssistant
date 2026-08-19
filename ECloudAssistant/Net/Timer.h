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

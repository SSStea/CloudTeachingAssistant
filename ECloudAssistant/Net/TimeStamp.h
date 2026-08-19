#pragma once

#include <chrono>

class CTimeStamp
{
public:
	CTimeStamp()
		: m_timeBegin(std::chrono::high_resolution_clock::now())
	{
	}

	void Reset()
	{
		m_timeBegin = std::chrono::high_resolution_clock::now();
	}

	int64_t nElapsed() const
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now() - m_timeBegin).count();
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_timeBegin;
};

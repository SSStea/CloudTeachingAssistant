#pragma once
#include <cstdint>
#include <array>
#include "logindefine.h"


#pragma pack(push,1)
//登录
struct LoginInfo : public PacketHead
{
	LoginInfo() :PacketHead()
	{
		nCmd = Login;
		nLen = sizeof(LoginInfo);
		nTimestamp = -1;
	}
	uint64_t nTimestamp;
};

//登录应答
struct LoginReply : public PacketHead
{
	LoginReply() :PacketHead()
	{
		nCmd = Login;  //如果请求超时，将这个nCmd置为ERROR
		nLen = sizeof(LoginReply);
		nPort = -1;
		arrIP.fill('\0');
	}
	uint16_t nPort;
	std::array<char, 16> arrIP;
};

typedef std::pair<int, MonitorBody*> MinotorPair;

struct CmpByValue
{
	bool operator()(const MinotorPair& l, const MinotorPair& r)
	{
		return l.second->nMem < r.second->nMem;//排序，从小到大排序
	}
};

#pragma pack(pop)
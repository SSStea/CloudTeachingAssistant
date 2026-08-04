#pragma once
#include <cstdint>
#include <memory>

struct RtmpMessageHeader
{
	uint8_t nTimeStamp[3];	//时间戳：3字节
	uint8_t nLength[3];		//长度：3字节
	uint8_t nTypeId;		//类型id：1字节
	uint8_t nStreamId[4];	//流id：4字节，小端存储
};

struct RtmpMessage
{
	uint32_t nTimeStamp = 0;	//时间戳
	uint32_t nLength = 0;		//长度
	uint8_t nTypeId = 0;		//类型
	uint32_t nStreamId = 0;		//流id
	uint32_t nExtendTimestamp = 0;//拓展时间戳

	uint64_t internal_nTimeStamp = 0;	//内部时间戳
	uint8_t nCodeId = 0;

	uint8_t nCsId = 0;			//chunk stream id
	uint32_t nIndex = 0;		//消息解析的位置索引
	std::shared_ptr<char> pPlayload = nullptr;

	void Clear()
	{
		nIndex = 0;
		nTimeStamp = 0;
		nExtendTimestamp = 0;
		if (nLength > 0)
		{
			pPlayload.reset(new char[nLength], std::default_delete<char[]>());
		}
	}

	bool bIsCompleted() const
	{
		if (nIndex == nLength && nLength > 0 && pPlayload != nullptr)
		{
			return true;
		}
		return false;
	}
};



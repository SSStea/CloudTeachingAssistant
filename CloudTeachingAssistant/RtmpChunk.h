#pragma once
#include "BufferReader.h"
#include "BufferWriter.h"
#include "RtmpMessage.h"
#include <map>
#include <iostream>
#include <string.h>

class CRtmpChunk
{
public:
	enum State//当前在解析头还是尾
	{
		PARSE_HEADER,
		PARSE_BODY,
	};
	CRtmpChunk();
	virtual ~CRtmpChunk();

	/*	解析数据，从读缓存中读取数据出来解析成rtmp message
		inBuffer：Socket接收到的数据缓冲区。
		outRtmpMsg：当一条完整消息解析完成时，将结果放到这里。
		返回值：本次从缓冲区消耗的字节数；负数表示失败或数据不足。
	*/
	int nParse(CBufferReader& inBuffer, RtmpMessage& outRtmpMsg);

	/*	创建块
		nCsId：RTMP Chunk Stream ID，表示这条消息属于哪条块流。
		rtmpInMsg：需要切块的完整 RTMP 消息。
		pBuf：输出缓冲区，生成的 Chunk 数据写到这里。
		nBufSize：pBuf 的总容量。
		返回值：理论上返回最终写入的字节数；失败返回 -1。
	*/
	int nCreateChunk(uint32_t nCsId, RtmpMessage& rtmpInMsg, char* pBuf, uint32_t nBufSize);
	
	//设置输入块大小
	void SetInChunkSize(uint32_t nInChunkSize)
	{
		m_nInChunkSize = nInChunkSize;
	}
	//设置输出块大小
	void SetOutChunkSize(uint32_t nOutChunk_Size)
	{
		m_nOutChunkSize = nOutChunk_Size;
	}

	void Clear()
	{
		m_mapRtmpMessage.clear();
	}

	int nGetStreamId()const {
		return m_nStreamId;
	}
protected:
	//解析块头
	int nParseChunkHeader(CBufferReader& buffer);
	//解析块体（块荷载），这个函数每次最多读取一个Chunk的负载，不一定读取完整Message
	int nParseChunkBody(CBufferReader& buffer);
	//创建基本头：包含Chunk Stream Id和 Chunk Type
	int nCreateBasicHeader(uint8_t nFmt, uint32_t nCsId, char* pBuf);
	//创建数据流头
	int nCreateMessageHeader(uint8_t nFmt, RtmpMessage& rtmpMsg, char* pBuf);
private:
	State m_state;					//当前状态
	int m_nChunkStreamId = 0;		//块流Id
	static int m_nStreamId;			//流Id，唯一值，通过函数递增
	uint32_t m_nInChunkSize = 128;	//输入，接收块大小
	uint32_t m_nOutChunkSize = 128;	//输出，发送块大小
	//用于同时维护不同 块流Id 的RTMP消息
	std::map<int, RtmpMessage> m_mapRtmpMessage; //发送消息映射表
	const int nKChunkMessageHeaderLength[4] = { 11,7,3,0 }; //分块的消息头长度
};


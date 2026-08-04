#include "RtmpChunk.h"

int CRtmpChunk::m_nStreamId = 0;

CRtmpChunk::CRtmpChunk()
{
	m_state = PARSE_HEADER;
	m_nChunkStreamId = -1;
	m_nStreamId++;
}

CRtmpChunk::~CRtmpChunk()
{
}

int CRtmpChunk::nParse(CBufferReader& inBuffer, RtmpMessage& outRtmpMsg)
{
	int nRet = 0;
	if (!inBuffer.nReadableBytes())
	{
		std::cout << "no data readable" << std::endl;
		return 0;
	}

	if (m_state == PARSE_HEADER)
	{
		nRet = nParseChunkHeader(inBuffer);
	}
	else
	{
		nRet = nParseChunkBody(inBuffer);
		//成功读取了一些Chunk Body，当前存在有效的Chunk Stream
		if (nRet > 0 && m_nChunkStreamId >= 0)//解析成功
		{
			//从消息映射表中取得当前Chunk Stream对应的消息
			auto& rtmpMsg = m_mapRtmpMessage[m_nChunkStreamId];
			if (rtmpMsg.nIndex == rtmpMsg.nLength)//message完整
			{
				//更新消息的最终时间戳：RTMP头部普通时间戳只有3字节，最大值：0xFFFFFF，
				//如果普通时间戳字段是 0xFFFFFF，真正的时间戳保存在4字节扩展时间戳中
				if (rtmpMsg.nTimeStamp >= 0xffffff)
				{
					rtmpMsg.internal_nTimeStamp += rtmpMsg.nExtendTimestamp;
				}
				else
				{
					rtmpMsg.internal_nTimeStamp += rtmpMsg.nTimeStamp;
				}
				//对 fmt 0 来说，前面会把 internal_nTimeStamp 清零，所以这里相当于设置绝对时间戳。
				//对 fmt 1 / 2 来说，时间戳是相对上一条消息的增量，所以这里使用 +=

				//完整消息复制给调用者。因为 RtmpMessage::playload 是 shared_ptr，
				// 这里不会复制整块负载数据，只会复制智能指针并增加引用计数。
				outRtmpMsg = rtmpMsg;
				m_nChunkStreamId = -1;
				//重置内部消息状态，为同一个 csid 的下一条消息做准备
				rtmpMsg.Clear();
			}
		}
	}

	/*
		返回本次消耗的字节数。它返回的不是“完整消息长度”，
		而只是本次解析Header或Body所消耗的长度。
	*/
	return nRet;
}

int CRtmpChunk::nCreateChunk(uint32_t nCsId, RtmpMessage& rtmpInMsg, char* pBuf, uint32_t nBufSize)
{
	//buf偏移量，记录已经向pBuf写入了多少字节
	uint32_t nBufOffset = 0;
	//载荷偏移量，记录已经复制了多少消息负载
	uint32_t nPayloadOffset = 0;
	//块容量
	uint32_t nCapacity = rtmpInMsg.nLength + rtmpInMsg.nLength / m_nOutChunkSize * 5;
	
	if (nBufSize < nCapacity)
	{
		std::cout << "nBufSize < nCapacity" << std::endl;
		return -1;
	}

	//创建头
	nBufOffset += nCreateBasicHeader(0, nCsId, pBuf + nBufOffset);
	nBufOffset += nCreateMessageHeader(0, rtmpInMsg, pBuf + nBufOffset);
	//RTMP 普通时间戳字段只有3字节，最大值为：0xFFFFFF，如果时间戳大于或等于这个值
	if (rtmpInMsg.internal_nTimeStamp >= 0xffffff)
	{
		//再额外写入一个4字节扩展时间戳
		WriteUint32BE((char*)pBuf + nBufOffset, (uint32_t)rtmpInMsg.nExtendTimestamp);
		nBufOffset += 4;
	}

	//创建块
	while (rtmpInMsg.nLength > 0)
	{
		if (rtmpInMsg.nLength > m_nOutChunkSize)
		{
			//把消息中的载荷数据按输出块大小复制到pBuf中
			memcpy(pBuf + nBufOffset, rtmpInMsg.playload.get() + nPayloadOffset, m_nOutChunkSize);
			nPayloadOffset += m_nOutChunkSize;
			nBufOffset += m_nOutChunkSize;
			rtmpInMsg.nLength -= m_nOutChunkSize;

			nBufOffset += nCreateBasicHeader(3, nCsId, pBuf + nBufOffset);
			if (rtmpInMsg.internal_nTimeStamp >= 0xffffff)
			{//如果第一块使用了扩展时间戳，后续 fmt 3 的 Chunk 也需要携带扩展时间戳
				WriteUint32BE((char*)pBuf + nBufOffset, (uint32_t)rtmpInMsg.nExtendTimestamp);
				nBufOffset += 4;
			}
		}
		else//最后一个包
		{
			memcpy(pBuf + nBufOffset, rtmpInMsg.playload.get() + nPayloadOffset, rtmpInMsg.nLength);
			nBufOffset += rtmpInMsg.nLength;
			rtmpInMsg.nLength = 0;
			break;
		}
	}

	return nBufOffset;
}

int CRtmpChunk::nParseChunkHeader(CBufferReader& buffer)
{
	uint32_t nBytesUsed = 0;
	uint8_t* pBuf = (uint8_t*)buffer.pPeek();
	uint32_t nBufSize = buffer.nReadableBytes();

	//获取fmt，第一个字节的高2位时fmt，所以需要右移6位
	uint8_t nFlags = pBuf[nBytesUsed];
	uint8_t nFmt = (nFlags >> 6);
	if (nFmt >= 4)
	{
		std::cout << "fmt is too large" << std::endl;
		return -1;
	}

	nBytesUsed += 1;
	
	uint8_t nCsId = nFlags & 0x3f;
	//2字节，低6位为0表示Basic Header总长度为2字节，真正的csid为：64 + 后面1字节
	if (nCsId == 0) 
	{
		if (nBufSize < (nBytesUsed + 1))
		{
			std::cout << "chunk stream id is not complete" << std::endl;
			return -2;
		}

		nCsId += (uint8_t)(pBuf[nBytesUsed] + 64);
		nBytesUsed += 1;
	}
	//低6位为1表示Basic Header总长度为3字节。真正的csid应该是：
	// 64 + 第2字节 + 第3字节 × 256
	else if(nCsId == 1) 
	{
		if (nBufSize < (nBytesUsed + 2))
		{
			std::cout << "chunk stream id is not complete" << std::endl;
			return -3;
		}

		nCsId = (uint8_t)(pBuf[nBytesUsed + 1] * 256 + pBuf[nBytesUsed] + 64);
		nBytesUsed += 2;
	}

	uint32_t nHeaderLen = nKChunkMessageHeaderLength[nFmt];
	//检查缓冲区中是否有完整Message Header
	if (nBufSize < (nHeaderLen + nBytesUsed))
	{
		std::cout << "data is not complete" << std::endl;
		return -4;
	}

	/*创建临时Header对象：
		先全部清零；
		根据fmt复制11、7、3或0字节；
		更新已解析长度
	*/
	RtmpMessageHeader header;
	memset(&header, 0, sizeof(header));
	memcpy(&header, pBuf + nBytesUsed, nHeaderLen);
	nBytesUsed += nHeaderLen;

	auto& rtmpMsg = m_mapRtmpMessage[nCsId];
	m_nChunkStreamId = rtmpMsg.nCsId = nCsId;

	//因为fmt为0、1都有消息长度、消息类型
	if (nFmt == 0 || nFmt == 1)
	{
		uint32_t nLength = ReadUint24BE((char*)header.nLength);
		//如果长度变化，或者还没有负载缓冲区，就重新分配内存
		//智能指针负责在消息不再使用时自动调用 delete[]
		if (rtmpMsg.nLength != nLength || !rtmpMsg.playload)
		{
			rtmpMsg.nLength = nLength;
			rtmpMsg.playload.reset(new char[rtmpMsg.nLength], std::default_delete<char[]>());
		}
		//开始接收一条新消息，将当前负载写入位置重置到0。
		rtmpMsg.nIndex = 0;
		//保存RTMP消息类型，例如音频、视频或控制消息。
		rtmpMsg.nTypeId = header.nTypeId;
	}

	//只有fmt 0携带4字节Message Stream ID
	// RTMP规定该字段使用小端字节序，所以这里调用 ReadUint32LE()
	// fmt 1/2/3继续复用之前保存的 nStreamId
	if (nFmt == 0)
	{
		rtmpMsg.nStreamId = ReadUint32LE((char*)header.nStreamId);
	}

	//读取三字节的时间戳
	uint32_t nTimeStamp = ReadUint24BE((char*)header.nTimeStamp);
	//准备保存可能存在的4字节扩展时间戳	
	uint32_t nExtendTimeStamp = 0;
	//当前Header的3字节时间戳为 0xFFFFFF 或 fmt3没有时间戳字段，但之前的Header使用了扩展时间戳
	if (nTimeStamp >= 0xffffff || rtmpMsg.nTimeStamp >= 0xffffff)
	{
		if (nBufSize < (nBytesUsed + 4))
		{
			std::cout << "data is not complete" << std::endl;
			return -5;
		}

		//按大端方式读取4字节扩展时间戳
		nExtendTimeStamp = ReadUint32BE((char*)pBuf + nBytesUsed);
		nBytesUsed += 4;
	}

	//只有当前Chunk是这条Message的第一块时，才更新消息时间戳。
	if (rtmpMsg.nIndex == 0)
	{
		if (nFmt == 0)
		{
			rtmpMsg.internal_nTimeStamp = 0;
			rtmpMsg.nTimeStamp = nTimeStamp;
			rtmpMsg.nExtendTimestamp = nExtendTimeStamp;
		}
		else
		{
			if (rtmpMsg.nTimeStamp >= 0xffffff)
			{
				rtmpMsg.nExtendTimestamp += nExtendTimeStamp;
			}
			else
			{
				rtmpMsg.nTimeStamp = nTimeStamp;
			}
		}
	}

	m_state = PARSE_BODY;
	//正式从接收缓冲区中移除已经解析的Header数据
	buffer.Retrieve(nBytesUsed);

	return nBytesUsed;
}

int CRtmpChunk::nParseChunkBody(CBufferReader& buffer)
{
	uint32_t nBytesUsed = 0;
	uint8_t* pBuf = (uint8_t*)buffer.pPeek();
	uint32_t nBufSize = buffer.nReadableBytes();

	//Body必须属于前面已经解析过的Header
	if (m_nChunkStreamId < 0)
	{
		std::cout << "chunk header parse fail" << std::endl;
		return -1;
	}

	//取得当前Chunk Stream正在组装的RTMP消息
	auto& rtmpMsg = m_mapRtmpMessage[m_nChunkStreamId];
	//计算这条Message还剩多少负载没有接收
	uint32_t nChunkSize = rtmpMsg.nLength - rtmpMsg.nIndex;
	//一个Chunk最多只能读取 m_nInChunkSize 字节
	if (nChunkSize > m_nInChunkSize)
	{
		nChunkSize = m_nInChunkSize;
	}
	if (nBufSize < (nChunkSize + nBytesUsed))
	{
		std::cout << "data is not complete" << std::endl;
		return -2;
	}
	if (rtmpMsg.nIndex + nChunkSize > rtmpMsg.nLength)
	{
		return -3;
	}

	/*将当前Chunk负载复制到完整消息缓冲区中的正确位置
		目标地址：playload起始地址 + 已接收长度
		源地址是当前接收缓冲区的可读位置
	*/
	memcpy(rtmpMsg.playload.get() + rtmpMsg.nIndex, pBuf + nBytesUsed, nChunkSize);
	nBytesUsed += nChunkSize;
	rtmpMsg.nIndex += nChunkSize;
	
	//解析了一个完整的message
	if (rtmpMsg.nIndex >= rtmpMsg.nLength || rtmpMsg.nIndex % m_nInChunkSize == 0)
	{
		m_state = PARSE_HEADER;
	}
	//从接收缓冲区中移除已经复制的Chunk负载
	buffer.Retrieve(nBytesUsed);

	return nBytesUsed;
}

int CRtmpChunk::nCreateBasicHeader(uint8_t nFmt, uint32_t nCsId, char* pBuf)
{
	int nLen = 0;

	if (nCsId > 64 + 255)//basic头占3个字节
	{
		pBuf[nLen++] = (char)((nFmt << 6) | 1);
		pBuf[nLen++] = (char)((nCsId - 64) & 0xff);
		pBuf[nLen++] = (char)(((nCsId - 64) >> 8) & 0xff);
	}
	else if (nCsId >= 64)//2字节
	{
		pBuf[nLen++] = (char)((nFmt << 6) | 0);
		pBuf[nLen++] = (char)((nCsId - 64) & 0xff);
	}
	else//1字节
	{
		pBuf[nLen++] = (char)((nFmt << 6) | nCsId);
	}

	return nLen;
}

int CRtmpChunk::nCreateMessageHeader(uint8_t nFmt, RtmpMessage& rtmpMsg, char* pBuf)
{
	int nLen = 0;
	
	if (nFmt <= 2)
	{
		if (rtmpMsg.internal_nTimeStamp < 0xffffff)
		{
			WriteUint24BE((char*)pBuf, (uint32_t)rtmpMsg.internal_nTimeStamp);
		}
		else
		{
			WriteUint24BE((char*)pBuf, 0xffffff);
		}
		nLen += 3;
	}
	if (nFmt <= 1)
	{
		WriteUint24BE((char*)pBuf + nLen, rtmpMsg.nLength);
		nLen += 3;
		pBuf[nLen++] = rtmpMsg.nTypeId;
	}
	if (nFmt == 0)
	{
		WriteUint32LE((char*)pBuf + nLen, rtmpMsg.nStreamId);
		nLen += 4;
	}

	return nLen;
}

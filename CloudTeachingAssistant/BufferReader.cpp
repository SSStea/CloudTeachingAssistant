#include "BufferReader.h"

CBufferReader::CBufferReader(uint32_t nInitSize)
{
	m_vecBuffer_.resize(nInitSize);
}

CBufferReader::~CBufferReader()
{
}

uint32_t CBufferReader::nReadableBytes() const
{
	return m_nWriteIndex - m_nReadIndex;
}

uint32_t CBufferReader::nWritableBytes() const
{
	return (uint32_t)(m_vecBuffer_.size()) - m_nWriteIndex;
}

char* CBufferReader::pPeek()
{
	return pBegin() + m_nReadIndex;
}

const char* CBufferReader::pPeek() const
{
	return pBegin() + m_nReadIndex;
}

void CBufferReader::RetrieveAll()
{
	m_nReadIndex = 0;
	m_nWriteIndex = 0;
}

void CBufferReader::Retrieve(size_t nLen/*nLen是当前读的长度*/)
{
	if (nLen <= nReadableBytes())//比当前可读大小要小
	{
		m_nReadIndex += (uint32_t)nLen;//更新读索引
		if (m_nReadIndex == m_nWriteIndex)//读索引等于写索引表示读完了
		{
			RetrieveAll();//重置
		}
	}
	else
	{
		RetrieveAll();//重置
	}
}

int CBufferReader::nReadFromSocket(int nFd)
{
	//简单扩容
	uint32_t nSize = nWritableBytes();//目前缓冲区可写的大小
	if (nSize < MAX_BYTES_PER_READ)//如果小于每次可以从socket中读出来的最大长度
	{
		uint32_t bufferReadSize = (uint32_t)m_vecBuffer_.size();//缓冲区当前大小
		if (bufferReadSize < MAX_BUFFER_SIZE)//如果缓冲区当前大小比最大缓冲区大小要小
		{
			m_vecBuffer_.resize(bufferReadSize + MAX_BYTES_PER_READ);//扩容
		}
		else//如果缓冲区当前大小已经比最大缓冲区大小要大
		{//就不能再从socket中读数据往里写，要返回，等待缓冲区的数据被处理
			return 0;
		}
	}

	//读数据，从socket接收数据到缓冲区可写的位置
	ssize_t nBytesRecv = ::recv(nFd, pBeginWrite(), (size_t)MAX_BYTES_PER_READ, 0);
	if (nBytesRecv > 0)
	{
		m_nWriteIndex += (uint32_t)nBytesRecv;
	}

	return (int)nBytesRecv;
}

uint32_t CBufferReader::nReadFromBuffer(std::string& data)
{
	uint32_t nSize = nReadableBytes();//获取缓冲区可读大小
	if (nSize > 0)
	{
		data.assign(pPeek(), nSize);//将缓冲区中的数据从可读的首地址开始，按可读大小全部转移到data中
		m_nReadIndex = 0;
		m_nWriteIndex = 0;
	}
	return nSize;
}

uint32_t CBufferReader::nSize() const
{
	return (uint32_t)m_vecBuffer_.size();
}

char* CBufferReader::pBegin()
{
	return &*m_vecBuffer_.begin();
}

const char* CBufferReader::pBegin() const
{
	return &*m_vecBuffer_.begin();
}

char* CBufferReader::pBeginWrite()
{
	return pBegin() + m_nWriteIndex;
}

const char* CBufferReader::pBeginWrite() const
{
	return pBegin() + m_nWriteIndex;
}

uint32_t ReadUint32BE(char* data)
{
	uint8_t* p = (uint8_t*)data;
	uint32_t value = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
	return value;
}

uint32_t ReadUint32LE(char* data)
{
	uint8_t* p = (uint8_t*)data;
	uint32_t value = (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
	return value;
}

uint32_t ReadUint24BE(char* data)
{
	uint8_t* p = (uint8_t*)data;
	uint32_t value = (p[0] << 16) | (p[1] << 8) | p[2];
	return value;
}

uint32_t ReadUint24LE(char* data)
{
	uint8_t* p = (uint8_t*)data;
	uint32_t value = (p[2] << 16) | (p[1] << 8) | p[0];
	return value;
}

uint16_t ReadUint16BE(char* data)
{
	uint8_t* p = (uint8_t*)data;
	uint16_t value = (p[0] << 8) | p[1];
	return value;
}

uint16_t ReadUint16LE(char* data)
{
	uint8_t* p = (uint8_t*)data;
	uint16_t value = (p[1] << 8) | p[0];
	return value;
}

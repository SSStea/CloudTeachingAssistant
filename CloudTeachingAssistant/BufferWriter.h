#pragma once
#include <memory>
#include <queue>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>

void WriteUint32BE(char* p, uint32_t value);//大端写4字节
void WriteUint32LE(char* p, uint32_t value);//小端写4字节
void WriteUint24BE(char* p, uint32_t value);//3字节
void WriteUint24LE(char* p, uint32_t value);
void WriteUint16BE(char* p, uint32_t value);//2字节
void WriteUint16LE(char* p, uint32_t value);

class CBufferWriter
{
public:
	CBufferWriter(int capacity = m_nMaxQueueLength);
	~CBufferWriter() {}

	bool bAppend(std::shared_ptr<char> pData, uint32_t nSize, uint32_t nIndex = 0);
	bool bAppend(const char* pData, uint32_t nSize, uint32_t nIndex = 0);
	int nSend(int sockfd);

	bool bIsEmpty() const
	{
		return m_qBuffer_.empty();
	}

	bool bIsFull() const
	{
		return ((int)m_qBuffer_.size() >= m_nMaxQueueLength_ ? true : false);
	}

	uint32_t nSize() const
	{
		return (uint32_t)m_qBuffer_.size();
	}

private:
	typedef struct
	{
		//数据，使用智能指针保证数据在队列中等待发送期间不会被提前释放。
		// 如果调用方new data，在调用bAppend之后立即delete，此时pkt还在队列中没有发送，当
		// 轮到这些数据发送时由于调用方已经销毁指针，就可能出现崩溃或乱码，使用智能指针后，
		// 调用方和队列共同持有数据，即使调用方释放自己的智能指针，队列里的Packet仍然持有
		// 数据，所以缓冲区不会释放。只有当数据发送完成、对应的 Packet 被移出队列，
		// 且没有其他持有者时，缓冲区才会自动释放
		std::shared_ptr<char> data;
		uint32_t size;//数据大小
		uint32_t writeIndex;//写索引，判断当前数据发送到什么位置
	} Packet;//用户将数据打包未packet，添加进队列

	std::queue<Packet>	m_qBuffer_;//通过队列存储需要发送的数据包
	int					m_nMaxQueueLength_ = 0;
	static const int	m_nMaxQueueLength = 10000;//队列能支持的最大长度
};


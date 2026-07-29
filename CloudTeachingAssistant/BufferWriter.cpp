#include "BufferWriter.h"

CBufferWriter::CBufferWriter(int capacity) : m_nMaxQueueLength_(capacity)
{

}

bool CBufferWriter::bAppend(std::shared_ptr<char> pData, uint32_t nSize, uint32_t nIndex)
{
	if (nSize < nIndex)//索引从0开始，如果大小小于0说明没有数据
	{
		return false;
	}

	if (m_qBuffer_.size() >= (size_t)m_nMaxQueueLength_)//如果当前数据包队列长度大于最大长度
	{//队列没有多余位置
		return false;
	}

	Packet pkt = {pData, nSize, nIndex};
	m_qBuffer_.emplace(std::move(pkt));

	return true;
}

bool CBufferWriter::bAppend(const char* pData, uint32_t nSize, uint32_t nIndex)
{
	if (nSize < nIndex)//索引从0开始，如果大小小于0说明没有数据
	{
		return false;
	}

	if (m_qBuffer_.size() >= (size_t)m_nMaxQueueLength_)//如果当前数据包队列长度大于最大长度
	{//队列没有多余位置
		return false;
	}

	Packet pkt;
	//使用智能指针管理复制出来的内存，Packet 销毁时自动释放，不需要手动 delete[]
	pkt.data.reset(new char[nSize + 512], std::default_delete<char[]>());
	//复制调用者传入的数据，避免依赖原始 pData 的生命周期
	memcpy(pkt.data.get(), pData, nSize);
	pkt.size = nSize;
	pkt.writeIndex = nIndex;
	m_qBuffer_.push(std::move(pkt));

	return true;
}

int CBufferWriter::nSend(int sockfd)
{
	ssize_t nBytesSend = 0;
	int nCnt = 1;

	do 
	{
		if (m_qBuffer_.empty())
		{
			return 0;
		}

		nCnt -= 1;
		Packet& pkt = m_qBuffer_.front();
		nBytesSend = ::send(sockfd, pkt.data.get() + pkt.writeIndex, (size_t)(pkt.size - pkt.writeIndex), 0);

		if (nBytesSend > 0)
		{
			pkt.writeIndex += (uint32_t)nBytesSend;
			if (pkt.size == pkt.writeIndex)
			{
				nCnt += 1;
				m_qBuffer_.pop();
			}
		}
		else if (nBytesSend < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				nBytesSend = 0;
			}
		}

	} while (nCnt > 0);

	return (int)nBytesSend;
}

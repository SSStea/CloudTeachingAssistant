#include "RtmpSession.h"
#include "RtmpConnection.h"

CRtmpSession::CRtmpSession()
{
}

CRtmpSession::~CRtmpSession()
{
}

void CRtmpSession::AddSink(std::shared_ptr<CRtmpSink> pSink)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	m_mapRtmpSinks[pSink->nGetId()] = pSink;
	if (pSink->bIsPublisher()) // 如果是推流者，说明刚刚创建直播间，没有设置元数据
	{
		m_pAvcSequenceHeader = nullptr;
		m_pAacSequenceHeader = nullptr;
		m_nAvcSequenceHeaderSize = 0;
		m_nAacSequenceHeaderSize = 0;
		m_bHasPublisher = true;
		m_pPublisher = pSink;
	}
}

void CRtmpSession::RemoveSink(std::shared_ptr<CRtmpSink> pSink)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if (pSink->bIsPublisher())
	{
		m_pAvcSequenceHeader = nullptr;
		m_pAacSequenceHeader = nullptr;
		m_nAvcSequenceHeaderSize = 0;
		m_nAacSequenceHeaderSize = 0;
		m_bHasPublisher = false;
	}

	m_mapRtmpSinks.erase(pSink->nGetId());
}

int CRtmpSession::nGetClients()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	int nClients = 0;

	for (auto it : m_mapRtmpSinks)
	{
		auto conn = it.second.lock();
		if (conn != nullptr)
		{
			nClients++;
		}
	}

	return nClients;
}

void CRtmpSession::SendMetaData(mapAmfObjects& metaData)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	for (auto it = m_mapRtmpSinks.begin(); it != m_mapRtmpSinks.end();)
	{
		auto conn = it->second.lock();
		if (!conn)
		{
			m_mapRtmpSinks.erase(it++);
		}
		else
		{
			if (conn->bIsPlayer())
			{
				conn->bSendMetaData(metaData);
			}
			it++;
		}
	}
}

void CRtmpSession::SendMediaData(uint8_t nType, uint64_t nTimeStamp, std::shared_ptr<char> pData, uint32_t nSize)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	for (auto it = m_mapRtmpSinks.begin(); it != m_mapRtmpSinks.end();)
	{
		auto conn = it->second.lock();
		if (!conn)
		{
			m_mapRtmpSinks.erase(it++);
		}
		else//在线用户，观看中：发送音视频，刚进直播间：先发送元数据再发送音视频
		{
			if (conn->bIsPlayer())
			{
				if (!conn->bIsPlaying())//刚进直播间
				{
					//音频、视频元数据
					conn->bSendMediaData(RTMP_AAC_SEQUENCE_HEADER, nTimeStamp, m_pAacSequenceHeader,m_nAacSequenceHeaderSize);
					conn->bSendMediaData(RTMP_AVC_SEQUENCE_HEADER, nTimeStamp, m_pAvcSequenceHeader, m_nAvcSequenceHeaderSize);
				}

				//已经观看中
				conn->bSendMediaData(nType, nTimeStamp, pData, nSize);
			}
			it++;
		}
	}
}

std::shared_ptr<CRtmpConnection> CRtmpSession::pGetPublisher()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto publisher = m_pPublisher.lock();
	if (publisher)
	{
		return std::dynamic_pointer_cast<CRtmpConnection>(publisher);
	}

	return nullptr;
}

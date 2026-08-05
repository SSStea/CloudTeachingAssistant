#pragma once
#include <memory>
#include <mutex>
#include "RtmpSink.h"
#include "amf.h"
#include "Rtmp.h"

class CRtmpSink;
class RtmpConnection;
class CRtmpSession
{
public:
	using Ptr = std::shared_ptr<CRtmpSession>;
	CRtmpSession();
	virtual ~CRtmpSession();
	void SetAvcSequenceHeader(std::shared_ptr<char> pAvcSequenceHeader, uint32_t nAvcSequenceHeaderSize)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_pAvcSequenceHeader = pAvcSequenceHeader;
		m_nAvcSequenceHeaderSize = nAvcSequenceHeaderSize;
	}

	void SetAacSequenceHeader(std::shared_ptr<char> pAacSequenceHeader, uint32_t nAacSequenceHeaderSize)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_pAacSequenceHeader = pAacSequenceHeader;
		m_nAacSequenceHeaderSize = nAacSequenceHeaderSize;
	}

	//session添加客户端
	void AddSink(std::shared_ptr<CRtmpSink> pSink);
	//移除客户端
	void RemoveSink(std::shared_ptr<CRtmpSink> pSink);
	//获取当前有多少人在直播间，包括主播和观众
	int nGetClients();
	//发送消息通知，通知元数据
	void SendMetaData(mapAmfObjects& metaData);
	//发送数据
	void SendMediaData(uint8_t nType, uint64_t nTimeStamp, std::shared_ptr<char> pData, uint32_t nSize);
	//获取推流对象
	std::shared_ptr<RtmpConnection> pGetPublisher();
private:
	std::mutex m_mutex;
	bool m_bHasPublisher = false;
	std::weak_ptr<CRtmpSink> m_pPublisher;

	//管理客户端的接入和移出
	std::unordered_map<int, std::weak_ptr<CRtmpSink>> m_mapRtmpSinks;

	//视频信息元数据
	std::shared_ptr<char> m_pAvcSequenceHeader;
	//音频信息元数据
	std::shared_ptr<char> m_pAacSequenceHeader;
	uint32_t m_nAvcSequenceHeaderSize = 0;
	uint32_t m_nAacSequenceHeaderSize = 0;
};


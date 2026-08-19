#include "RtmpPublisher.h"

#include <cstring>

std::shared_ptr<CRtmpPublisher> CRtmpPublisher::pCreate(CEventLoop* pEventLoop)
{
	return std::shared_ptr<CRtmpPublisher>(new CRtmpPublisher(pEventLoop));
}

CRtmpPublisher::CRtmpPublisher(CEventLoop* pEventLoop)
	: m_pEventLoop(pEventLoop)
{
}

CRtmpPublisher::~CRtmpPublisher()
{
}

int CRtmpPublisher::nSetMediaInfo(RtmpMediaInfo mediaInfo)
{
	m_mediaInfo = mediaInfo;

	if (m_mediaInfo.m_nAudioCodecId == RTMP_CODEC_ID_AAC)
	{
        //如果有视频配置信息
		if (m_mediaInfo.m_nAudioSpecificConfigSize > 0)
		{
            m_nAacSequenceHeaderSize = m_mediaInfo.m_nAudioSpecificConfigSize + 2;
			m_pAacSequenceHeader.reset(new char[m_nAacSequenceHeaderSize],std::default_delete<char[]>());

			uint8_t* pData = (uint8_t*)m_pAacSequenceHeader.get();
			pData[0] = 0xAF;
			pData[1] = 0;
            //把配置信息拷贝到aac头
            memcpy(pData + 2, m_mediaInfo.m_pAudioSpecificConfig.get(),
                   m_mediaInfo.m_nAudioSpecificConfigSize);
		}
		else
		{
			m_mediaInfo.m_nAudioCodecId = 0;
		}
	}

	if (m_mediaInfo.m_nVideoCodecId == RTMP_CODEC_ID_H264)
	{
        //如果音频的有sps、pps数据
		if (m_mediaInfo.m_nSpsSize > 0 && m_mediaInfo.m_nPpsSize > 0)
		{
			m_pAvcSequenceHeader.reset(new char[4096],std::default_delete<char[]>());
			uint8_t* pData = (uint8_t*)m_pAvcSequenceHeader.get();
			uint32_t nIndex = 0;

			pData[nIndex++] = 0x17;
			pData[nIndex++] = 0;
			pData[nIndex++] = 0;
			pData[nIndex++] = 0;
			pData[nIndex++] = 0;
			pData[nIndex++] = 0x01;
			pData[nIndex++] = m_mediaInfo.m_pSps.get()[1];
			pData[nIndex++] = m_mediaInfo.m_pSps.get()[2];
			pData[nIndex++] = m_mediaInfo.m_pSps.get()[3];
			pData[nIndex++] = 0xff;
			pData[nIndex++] = 0xE1;
			pData[nIndex++] = (uint8_t)(m_mediaInfo.m_nSpsSize >> 8);
			pData[nIndex++] = (uint8_t)(m_mediaInfo.m_nSpsSize & 0xff);
            memcpy(pData + nIndex, m_mediaInfo.m_pSps.get(), m_mediaInfo.m_nSpsSize);
			nIndex += m_mediaInfo.m_nSpsSize;

			pData[nIndex++] = 0x01;
			pData[nIndex++] = (uint8_t)(m_mediaInfo.m_nPpsSize >> 8);
			pData[nIndex++] = (uint8_t)(m_mediaInfo.m_nPpsSize & 0xff);
            memcpy(pData + nIndex, m_mediaInfo.m_pPps.get(), m_mediaInfo.m_nPpsSize);
			nIndex += m_mediaInfo.m_nPpsSize;
			m_nAvcSequenceHeaderSize = nIndex;
		}
	}

	return 0;
}

int CRtmpPublisher::nOpenUrl(std::string strUrl, int nMsec)
{
	if (nParseRtmpUrl(strUrl) != 0)
	{
		return -1;
	}

    //如果已经有rtmp连接了，断开连接
	if (m_pRtmpConnection)
	{
		std::shared_ptr<CRtmpConnection> pConnection = m_pRtmpConnection;
		m_pRtmpConnection = nullptr;
		pConnection->disConnect();
	}

	CTcpSocket tcpSocket;
	tcpSocket.nCreate();
	if (!tcpSocket.bConnect(m_strIP, m_nPort, nMsec))
	{
		tcpSocket.Close();
		return -1;
	}

	m_pRtmpConnection.reset(new CRtmpConnection(
		shared_from_this(),
		m_pEventLoop->GetReactor().get(),
		tcpSocket.nGetSocket()));

	m_pRtmpConnection->bHandshake();

	return 0;
}

int CRtmpPublisher::nPushVideoFrame(uint8_t* pData, uint32_t nSize)
{
	if (!m_pRtmpConnection || m_pRtmpConnection->bIsClosed() || nSize <= 5)
	{
		return -1;
	}

    //如果是H.264格式并且还未发送关键帧
	if (m_mediaInfo.m_nVideoCodecId == RTMP_CODEC_ID_H264 && !m_bHasKeyFrame)
	{
        //判断关键帧，不是说明数据不全
		if (!bIsKeyFrame(pData, nSize))
		{
			return 0;
		}

		m_bHasKeyFrame = true;
        //开始推送第一帧视频前，把播放器需要的音视频解码配置一起发出去
		m_pRtmpConnection->bSendVideoData(
			0,
			m_pAvcSequenceHeader,
			m_nAvcSequenceHeaderSize);

        m_pRtmpConnection->bSendAudioData(
			0,
			m_pAacSequenceHeader,
			m_nAacSequenceHeaderSize);
	}

	uint64_t nTimeStamp = (uint64_t)m_timeStamp.nElapsed();
    std::shared_ptr<char> pPlayload(new char[nSize + 4096], std::default_delete<char[]>());
	uint8_t* pBody = (uint8_t*)pPlayload.get();
	uint32_t nIndex = 0;
	pBody[nIndex++] = bIsKeyFrame(pData, nSize) ? 0x17 : 0x27;
	pBody[nIndex++] = 1;
	pBody[nIndex++] = 0;
	pBody[nIndex++] = 0;
	pBody[nIndex++] = 0;
	pBody[nIndex++] = (uint8_t)((nSize >> 24) & 0xff);
	pBody[nIndex++] = (uint8_t)((nSize >> 16) & 0xff);
	pBody[nIndex++] = (uint8_t)((nSize >> 8) & 0xff);
	pBody[nIndex++] = (uint8_t)(nSize & 0xff);
	memcpy(pBody + nIndex, pData, nSize);
	nIndex += nSize;

	m_pRtmpConnection->bSendVideoData(nTimeStamp, pPlayload, nIndex);
	return 0;
}

int CRtmpPublisher::nPushAudioFrame(uint8_t* pData, uint32_t nSize)
{
	if (!m_pRtmpConnection || m_pRtmpConnection->bIsClosed() || nSize == 0)
	{
		return -1;
	}

    //如果是AAC格式并且已发送关键帧
	if (m_mediaInfo.m_nAudioCodecId == RTMP_CODEC_ID_AAC && m_bHasKeyFrame)
	{
		uint64_t nTimeStamp = (uint64_t)m_timeStamp.nElapsed();
		uint32_t nPlayloadSize = nSize + 2;
        std::shared_ptr<char> pPlayload(new char[nPlayloadSize], std::default_delete<char[]>());
		pPlayload.get()[0] = (char)0xAF;
		pPlayload.get()[1] = 1;
		memcpy(pPlayload.get() + 2, pData, nSize);
		m_pRtmpConnection->bSendAudioData(
			nTimeStamp,
			pPlayload,
			nPlayloadSize);
	}

	return 0;
}

void CRtmpPublisher::Close()
{
	if (m_pRtmpConnection)
	{
		std::shared_ptr<CRtmpConnection> pConnection = m_pRtmpConnection;
		m_pRtmpConnection = nullptr;
		pConnection->disConnect();
		m_bHasKeyFrame = false;
	}
}

bool CRtmpPublisher::bIsConnected() const
{
	return m_pRtmpConnection && !m_pRtmpConnection->bIsClosed();
}

bool CRtmpPublisher::bIsKeyFrame(uint8_t* pData, uint32_t nSize) const
{
	(void)nSize;

	int nStartCodeSize = 0;
	if (pData[0] == 0 && pData[1] == 0 && pData[2] == 0)
	{
		nStartCodeSize = 3;
	}
	else if (pData[0] == 0 && pData[1] == 0 && pData[2] == 0 && pData[3] == 0)
	{
		nStartCodeSize = 4;
	}

	int nType = pData[nStartCodeSize] & 0x1f;
	return nType == 5 || nType == 7;
}

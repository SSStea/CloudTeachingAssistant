#include "RtmpConnection.h"
#include "RtmpPublisher.h"

#include <QDebug>

CRtmpConnection::CRtmpConnection(std::shared_ptr<CRtmpPublisher> pPublisher, CReactorBase* pReactor, int nSocket)
	: CRtmpConnection(pReactor, nSocket, pPublisher.get())
{
	m_pHandshake.reset(new CRtmpHandshake(CRtmpHandshake::HANDSHAKE_S0S1S2));
	m_pRtmpPublisher = pPublisher;
}

CRtmpConnection::CRtmpConnection(CReactorBase* pReactor, int nSocket, CRtmp* pRtmp)
	: CTcpConnection(pReactor, nSocket)
	, m_pRtmpChunk(new CRtmpChunk())
{
    m_amfDecoder.reset(new CAmfDecoder());
    m_amfEncoder.reset(new CAmfEncoder());

	m_nMaxChunkSize = pRtmp->nGetChunkSize();
	m_strStreamPath = pRtmp->strGetStreamPath();
	m_strStreamName = pRtmp->strGetStreamName();
	m_strAppName = pRtmp->strGetApp();

	setReadCallback([this](std::shared_ptr<CTcpConnection> pConnection, CBufferReader& buffer)
	{
		(void)pConnection;
		return bOnRead(buffer);
	});

	setCloseCallback([this](std::shared_ptr<CTcpConnection> pConnection)
	{
		(void)pConnection;
		OnClose();
	});
}

CRtmpConnection::~CRtmpConnection()
{
}

bool CRtmpConnection::bOnRead(CBufferReader& buffer)
{
	bool bResult = true;
	if (m_pHandshake->bHandshakeIsCompleted())
	{
		bResult = bHandleChunk(buffer);
	}
	else
	{
		std::shared_ptr<char> pResponse(new char[4096], std::default_delete<char[]>());
		int nResponseSize = m_pHandshake->nParse(buffer, pResponse.get(), 4096);
		if (nResponseSize < 0)
		{
			bResult = false;
		}

		if (nResponseSize > 0)
		{
			Send(pResponse.get(), (uint32_t)nResponseSize);
		}

		if (m_pHandshake->bHandshakeIsCompleted())
		{
			if (buffer.nReadableBytes() > 0)
			{
				bResult = bHandleChunk(buffer);
			}

			SetChunkSize();
			bConnect();
		}
	}

	return bResult;
}

void CRtmpConnection::OnClose()
{
	bDeleteStream();
}

bool CRtmpConnection::bHandleChunk(CBufferReader& buffer)
{
	int nResult = -1;
	do
	{
		RtmpMessage rtmpMsg;
		nResult = m_pRtmpChunk->nParse(buffer, rtmpMsg);
		if (nResult < 0)
		{
			return false;
		}

		if (rtmpMsg.bIsCompleted() && !bHandleMessage(rtmpMsg))
		{
			return false;
		}

		if (nResult == 0)
		{
			break;
		}
	} while (buffer.nReadableBytes() > 0);

	return true;
}

bool CRtmpConnection::bHandleMessage(RtmpMessage& rtmpMsg)
{
	bool bResult = true;
	switch (rtmpMsg.nTypeId)
	{
	case RTMP_INVOKE:
		bResult = bHandleInvoke(rtmpMsg);
		break;
	case RTMP_SET_CHUNK_SIZE:
		m_pRtmpChunk->SetInChunkSize(ReadUint32BE(rtmpMsg.pPlayload.get()));
		break;
	default:
		break;
	}

	return bResult;
}

bool CRtmpConnection::bHandleInvoke(RtmpMessage& rtmpMsg)
{
    m_amfDecoder->Reset();

    int nBytesUsed = m_amfDecoder->nDecode(rtmpMsg.pPlayload.get(), (int)rtmpMsg.nLength, 1);
	if (nBytesUsed < 0)
	{
		return false;
	}

    std::string strMethod = m_amfDecoder->strGetString();
    nBytesUsed = m_amfDecoder->nDecode(
		rtmpMsg.pPlayload.get() + nBytesUsed,
		(int)rtmpMsg.nLength - nBytesUsed);

	if (strMethod == "_result")
	{
		return bHandleResult(rtmpMsg);
	}

	if (strMethod == "onStatus")
	{
		return bHandleOnStatus(rtmpMsg);
	}

	return true;
}

bool CRtmpConnection::bHandshake()
{
	uint32_t nSize = 1 + 1536;
	std::shared_ptr<char> pRequest(new char[nSize], std::default_delete<char[]>());
	m_pHandshake->nBuildC0C1(pRequest.get(), nSize);
	Send(pRequest.get(), nSize);
	return true;
}

bool CRtmpConnection::bConnect()
{
	mapAmfObjects mapObjects;
    m_amfEncoder->Reset();
    m_amfEncoder->encodeString("connect", 7);
    m_amfEncoder->encodeNumber((double)++m_nNumber);
	mapObjects["app"] = AmfObject(m_strAppName);
	mapObjects["type"] = AmfObject(std::string("nonprivate"));
    m_amfEncoder->encodeObjects(mapObjects);

	m_state = START_CONNECT;
    bSendInvokeMessage(RTMP_CHUNK_INVOKE_ID, m_amfEncoder->pData(), m_amfEncoder->nSize());
	qDebug() << "Connect";
	return true;
}

bool CRtmpConnection::bCreateStream()
{
	mapAmfObjects mapObjects;
    m_amfEncoder->Reset();
    m_amfEncoder->encodeString("createStream", 12);
    m_amfEncoder->encodeNumber((double)++m_nNumber);
    m_amfEncoder->encodeObjects(mapObjects);

	m_state = START_CREATE_STREAM;
    bSendInvokeMessage(RTMP_CHUNK_INVOKE_ID, m_amfEncoder->pData(), m_amfEncoder->nSize());
	qDebug() << "CreateStream";
	return true;
}

bool CRtmpConnection::bPublish()
{
	mapAmfObjects mapObjects;
    m_amfEncoder->Reset();
    m_amfEncoder->encodeString("publish", 7);
    m_amfEncoder->encodeNumber((double)++m_nNumber);
    m_amfEncoder->encodeObjects(mapObjects);
    m_amfEncoder->encodeString(m_strStreamName.c_str(), (int)m_strStreamName.size());

	m_state = START_PUBLISH;
    bSendInvokeMessage(RTMP_CHUNK_INVOKE_ID, m_amfEncoder->pData(), m_amfEncoder->nSize());
	qDebug() << "Publish";
	return true;
}

bool CRtmpConnection::bDeleteStream()
{
	mapAmfObjects mapObjects;
    m_amfEncoder->Reset();
    m_amfEncoder->encodeString("DeleteStream", 12);
    m_amfEncoder->encodeNumber((double)++m_nNumber);
    m_amfEncoder->encodeObjects(mapObjects);
    m_amfEncoder->encodeNumber(m_nStreamId);

	m_state = START_DELETE_STREAM;
    return bSendInvokeMessage(RTMP_CHUNK_INVOKE_ID, m_amfEncoder->pData(), m_amfEncoder->nSize());
}

bool CRtmpConnection::bHandleResult(RtmpMessage& rtmpMsg)
{
	(void)rtmpMsg;

	if (m_state == START_CONNECT)
	{
        if (m_amfDecoder->bHasObject("code"))
		{
            AmfObject amfObject = m_amfDecoder->GetObject("code");
			if (amfObject.strAmfString == "NetConnection.Connect.Success")
			{
				return bCreateStream();
			}
		}
	}
    else if (m_state == START_CREATE_STREAM && m_amfDecoder->fGetNumber() > 0)
	{
        m_nStreamId = (uint32_t)m_amfDecoder->fGetNumber();
		return bPublish();
	}

	return true;
}

bool CRtmpConnection::bHandleOnStatus(RtmpMessage& rtmpMsg)
{
	(void)rtmpMsg;

    if (!m_amfDecoder->bHasObject("code"))
	{
		return true;
	}

    AmfObject amfObject = m_amfDecoder->GetObject("code");
	if (m_state == START_PUBLISH)
	{
		if (amfObject.strAmfString == "NetStream.Publish.Start")
		{
			m_bIsPublishing = true;
			return true;
		}

		return false;
	}

	if (m_state == START_DELETE_STREAM)
	{
		return amfObject.strAmfString == "NetStream.Unpublish.Success";
	}

	return true;
}

bool CRtmpConnection::bSendVideoData(
	uint64_t nTimeStamp,
	std::shared_ptr<char> pPlayload,
	uint32_t nPlayloadSize)
{
	if (nPlayloadSize == 0)
	{
		return false;
	}

	RtmpMessage rtmpMsg;
	rtmpMsg.nTypeId = RTMP_VIDEO;
	rtmpMsg.internal_nTimeStamp = nTimeStamp;
	rtmpMsg.nStreamId = m_nStreamId;
	rtmpMsg.pPlayload = pPlayload;
	rtmpMsg.nLength = nPlayloadSize;
	SendRtmpChunks(RTMP_CHUNK_VIDEO_ID, rtmpMsg);
	return true;
}

bool CRtmpConnection::bSendAudioData(
	uint64_t nTimeStamp,
	std::shared_ptr<char> pPlayload,
	uint32_t nPlayloadSize)
{
	if (nPlayloadSize == 0)
	{
		return false;
	}

	RtmpMessage rtmpMsg;
	rtmpMsg.nTypeId = RTMP_AUDIO;
	rtmpMsg.internal_nTimeStamp = nTimeStamp;
	rtmpMsg.nStreamId = m_nStreamId;
	rtmpMsg.pPlayload = pPlayload;
	rtmpMsg.nLength = nPlayloadSize;
	SendRtmpChunks(RTMP_CHUNK_AUDIO_ID, rtmpMsg);
	return true;
}

void CRtmpConnection::SetChunkSize()
{
	m_pRtmpChunk->SetOutChunkSize(m_nMaxChunkSize);

	std::shared_ptr<char> pData(new char[4], std::default_delete<char[]>());
	WriteUint32BE(pData.get(), m_nMaxChunkSize);

	RtmpMessage rtmpMsg;
	rtmpMsg.nTypeId = RTMP_SET_CHUNK_SIZE;
	rtmpMsg.pPlayload = pData;
	rtmpMsg.nLength = 4;
	SendRtmpChunks(RTMP_CHUNK_CONTROL_ID, rtmpMsg);
}

bool CRtmpConnection::bSendInvokeMessage(
	uint32_t nCsId,
	std::shared_ptr<char> pPlayload,
	uint32_t nPlayloadSize)
{
	if (bIsClosed())
	{
		return false;
	}

	RtmpMessage rtmpMsg;
	rtmpMsg.nTypeId = RTMP_INVOKE;
	rtmpMsg.nTimeStamp = 0;
	rtmpMsg.nStreamId = m_nStreamId;
	rtmpMsg.pPlayload = pPlayload;
	rtmpMsg.nLength = nPlayloadSize;
	SendRtmpChunks(nCsId, rtmpMsg);
	return true;
}

bool CRtmpConnection::bSendNotifyMessage(
	uint32_t nCsId,
	std::shared_ptr<char> pPlayload,
	uint32_t nPlayloadSize)
{
	if (bIsClosed())
	{
		return false;
	}

	RtmpMessage rtmpMsg;
	rtmpMsg.nTypeId = RTMP_NOTIFY;
	rtmpMsg.nTimeStamp = 0;
	rtmpMsg.nStreamId = m_nStreamId;
	rtmpMsg.pPlayload = pPlayload;
	rtmpMsg.nLength = nPlayloadSize;
	SendRtmpChunks(nCsId, rtmpMsg);
	return true;
}

bool CRtmpConnection::bIsKeyFrame(std::shared_ptr<char> pData, uint32_t nSize)
{
	(void)nSize;

	uint8_t nFrameType = ((uint8_t)pData.get()[0] >> 4) & 0x0f;
	uint8_t nCodecId = (uint8_t)pData.get()[0] & 0x0f;
	return nFrameType == 1 && nCodecId == RTMP_CODEC_ID_H264;
}

void CRtmpConnection::SendRtmpChunks(uint32_t nCsId, RtmpMessage& rtmpMsg)
{
	uint32_t nCapacity =
		rtmpMsg.nLength + rtmpMsg.nLength / m_nMaxChunkSize * 5 + 1024;
	std::shared_ptr<char> pBuffer(
		new char[nCapacity],
		std::default_delete<char[]>());

	int nSize = m_pRtmpChunk->nCreateChunk(
		nCsId,
		rtmpMsg,
		pBuffer.get(),
		nCapacity);
	if (nSize > 0)
	{
		Send(pBuffer.get(), (uint32_t)nSize);
	}
}

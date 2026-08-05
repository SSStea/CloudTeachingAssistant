#include "RtmpConnection.h"

CRtmpConnection::CRtmpConnection(CReactorBase* reactor, int nSocket, CRtmp* rtmp)
	: CTcpConnection(reactor, nSocket), m_state(HANDSHAKE),
	m_pRtmpChunk(new CRtmpChunk())
{
	m_nPeerWidth = rtmp->nGetPeerBandwidth();
	m_nAcknowledgementSize = rtmp->nGetAcknowledgementSize();
	m_nMaxChunkSize = rtmp->nGetChunkSize();
	m_strStreamPath = rtmp->strGetStreamPath();
	m_strStreamName = rtmp->strGetStreamName();
	m_strAppName = rtmp->strGetApp();

	//设置回调函数，处理读数据
	this->setReadCallback([this](std::shared_ptr<CTcpConnection> conn, CBufferReader& buffer)
		{
			return this->bOnRead(buffer);
		});
	
	//设置关闭回调，释放资源
	this->setCloseCallback([this](std::shared_ptr<CTcpConnection> conn)
		{
			this->OnClose();
		});

}

CRtmpConnection::CRtmpConnection(std::shared_ptr<CRtmpServer> rtmpServer, CReactorBase* reactor, int nSocket)
	: CRtmpConnection(reactor, nSocket, rtmpServer.get())
{
	m_pHandShake.reset(new CRtmpHandshake(CRtmpHandshake::HANDSHAKE_C0C1));
	m_pRtmpServer = rtmpServer;
}

CRtmpConnection::~CRtmpConnection()
{
}

bool CRtmpConnection::bOnRead(CBufferReader& buffer)
{
	bool bRet = true;

	if (m_pHandShake->bHandshakeIsCompleted())//是否rtmp握手完成，完成之后才能发送message
	{
		bRet = bHandleChunk(buffer);
	}
	else
	{
		//分配内存，存储解析握手的结果
		std::shared_ptr<char> res(new char[4096], std::default_delete<char[]>());
		//解析握手
		int nResSize = m_pHandShake->nParse(buffer, res.get(), 4096);
		if (nResSize < 0)
		{
			bRet = false;
		}
		if(nResSize > 0)
		{
			//发送握手结果
			this->Send(res.get(), nResSize);
		}

		if (m_pHandShake->bHandshakeIsCompleted() && buffer.nReadableBytes() > 0)
		{
			bRet = bHandleChunk(buffer);
		}

	}

	return bRet;
}

void CRtmpConnection::OnClose()
{
	this->bHandleDeleteStream();
}

bool CRtmpConnection::bHandleChunk(CBufferReader& buffer)
{
	int nRet = -1;

	do 
	{
		RtmpMessage msg;
		nRet = m_pRtmpChunk->nParse(buffer, msg);
		if (nRet >= 0)
		{
			if (msg.bIsCompleted())
			{
				if (!bHandleMessage(msg))
				{
					std::cout << "hanlde rtmp message fail" << std::endl;
					return false;
				}
			}
			if (nRet == 0)
			{
				break;
			}
		}
		else
		{
			return false;
		}
	} while (buffer.nReadableBytes() > 0);

	return true;
}

bool CRtmpConnection::bHandleMessage(RtmpMessage& rtmpMsg)
{
	bool bRet = true;

	switch (rtmpMsg.nTypeId)
	{
	case RTMP_VIDEO:
		bRet = bHandleVideo(rtmpMsg);
		break;

	case  RTMP_AUDIO:
		bRet = bHandleAudio(rtmpMsg);
		break;

	case RTMP_INVOKE:
		bRet = bHandleInvoke(rtmpMsg);
		break;

	case RTMP_NOTIFY:
		bRet = bHandleNotify(rtmpMsg);
		break;

	case RTMP_SET_CHUNK_SIZE:
		m_pRtmpChunk->SetInChunkSize(ReadUint32BE(rtmpMsg.pPlayload.get()));
		break;

	default:
		break;
	}

	return bRet;
}

bool CRtmpConnection::bHandleInvoke(RtmpMessage& rtmpMsg)
{
	return false;
}

bool CRtmpConnection::bHandleNotify(RtmpMessage& rtmpMsg)
{
	return false;
}

bool CRtmpConnection::bHandleAudio(RtmpMessage& rtmpMsg)
{
	return false;
}

bool CRtmpConnection::bHandleVideo(RtmpMessage& rtmpMsg)
{
	return false;
}

bool CRtmpConnection::bHandleConnection()
{
	return false;
}

bool CRtmpConnection::bHandleCreateStream()
{
	return false;
}

bool CRtmpConnection::bHandlePublish()
{
	return false;
}

bool CRtmpConnection::bHandlePlay()
{
	return false;
}

bool CRtmpConnection::bHandleDeleteStream()
{
	return false;
}

void CRtmpConnection::SetPeerBandWidth()
{
	std::shared_ptr<char> pData(new char[5], std::default_delete<char[]>());

	WriteUint32BE(pData.get(), m_nPeerWidth);
	pData.get()[4] = 2; //0：客户端必须按带宽处理，超出的话丢弃；1：软限制，可以按带宽处理允许超出
						//2：动态限制，表示客户端可以按照网络限制动态调整
	RtmpMessage msg;
	msg.nTypeId = RTMP_BANDWIDTH_SIZE;
	msg.pPlayload = pData;
	msg.nLength = 5;
	this->SendRtmpChunks(RTMP_CHUNK_CONTROL_ID, msg);
}

void CRtmpConnection::SendAcknowlegement()
{
	std::shared_ptr<char> pData(new char[4], std::default_delete<char[]>());
	WriteUint32BE(pData.get(), m_nAcknowledgementSize);

	RtmpMessage msg;
	msg.nTypeId = RTMP_ACK_SIZE;
	msg.pPlayload = pData;
	msg.nLength = 4;
	this->SendRtmpChunks(RTMP_CHUNK_CONTROL_ID, msg);
}

void CRtmpConnection::SetChunkSize()
{
	m_pRtmpChunk->SetOutChunkSize(m_nMaxChunkSize);
	std::shared_ptr<char> pData(new char[4], std::default_delete<char[]>());
	WriteUint32BE(pData.get(), m_nMaxChunkSize);

	RtmpMessage msg;
	msg.nTypeId = RTMP_SET_CHUNK_SIZE;
	msg.pPlayload = pData;
	msg.nLength = 4;
	this->SendRtmpChunks(RTMP_CHUNK_CONTROL_ID, msg);
}

bool CRtmpConnection::bSendInvokeMessage(uint32_t nCsId, std::shared_ptr<char> pPayload, uint32_t nPayloadSize)
{
	return false;
}

bool CRtmpConnection::bSendNotifyMessage(uint32_t nCsId, std::shared_ptr<char> pPayload, uint32_t nPayloadSize)
{
	return false;
}

void CRtmpConnection::SendRtmpChunks(uint32_t nCsId, RtmpMessage& rtmpMsg)
{
	//创建块
	uint32_t nCapacity = rtmpMsg.nLength + rtmpMsg.nLength / m_nMaxChunkSize * 5 + 1024;
	std::shared_ptr<char> pBuffer(new char[nCapacity], std::default_delete<char[]>());
	int nSize = m_pRtmpChunk->nCreateChunk(nCsId, rtmpMsg, pBuffer.get(), nCapacity);

	if (nSize > 0)
	{
		this->Send(pBuffer.get(), nSize);
	}
}

bool CRtmpConnection::bSendMetaData(mapAmfObjects metaData)
{
	return false;
}

bool CRtmpConnection::bSendMediaData(uint8_t nType, uint64_t nTimeStamp, std::shared_ptr<char> pPlayload, uint32_t nPlayloadSize)
{
	return false;
}

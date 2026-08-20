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
	bool bRet = true;
	m_amfDecoder.Reset();//解析消息

	int nBytesUsed = m_amfDecoder.nDecode(rtmpMsg.pPlayload.get(), rtmpMsg.nLength, 1);
	if (nBytesUsed < 0)
	{
		std::cout << "handle invoke rtmp decode fail" << std::endl;
		return false;
	}

	std::string strMethod = m_amfDecoder.strGetString();
	//需要判断流是否创建
	if (rtmpMsg.nStreamId == 0)
	{
		//如果都没有流，这一段msg无用直接解析完，然后去创建或连接
		nBytesUsed += m_amfDecoder.nDecode(rtmpMsg.pPlayload.get() + nBytesUsed, rtmpMsg.nLength - nBytesUsed);
		//处理连接或者创建流
		{
			if (strMethod == "connect")
			{
				bRet = bHandleConnection();
			}
			else if (strMethod == "createStream")
			{
				bRet = bHandleCreateStream();
			}
		}
	}
	else if (rtmpMsg.nStreamId == m_nStreamID)
	{
		nBytesUsed += m_amfDecoder.nDecode(rtmpMsg.pPlayload.get() + nBytesUsed, rtmpMsg.nLength - nBytesUsed, 3);
		m_strStreamName = m_amfDecoder.strGetString();
		m_strStreamPath = "/" + m_strAppName + "/" + m_strStreamName;

		if (rtmpMsg.nLength > (uint32_t)nBytesUsed)//说明数据没解析完成
		{
			nBytesUsed += m_amfDecoder.nDecode(rtmpMsg.pPlayload.get() + nBytesUsed, rtmpMsg.nLength - nBytesUsed);
		}

		if (strMethod == "publish")
		{
			bRet = bHandlePublish();
		}
		else if (strMethod == "play")
		{
			bRet = bHandlePlay();
		}
		else if (strMethod == "DeleteStream")
		{
			bRet = bHandleDeleteStream();
		}
	}

	return bRet;
}

bool CRtmpConnection::bHandleNotify(RtmpMessage& rtmpMsg)
{
	m_amfDecoder.Reset();

	//解析消息名称
	int nBytesUsed = m_amfDecoder.nDecode(rtmpMsg.pPlayload.get(), rtmpMsg.nLength, 1);
	std::string strMethod = m_amfDecoder.strGetString();
	if (strMethod == "@setDataFrame")
	{
		m_amfDecoder.Reset();
		nBytesUsed = m_amfDecoder.nDecode(rtmpMsg.pPlayload.get() + nBytesUsed, rtmpMsg.nLength - nBytesUsed, 1);
		if (nBytesUsed < 0)
		{
			std::cout << "handle notify rtmp decode fail" << std::endl;
			return false;
		}
		//是不是元数据
		if (m_amfDecoder.strGetString() == "onMetaData")
		{
			m_amfDecoder.nDecode(rtmpMsg.pPlayload.get() + nBytesUsed, rtmpMsg.nLength - nBytesUsed);
			m_amfobjsMetaData = m_amfDecoder.GetObjects();
		}

		//设置元数据，获取session
		auto server = m_pRtmpServer.lock();
		if (!server)
		{
			std::cout << "bHandleNotify server is not exist" << std::endl;
			return false;
		}

		auto session = m_pRtmpSession.lock();
		if (!session)
		{
			std::cout << "bHandleNotify session is not exist" << std::endl;
			return false;
		}
		session->SendMetaData(m_amfobjsMetaData);
	}
	return true;
}

bool CRtmpConnection::bHandleAudio(RtmpMessage& rtmpMsg)
{
	uint8_t nType = RTMP_AUDIO;
	uint8_t* pPayload = (uint8_t*)rtmpMsg.pPlayload.get();
	uint8_t nSoundFormat = (pPayload[0] >> 4) & 0x0f;//音频格式

	auto server = m_pRtmpServer.lock();
	if (!server)
	{
		std::cout << "bHandleAudio server is not exist" << std::endl;
		return false;
	}

	auto session = m_pRtmpSession.lock();
	if (!session)
	{
		std::cout << "bHandleAudio session is not exist" << std::endl;
		return false;
	}

	//说明编码器为aac，第二字节为0说明音频数据为aac序列头
	if (nSoundFormat == RTMP_CODEC_ID_AAC && pPayload[1] == 0)
	{
		m_nAacSequenceHeaderSize = rtmpMsg.nLength;
		m_pAacSequenceHeader.reset(new char[rtmpMsg.nLength], std::default_delete<char[]>());
		memcpy(m_pAacSequenceHeader.get(), rtmpMsg.pPlayload.get(), m_nAacSequenceHeaderSize);

		//需要使用session设置AAC序列头
		session->SetAacSequenceHeader(m_pAacSequenceHeader, m_nAacSequenceHeaderSize);
		nType = RTMP_AAC_SEQUENCE_HEADER;
	}

	//通过session发送音频数据
	session->SendMediaData(nType, rtmpMsg.nTimeStamp, rtmpMsg.pPlayload, rtmpMsg.nLength);

	return true;
}

bool CRtmpConnection::bHandleVideo(RtmpMessage& rtmpMsg)
{
	uint8_t nType = RTMP_VIDEO;
	uint8_t* pPayload = (uint8_t*)rtmpMsg.pPlayload.get();
	uint8_t nFrameFormat = (pPayload[0] >> 4) & 0x0f;//视频帧格式
	uint8_t nCodecId = pPayload[0]& 0x0f;//编码id

	auto server = m_pRtmpServer.lock();
	if (!server)
	{
		std::cout << "bHandleVideo server is not exist" << std::endl;
		return false;
	}

	auto session = m_pRtmpSession.lock();
	if (!session)
	{
		std::cout << "bHandleVideo session is not exist" << std::endl;
		return false;
	}

	//更新h264序列头
	if (nFrameFormat == 1 && nCodecId == RTMP_CODEC_ID_H264 && pPayload[1] == 0)
	{
		m_nAvcSequenceHeaderSize = rtmpMsg.nLength;
		m_pAvcSequenceHeader.reset(new char[rtmpMsg.nLength], std::default_delete<char[]>());
		memcpy(m_pAvcSequenceHeader.get(), rtmpMsg.pPlayload.get(), m_nAvcSequenceHeaderSize);

		//需要使用session设置AVC序列头
		session->SetAvcSequenceHeader(m_pAvcSequenceHeader, m_nAvcSequenceHeaderSize);
		nType = RTMP_AVC_SEQUENCE_HEADER;
	}

	//通过session发送视频数据
	session->SendMediaData(nType, rtmpMsg.nTimeStamp, rtmpMsg.pPlayload, rtmpMsg.nLength);

	return true;
}

bool CRtmpConnection::bHandleConnection()
{
	//是否存在app应用
	if (!m_amfDecoder.bHasObject("app"))
	{
		std::cout << "decoder did not decode app" << std::endl;
		return false;
	}

	AmfObject amfObj = m_amfDecoder.GetObject("app");
	m_strAppName = amfObj.strAmfString;//获取应用程序名称
	if (m_strAppName == "")
	{
		std::cout << "app name is null" << std::endl;
		return false;
	}

	SendAcknowlegement();
	SetPeerBandWidth();
	SetChunkSize();

	//应答
	mapAmfObjects ackAmfObj;
	m_amfEncoder.Reset();
	//编码结果
	m_amfEncoder.encodeString("_result", 7);
	m_amfEncoder.encodeNumber(m_amfDecoder.fGetNumber());

	ackAmfObj["fmsVer"] = AmfObject(std::string("FMS/4,5,0,297"));
	ackAmfObj["capabilities"] = AmfObject(255.0);
	ackAmfObj["mode"] = AmfObject(1.0);
	m_amfEncoder.encodeObjects(ackAmfObj);
	//清空对象
	ackAmfObj.clear();
	//添加参数
	ackAmfObj["level"] = AmfObject(std::string("status"));
	ackAmfObj["code"] = AmfObject(std::string("NetConnection.Connect.Success"));
	ackAmfObj["description"] = AmfObject(std::string("Connection succeeded"));
	ackAmfObj["objectEncoding"] = AmfObject(0.0);
	m_amfEncoder.encodeObjects(ackAmfObj);

	if (!bSendInvokeMessage(RTMP_CHUNK_INVOKE_ID, m_amfEncoder.pData(), m_amfEncoder.nSize()))
	{
		std::cout << "bHandleConnection send invoke message fail" << std::endl;
		return false;
	}

	return true;
}

bool CRtmpConnection::bHandleCreateStream()
{
	int nStreamID = m_pRtmpChunk->nGetStreamId();

	mapAmfObjects amfObjs;
	//获取编码结果
	m_amfEncoder.Reset();
	m_amfEncoder.encodeString("_result", 7);
	m_amfEncoder.encodeNumber(m_amfDecoder.fGetNumber());
	//需要填一个空对象
	m_amfEncoder.encodeObjects(amfObjs);
	m_amfEncoder.encodeNumber(nStreamID);

	if (!bSendInvokeMessage(RTMP_CHUNK_INVOKE_ID, m_amfEncoder.pData(), m_amfEncoder.nSize()))
	{
		std::cout << "bHandleCreateStream send invoke message fail" << std::endl;
		return false;
	}
	m_nStreamID = nStreamID;

	return true;
}

bool CRtmpConnection::bHandlePublish()
{
	auto server = m_pRtmpServer.lock();
	if (!server)
	{
		std::cout << "bHandlePublish server is not exist" << std::endl;
		return false;
	}

	mapAmfObjects amfObjs;
	m_amfEncoder.Reset();
	m_amfEncoder.encodeString("onStatus", 8);
	m_amfEncoder.encodeNumber(0);
	m_amfEncoder.encodeObjects(amfObjs);

	bool bIsPublished = false;

	//判断是否已经推流
	if(server->bHasPublisher(m_strStreamPath))
	{
		bIsPublished = true;

		amfObjs["level"] = AmfObject(std::string("error"));
		//说明这个流已经推送
		amfObjs["code"] = AmfObject(std::string("NetStream.Publish.BadName"));
		amfObjs["description"] = AmfObject(std::string("Stream already pulished."));
	}
	//正在推流状态也不能推送
	else if(m_state == START_PUBLISH)
	{
		bIsPublished = true;

		amfObjs["level"] = AmfObject(std::string("error"));
		//说明这个流正在推送
		amfObjs["code"] = AmfObject(std::string("NetStream.Publish.BadConnection"));
		amfObjs["description"] = AmfObject(std::string("Stream is pulishing."));
	}
	else
	{
		amfObjs["level"] = AmfObject(std::string("status"));
		amfObjs["code"] = AmfObject(std::string("NetStream.Publish.Start"));
		amfObjs["description"] = AmfObject(std::string("Start pulishing."));

		//添加session
		server->AddSession(m_strStreamPath);
		m_pRtmpSession = server->pGetSession(m_strStreamPath);

		server->NotifyEvent("publish.start", m_strStreamPath);
	}

	m_amfEncoder.encodeObjects(amfObjs);

	if (!bSendInvokeMessage(RTMP_CHUNK_INVOKE_ID, m_amfEncoder.pData(), m_amfEncoder.nSize()))
	{
		std::cout << "bHandlePublish send invoke message fail" << std::endl;
		return false;
	}

	if (bIsPublished)
	{
		std::cout << "stream has published or is publishing" << std::endl;
		return false;
	}
	else
	{
		m_state = START_PUBLISH;
		m_bIsPublishing = true;
	}

	auto session = m_pRtmpSession.lock();
	if (!session)
	{
		std::cout << "bHandlePublish session is not exist" << std::endl;
		return false;
	}
	session->AddSink(std::dynamic_pointer_cast<CRtmpSink>(shared_from_this()));

	return true;
}

bool CRtmpConnection::bHandlePlay()
{
	auto server = m_pRtmpServer.lock();
	if (!server)
	{
		std::cout << "bHandlePlay server is not exist" << std::endl;
		return false;
	}

	mapAmfObjects amfObjs;
	m_amfEncoder.Reset();

	//添加应答
	m_amfEncoder.encodeString("onStatus", 8);
	m_amfEncoder.encodeNumber(0);
	amfObjs["level"] = AmfObject(std::string("status"));
	amfObjs["code"] = AmfObject(std::string("NetStream.Play.Reset"));
	amfObjs["description"] = AmfObject(std::string("Resetting and playing stream."));

	m_amfEncoder.encodeObjects(amfObjs);
	if (!bSendInvokeMessage(RTMP_CHUNK_INVOKE_ID, m_amfEncoder.pData(), m_amfEncoder.nSize()))
	{
		std::cout << "bHandlePlay send ack invoke message fail" << std::endl;
		return false;
	}

	//添加play命令
	amfObjs.clear();
	m_amfEncoder.Reset();
	m_amfEncoder.encodeString("onStatus", 8);
	m_amfEncoder.encodeNumber(0);
	amfObjs["level"] = AmfObject(std::string("status"));
	amfObjs["code"] = AmfObject(std::string("NetStream.Play.Start"));
	amfObjs["description"] = AmfObject(std::string("Start playing"));
	m_amfEncoder.encodeObjects(amfObjs);
	if (!bSendInvokeMessage(RTMP_CHUNK_INVOKE_ID, m_amfEncoder.pData(), m_amfEncoder.nSize()))
	{
		std::cout << "bHandlePlay send play invoke message fail" << std::endl;
		return false;
	}

	//通知客户端权限
	m_amfEncoder.Reset();
	m_amfEncoder.encodeString("|RtmpSampleAccess", 17);
	m_amfEncoder.encodeBoolean(true);//允许读
	m_amfEncoder.encodeBoolean(true);//允许写
	if (!bSendNotifyMessage(RTMP_CHUNK_DATA_ID, m_amfEncoder.pData(), m_amfEncoder.nSize()))
	{
		std::cout << "bHandlePlay send root notify message fail" << std::endl;
		return false;
	}

	m_state = START_PLAY;

	m_pRtmpSession = server->pGetSession(m_strStreamPath);
	auto session = m_pRtmpSession.lock();
	if (!session)
	{
		std::cout << "bHandlePublish session is not exist" << std::endl;
		return false;
	}
	session->AddSink(std::dynamic_pointer_cast<CRtmpSink>(shared_from_this()));

	server->NotifyEvent("play.start", m_strStreamPath);

	return true;
}

bool CRtmpConnection::bHandleDeleteStream()
{
	auto server = m_pRtmpServer.lock();
	if (!server)
	{
		std::cout << "bHandlePlay server is not exist" << std::endl;
		return false;
	}

	if (m_strStreamPath != "")
	{
		//session移除会话
		auto session = m_pRtmpSession.lock();
		if (!session)
		{
			std::cout << "bHandleDeleteStream session is not exist" << std::endl;
			return false;
		}

		auto conn = std::dynamic_pointer_cast<CRtmpSink>(shared_from_this());
		getReactor()->AddTimer([session, conn] {
			session->RemoveSink(conn);
			return false;
			}, 1);

		if (m_bIsPublishing)
		{
			server->NotifyEvent("publish.stop", m_strStreamPath);
		}
		else
		{
			server->NotifyEvent("play.stop", m_strStreamPath);
		}

		m_bIsPlaying = false;
		m_bIsPublishing = false;
		m_bHasKeyFrame = false;
		m_pRtmpChunk->Clear();
	}

	return true;
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
	if (this->bIsClosed())
	{
		std::cout << "connection already closed, send invoke message fail" << std::endl;
		return false;
	}

	RtmpMessage rtmpMsg;
	rtmpMsg.nTypeId = RTMP_INVOKE;
	rtmpMsg.nTimeStamp = 0;
	rtmpMsg.nStreamId = m_nStreamID;
	rtmpMsg.pPlayload = pPayload;
	rtmpMsg.nLength = nPayloadSize;

	SendRtmpChunks(nCsId, rtmpMsg);

	return true;
}

bool CRtmpConnection::bSendNotifyMessage(uint32_t nCsId, std::shared_ptr<char> pPayload, uint32_t nPayloadSize)
{
	if (this->bIsClosed())
	{
		std::cout << "connection already closed, send invoke message fail" << std::endl;
		return false;
	}

	RtmpMessage rtmpMsg;
	rtmpMsg.nTypeId = RTMP_NOTIFY;
	rtmpMsg.nTimeStamp = 0;
	rtmpMsg.nStreamId = m_nStreamID;
	rtmpMsg.pPlayload = pPayload;
	rtmpMsg.nLength = nPayloadSize;

	SendRtmpChunks(nCsId, rtmpMsg);

	return true;
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
	if (this->bIsClosed())
	{
		std::cout << "connection already closed, send metadata fail" << std::endl;
		return false;
	}

	if (metaData.size() == 0)
	{
		std::cout << "metadata size is 0" << std::endl;
		return false;
	}

	m_amfEncoder.Reset();
	m_amfEncoder.encodeString("onMetaData", 10);
	m_amfEncoder.encodeECMA(metaData);

	if (!bSendNotifyMessage(RTMP_CHUNK_DATA_ID, m_amfEncoder.pData(), m_amfEncoder.nSize()))
	{
		std::cout << "bSendMetaData send metadata notify message fail" << std::endl;
		return false;
	}

	return true;
}

bool CRtmpConnection::bSendMediaData(uint8_t nType, uint64_t nTimeStamp, std::shared_ptr<char> pPlayload, uint32_t nPlayloadSize)
{
	if (this->bIsClosed())
	{
		std::cout << "connection already closed, send mediadata fail" << std::endl;
		return false;
	}

	if (nPlayloadSize == 0)
	{
		std::cout << "bSendMediaData payloadsize is 0" << std::endl;
		return false;
	}

	m_bIsPlaying = true;

	if (nType == RTMP_AVC_SEQUENCE_HEADER)
	{
		m_pAvcSequenceHeader = pPlayload;
		m_nAvcSequenceHeaderSize = nPlayloadSize;
	}
	else if (nType == RTMP_AAC_SEQUENCE_HEADER)
	{
		m_pAacSequenceHeader = pPlayload;
		m_nAacSequenceHeaderSize = nPlayloadSize;
	}

	if (!m_bHasKeyFrame && m_nAvcSequenceHeaderSize > 0
		&& (nType != RTMP_AVC_SEQUENCE_HEADER)
		&& (nType != RTMP_AAC_SEQUENCE_HEADER))//说明数据包既不是序列头，还没有收到关键帧
	{
		//判断是否为关键帧
		if (bIsKeyFrame(pPlayload, nPlayloadSize))
		{
			m_bHasKeyFrame = true;
		}
		else
		{
			//std::cout << "data is not header, and not has key frame" << std::endl;
			return true;
		}
	}

	//收到关键帧，发送message
	RtmpMessage rtmpMsg;
	rtmpMsg.internal_nTimeStamp = nTimeStamp;
	rtmpMsg.nStreamId = m_nStreamID;
	rtmpMsg.pPlayload = pPlayload;
	rtmpMsg.nLength = nPlayloadSize;

	if (nType == RTMP_VIDEO || nType == RTMP_AVC_SEQUENCE_HEADER)
	{
		rtmpMsg.nTypeId = RTMP_VIDEO;
		SendRtmpChunks(RTMP_CHUNK_VIDEO_ID, rtmpMsg);
	}
	else if (nType == RTMP_AUDIO || nType == RTMP_AAC_SEQUENCE_HEADER)
	{
		rtmpMsg.nTypeId = RTMP_AUDIO;
		SendRtmpChunks(RTMP_CHUNK_AUDIO_ID, rtmpMsg);
	}

	return true;
}

bool CRtmpConnection::bIsKeyFrame(std::shared_ptr<char> pData, uint32_t nSize)
{
	uint8_t nFrameType = (pData.get()[0] >> 4) & 0x0f;
	uint8_t nCodecId = pData.get()[0] & 0x0f;

	return (nFrameType == 1 && nCodecId == RTMP_CODEC_ID_H264);
}

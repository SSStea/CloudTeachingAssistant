#pragma once
#include "TcpConnection.h"
#include "Amf.h"
#include "RtmpSink.h"
#include "RtmpChunk.h"
#include "RtmpHandshake.h"
#include "Rtmp.h"
#include "RtmpServer.h"
#include "RtmpSession.h"

class CRtmpServer;
class CRtmpSession;
class CRtmpConnection : public CTcpConnection, public CRtmpSink
{
public:
	enum ConnectionState
	{
		HANDSHAKE,
		START_CONNECT,
		START_CREATE_STREAM,
		START_DELETE_STREAM,
		START_PLAY,
		START_PUBLISH,
	};

	CRtmpConnection(std::shared_ptr<CRtmpServer> rtmpServer, CReactorBase* reactor, int nSocket);
	virtual ~CRtmpConnection();

	virtual bool bIsPlayer() override { return m_state == START_PLAY; }
	virtual bool bIsPublisher() override { return m_state == START_PUBLISH; }
	virtual bool bIsPlaying() override { return m_bIsPlaying; }
	virtual bool bIsPublishing() override { return m_bIsPublishing; }

	virtual uint32_t nGetId() override { return this->nGetSocket(); }

private:
	CRtmpConnection(CReactorBase* reactor, int nSocket, CRtmp* rtmp);
	bool bOnRead(CBufferReader& buffer);//读数据
	void OnClose();

	bool bHandleChunk(CBufferReader& buffer);//处理块
	bool bHandleMessage(RtmpMessage& rtmpMsg);//处理消息
	bool bHandleInvoke(RtmpMessage& rtmpMsg);//处理调用
	bool bHandleNotify(RtmpMessage& rtmpMsg);//处理通知
	bool bHandleAudio(RtmpMessage& rtmpMsg);//处理音频
	bool bHandleVideo(RtmpMessage& rtmpMsg);//处理视频
	bool bHandleConnection();//处理rtmp连接
	bool bHandleCreateStream();//处理创建流
	bool bHandlePublish();//处理推流
	bool bHandlePlay();// 处理拉流
	bool bHandleDeleteStream();//处理删除流

	void SetPeerBandWidth();//设置带宽
	void SendAcknowlegement();//发送接收大小
	void SetChunkSize();//设置块大小

	//发送启动消息
	bool bSendInvokeMessage(uint32_t nCsId, std::shared_ptr<char> pPayload, uint32_t nPayloadSize);
	//发送通知消息
	bool bSendNotifyMessage(uint32_t nCsId, std::shared_ptr<char> pPayload, uint32_t nPayloadSize);
	void SendRtmpChunks(uint32_t nCsId, RtmpMessage& rtmpMsg);//发送块数据

	virtual bool bSendMetaData(mapAmfObjects metaData) override;
	virtual bool bSendMediaData(uint8_t nType, uint64_t nTimeStamp, std::shared_ptr<char> pPlayload, uint32_t nPlayloadSize)override;

private:
	ConnectionState m_state;

	std::shared_ptr<CRtmpHandshake> m_pHandShake;
	std::shared_ptr<CRtmpChunk> m_pRtmpChunk;

	uint32_t m_nPeerWidth;
	uint32_t m_nAcknowledgementSize;
	uint32_t m_nMaxChunkSize;
	uint32_t m_nStreamID;

	mapAmfObjects m_amfobjMetaData;
	CAmfDecoder m_amfDecoder;
	CAmfEncoder m_amfEncoder;

	bool m_bIsPlaying = false;
	bool m_bIsPublishing = false;

	//元数据
	std::shared_ptr<char> m_pAvcSequenceHeader;
	std::shared_ptr<char> m_pAacSequenceHeader;
	uint32_t m_nAvcSequenceHeaderSize = 0;
	uint32_t m_nAacSequenceHeaderSize = 0;

	std::string m_strAppName;
	std::string m_strStreamName;
	std::string m_strStreamPath;

	std::weak_ptr<CRtmpServer> m_pRtmpServer;
	std::weak_ptr<CRtmpSession> m_pRtmpSession;
};


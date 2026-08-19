#pragma once

#include "Amf.h"
#include "Rtmp.h"
#include "RtmpChunk.h"
#include "RtmpHandshake.h"
#include "TcpConnection.h"

class CRtmpPublisher;

class CRtmpConnection : public CTcpConnection
{
public:
	enum ConnectionState
	{
		HANDSHAKE,
		START_CONNECT,
		START_CREATE_STREAM,
		START_DELETE_STREAM,
		START_PUBLISH,
	};

	CRtmpConnection(std::shared_ptr<CRtmpPublisher> pPublisher, CReactorBase* pReactor, int nSocket);
	virtual ~CRtmpConnection();

	bool bHandshake();
	bool bSendVideoData(uint64_t nTimeStamp, std::shared_ptr<char> pPlayload, uint32_t nPlayloadSize);
	bool bSendAudioData(uint64_t nTimeStamp, std::shared_ptr<char> pPlayload, uint32_t nPlayloadSize);

private:
	CRtmpConnection(CReactorBase* pReactor, int nSocket, CRtmp* pRtmp);

	bool bOnRead(CBufferReader& buffer);
	void OnClose();

	bool bHandleChunk(CBufferReader& buffer);
	bool bHandleMessage(RtmpMessage& rtmpMsg);
	bool bHandleInvoke(RtmpMessage& rtmpMsg);
	bool bConnect();
	bool bCreateStream();
	bool bPublish();
	bool bDeleteStream();
	bool bHandleResult(RtmpMessage& rtmpMsg);
	bool bHandleOnStatus(RtmpMessage& rtmpMsg);
	bool bSendInvokeMessage(uint32_t nCsId, std::shared_ptr<char> pPlayload, uint32_t nPlayloadSize);
	bool bSendNotifyMessage(uint32_t nCsId, std::shared_ptr<char> pPlayload, uint32_t nPlayloadSize);
	bool bIsKeyFrame(std::shared_ptr<char> pData, uint32_t nSize);

	void SetChunkSize();
	void SendRtmpChunks(uint32_t nCsId, RtmpMessage& rtmpMsg);

private:
	ConnectionState m_state = HANDSHAKE;

	std::string m_strAppName;
	std::string m_strStreamName;
	std::string m_strStreamPath;

	uint32_t m_nNumber = 0;
	uint32_t m_nStreamId = 0;
	uint32_t m_nMaxChunkSize = 128;
	uint32_t m_nAvcSequenceHeaderSize = 0;
	uint32_t m_nAacSequenceHeaderSize = 0;

	bool m_bIsPublishing = false;
	bool m_bHasKeyFrame = false;

	std::weak_ptr<CRtmpPublisher> m_pRtmpPublisher;
	std::shared_ptr<CRtmpHandshake> m_pHandshake;
	std::shared_ptr<CRtmpChunk> m_pRtmpChunk;
	CAmfDecoder m_amfDecoder;
	CAmfEncoder m_amfEncoder;

	std::shared_ptr<char> m_pAvcSequenceHeader;
	std::shared_ptr<char> m_pAacSequenceHeader;
};

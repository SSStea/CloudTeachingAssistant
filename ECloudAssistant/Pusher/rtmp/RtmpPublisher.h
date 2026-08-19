#pragma once

#include "EventLoop.h"
#include "Rtmp.h"
#include "RtmpConnection.h"
#include "TimeStamp.h"

class CRtmpPublisher : public CRtmp, public std::enable_shared_from_this<CRtmpPublisher>
{
public:
    static std::shared_ptr<CRtmpPublisher> pCreate(CEventLoop* pEventLoop);//创建 CRtmpPublisher 智能指针
	virtual ~CRtmpPublisher();

    int nSetMediaInfo(RtmpMediaInfo mediaInfo);//保存媒体信息并生成 AVC/AAC Sequence Header
    int nOpenUrl(std::string strUrl, int nMsec);//解析 URL、建立 TCP、创建 RTMP 连接并开始握手
    int nPushVideoFrame(uint8_t* pData, uint32_t nSize);//封装并发送 H.264 视频帧
    int nPushAudioFrame(uint8_t* pData, uint32_t nSize);//封装并发送 AAC 音频帧
    void Close();//断开连接并重置关键帧状态
    bool bIsConnected() const;//判断 RTMP TCP 连接是否存在且未关闭

private:
	CRtmpPublisher(CEventLoop* pEventLoop);
    bool bIsKeyFrame(uint8_t* pData, uint32_t nSize) const;//根据 H.264 NAL 类型判断关键帧

private:
	CEventLoop* m_pEventLoop = nullptr;
	std::shared_ptr<CRtmpConnection> m_pRtmpConnection;
    RtmpMediaInfo m_mediaInfo; //媒体信息
	bool m_bHasKeyFrame = false;
	CTimeStamp m_timeStamp;
	std::shared_ptr<char> m_pAvcSequenceHeader;
	std::shared_ptr<char> m_pAacSequenceHeader;
	uint32_t m_nAvcSequenceHeaderSize = 0;
	uint32_t m_nAacSequenceHeaderSize = 0;
};

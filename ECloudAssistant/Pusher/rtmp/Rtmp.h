#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <string.h>
#include <QtGlobal>
#include <memory>

static const int RTMP_VERSION = 0X3;			//版本
static const int RTMP_SET_CHUNK_SIZE = 0x1;		//块大小
static const int RTMP_ABORT_MESSAGE = 0x2;		//终止消息
static const int RTMP_ACK = 0x3;				//确认消息
static const int RTMP_ACK_SIZE = 0x5;			//确认大小
static const int RTMP_BANDWIDTH_SIZE = 0x6;		//设置客户端带宽
static const int RTMP_AUDIO = 0x08;				//音频消息
static const int RTMP_VIDEO = 0x09;				//视频消息
static const int RTMP_NOTIFY = 0x12;			//消息通知
static const int RTMP_INVOKE = 0x14;			//调用消息

//chunk类型
static const int RTMP_CHUNK_TYPE_0 = 0;
static const int RTMP_CHUNK_TYPE_1 = 1;
static const int RTMP_CHUNK_TYPE_2 = 2;
static const int RTMP_CHUNK_TYPE_3 = 3;

//RTMP消息ID
static const int RTMP_CHUNK_CONTROL_ID = 2;		//控制
static const int RTMP_CHUNK_INVOKE_ID = 3;		//调用
static const int RTMP_CHUNK_AUDIO_ID = 4;		//音频
static const int RTMP_CHUNK_VIDEO_ID = 5;		//视频
static const int RTMP_CHUNK_DATA_ID = 6;		//数据块

//编码类型
static const int RTMP_CODEC_ID_H264 = 7;
static const int RTMP_CODEC_ID_AAC = 10;

//元数据类型ID
static const int RTMP_AVC_SEQUENCE_HEADER = 0x18; //视频
static const int RTMP_AAC_SEQUENCE_HEADER = 0x19; //音频
//rtmp协议文档

struct RtmpMediaInfo
{
    quint8 m_nVideoCodecId = RTMP_CODEC_ID_H264;//视频编码类型，默认 H.264
	quint8 m_nVideoFrameRate = 0;
	quint32 m_nVideoWidth = 0;
	quint32 m_nVideoHeight = 0;
    std::shared_ptr<quint8> m_pSps; //H.264 SPS 数据及大小
    std::shared_ptr<quint8> m_pPps; //H.264 PPS 数据及大小
    std::shared_ptr<quint8> m_pSei; //H.264 SEI 数据及大小
	quint32 m_nSpsSize = 0;
	quint32 m_nPpsSize = 0;
	quint32 m_nSeiSize = 0;

    quint8 m_nAudioCodecId = RTMP_CODEC_ID_AAC; //音频编码类型，默认 AAC
    quint32 m_nAudioChannels = 0; //声道数
    quint32 m_nAudioSampleRate = 0; //采样率
    quint32 m_nAudioFrameLength = 0; //每帧采样数
    std::shared_ptr<quint8> m_pAudioSpecificConfig;//AAC 配置信息
    quint32 m_nAudioSpecificConfigSize = 0;//AAC 配置信息大小
};


class CRtmp
{
public:
	virtual ~CRtmp() {};

	void SetChunkSize(uint32_t nSize)
	{
		if (nSize > 0 && nSize <= 60000) {
			m_nMaxChunkSize = nSize;
		}
	}

	void SetPeerBandwidth(uint32_t nSize)
	{
		m_nPeerBandwidth = nSize;
	}

	uint32_t nGetChunkSize() const
	{
		return m_nMaxChunkSize;
	}

	uint32_t nGetAcknowledgementSize() const
	{
		return m_nAcknowledgementSize;
	}

	uint32_t nGetPeerBandwidth() const
	{
		return m_nPeerBandwidth;
	}

	virtual int nParseRtmpUrl(std::string strUrl)
	{
		char chIP[100] = { 0 };
		char chStreamPath[500] = { 0 };
		char chApp[100] = { 0 };
		char chStreamName[400] = { 0 };
		uint16_t nPort = 0;

		if (strstr(strUrl.c_str(), "rtmp://") == nullptr) {
			return -1;
		}

#if defined(__linux) || defined(__linux__)
		if (sscanf(strUrl.c_str() + 7, "%[^:]:%hu/%s", chIP, &nPort, chStreamPath) == 3)
#elif defined(WIN32) || defined(_WIN32)
        if (sscanf_s(strUrl.c_str() + 7, "%[^:]:%hu/%s", chIP, 100, &nPort, chStreamPath, 500) == 3)
#endif
		{
			m_nPort = nPort;
		}
#if defined(__linux) || defined(__linux__)
		else if (sscanf(strUrl.c_str() + 7, "%[^/]/%s", chIP, chStreamPath) == 2)
#elif defined(WIN32) || defined(_WIN32)
        else if (sscanf_s(strUrl.c_str() + 7, "%[^/]/%s", chIP, 100, chStreamPath, 500) == 2)
#endif
		{
			m_nPort = 1935;
		}
		else {
			return -1;
		}

		m_strIP = chIP;
		m_strStreamPath += "/";
		m_strStreamPath += chStreamPath;

#if defined(__linux) || defined(__linux__)
		if (sscanf(m_strStreamPath.c_str(), "/%[^/]/%s", chApp, chStreamName) != 2)
#elif defined(WIN32) || defined(_WIN32)
        if (sscanf_s(m_strStreamPath.c_str(), "/%[^/]/%s", chApp, 100, chStreamName, 400) != 2)
#endif
		{
			return -1;
		}

		m_strApp = chApp;
		m_strStreamName = chStreamName;
		return 0;
	}

	std::string strGetStreamPath() const
	{
		return m_strStreamPath;
	}

	std::string strGetApp() const
	{
		return m_strApp;
	}

	std::string strGetStreamName() const
	{
		return m_strStreamName;
	}

	uint16_t m_nPort = 1935;
	std::string m_strIP;
	std::string m_strApp;
	std::string m_strStreamName;
	std::string m_strStreamPath;

	uint32_t m_nPeerBandwidth = 5000000;
	uint32_t m_nAcknowledgementSize = 5000000;
	uint32_t m_nMaxChunkSize = 128;
};

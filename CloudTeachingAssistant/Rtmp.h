#pragma once
#include <cstdint>
#include <cstring>
#include <string>

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
		if (sscanf_s(url.c_str() + 7, "%[^:]:%hu/%s", ip, 100, &port, streamPath, 500) == 3)
#endif
		{
			m_nPort = nPort;
		}
#if defined(__linux) || defined(__linux__)
		else if (sscanf(strUrl.c_str() + 7, "%[^/]/%s", chIP, chStreamPath) == 2)
#elif defined(WIN32) || defined(_WIN32)
		else if (sscanf_s(url.c_str() + 7, "%[^/]/%s", ip, 100, streamPath, 500) == 2)
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
		if (sscanf_s(stream_path_.c_str(), "/%[^/]/%s", app, 100, streamName, 400) != 2)
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

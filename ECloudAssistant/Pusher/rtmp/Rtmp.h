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

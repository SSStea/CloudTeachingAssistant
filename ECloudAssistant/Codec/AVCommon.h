#ifndef AVCOMMON_H
#define AVCOMMON_H

#include <QtGlobal>
#include <QDebug>
#include <memory>
#include <mutex>
#include "AVQueue.h"
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include "libavutil/error.h"
}

using AVPacketPtr = std::shared_ptr<AVPacket>;
using AVFramePtr  = std::shared_ptr<AVFrame>;

typedef struct VIDEOCONFIG
{
    quint32 nWidth;
    quint32 nHeight;
    quint32 nBitRate;
    quint32 nFrameRate;
    quint32 nGop;
    AVPixelFormat format;
}VideoConfig;

typedef struct AUDIOCONFIG
{
    quint32 nChannels;
    quint32 nSamplerate;
    quint32 nBitRate;
    AVSampleFormat format;
}AudioConfig;

struct AVConfig
{
    VideoConfig video;
    AudioConfig audio;
};

struct AVContext
{
public:
    //音频相关参数//
    int32_t nAudioSampleRate;
    int32_t nAudioChannelsLayout;
    AVRational audioSrcTimebase;
    AVRational audioDstTimebase;
    AVSampleFormat audioFmt;
    double dAudioDuration;
    CAVQueue<AVFramePtr> qAudioQueue;
    //视频相关参数
    int32_t nVideoWidth;
    int32_t nVideoHeight;
    AVRational videoSrcTimebase;
    AVRational videodstTimebase;
    AVPixelFormat videoFmt;
    double dVideoDuration;
    CAVQueue<AVFramePtr> qVideoQueue;

    int nAvMediatype = 0;
};

class CEncodBase
{
public:
    CEncodBase():m_bIsInitialzed(false),m_pCodec(nullptr),m_pCodecContext(nullptr){m_config = {};}
    virtual ~CEncodBase(){if(m_pCodecContext)avcodec_free_context(&m_pCodecContext);}
    CEncodBase(const CEncodBase&) = delete;
    CEncodBase& operator=(const CEncodBase&) = delete;
public:
    virtual bool bOpen(AVConfig& config) = 0;
    virtual void Close() = 0;
    AVCodecContext* pGetAVCodecContext() const
    {return m_pCodecContext;}
protected:
    bool m_bIsInitialzed = false;
    AVConfig m_config;
    AVCodec* m_pCodec;
    AVCodecContext* m_pCodecContext = nullptr;
};

class CDecodBase
{
public:
    CDecodBase():m_bIsInitialzed(false),m_nVideoIndex(-1),m_nAudioIndex(-1),m_pCodec(nullptr),m_pCodecCtx(nullptr){m_config = {};}
    virtual ~CDecodBase(){if(m_pCodecCtx){avcodec_free_context(&m_pCodecCtx);};}
    CDecodBase(const CDecodBase&) = delete;
    CDecodBase& operator=(const CDecodBase&) = delete;
    AVCodecContext* pGetAVCodecContext() const
    {return m_pCodecCtx;}
protected:
    bool m_bIsInitialzed;
    std::mutex m_mutex;
    qint32 m_nVideoIndex;
    qint32 m_nAudioIndex;
    AVConfig m_config;
    AVCodec* m_pCodec;
    AVCodecContext* m_pCodecCtx;
};

#endif // AVCOMMON_H

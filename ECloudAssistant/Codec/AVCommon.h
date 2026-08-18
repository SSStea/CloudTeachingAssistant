#ifndef AVCOMMON_H
#define AVCOMMON_H

#include <QtGlobal>
#include <QDebug>
#include <memory>
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

#endif // AVCOMMON_H

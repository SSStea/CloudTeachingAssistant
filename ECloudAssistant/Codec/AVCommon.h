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

#endif // AVCOMMON_H

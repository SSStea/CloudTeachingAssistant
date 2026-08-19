#ifndef H264ENCODER_H
#define H264ENCODER_H
#include <QtGlobal>
#include <vector>
#include "AVCommon.h"

class CVideoEncoder;
class CH264Encoder
{
public:
    CH264Encoder();
    CH264Encoder(const CH264Encoder&) = delete;
    CH264Encoder& operator=(const CH264Encoder&) = delete;
    ~CH264Encoder();
public:
    bool bOPen(qint32 nWidth,qint32 nHeight,qint32 nFramerate,qint32 nBitRate,qint32 nFormat);
    void Close();
    qint32 nEncode(quint8* pRgbaBuffer,quint32 nWidth,quint32 nHeight,quint32 nSize,std::vector<quint8>& vecOutFrame);
    qint32 nGetSequenceParams(quint8* pOutBuffer, qint32 nOutBufferSize);
private:
    bool bIsKeyFrame(AVPacketPtr pPkt);
private:
    AVConfig m_config;
    std::unique_ptr<CVideoEncoder> m_pH264Encoder;
};

#endif // H264ENCODER_H

#ifndef VIDEOENCODER_H
#define VIDEOENCODER_H
#include <memory>
#include "AVCommon.h"
#include "VideoConvert.h"

class CVideoEncoder : public CEncodBase
{
public:
    CVideoEncoder();
    CVideoEncoder(const CVideoEncoder&) = delete;
    CVideoEncoder& operator=(const CVideoEncoder&) = delete;
    ~CVideoEncoder();
public:
    virtual bool bOpen(AVConfig& videoConfig) override;
    virtual void Close()override;
    virtual AVPacketPtr pEncode(const quint8* pData, quint32 nWidth, quint32 nHeight, quint32 nDataSize,
                                quint64 nPts = 0);
private:
    qint64  m_nPts;
    quint32 m_nWidth;
    quint32 m_nHeight;
    bool m_bForceIdr;
    AVFramePtr  m_pRgbaFrame;
    AVPacketPtr m_pH264Packet;
    std::unique_ptr<CVideoConvert> m_pConverter;
};

#endif // VIDEOENCODER_H

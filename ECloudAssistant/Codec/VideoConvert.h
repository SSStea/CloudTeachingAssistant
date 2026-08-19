#ifndef VIDEOCONVERT_H
#define VIDEOCONVERT_H
#include "AVCommon.h"

struct SwsContext;
class CVideoConvert
{
public:
    CVideoConvert();
    virtual ~CVideoConvert();
    CVideoConvert(const CVideoConvert&) = delete;
    CVideoConvert& operator=(const CVideoConvert&) = delete;
public:
    bool bOpen(qint32 nInWidth,qint32 nInHeight,AVPixelFormat inFormat,
              qint32 nOutWidth,qint32 nOutHeight,AVPixelFormat outFormat);
    void Close();

    qint32 nConvert(AVFramePtr pInFrame,AVFramePtr& pOutFrame);
private:
    qint32 m_nWidth;
    qint32 m_nHeight;
    AVPixelFormat m_format;
    SwsContext* m_pSwsContext;
};

#endif // VIDEOCONVERT_H

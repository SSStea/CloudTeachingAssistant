#ifndef AUDIORESAMPLER_H
#define AUDIORESAMPLER_H

#include "AVCommon.h"
struct SwrContext;
class CAudioResampler
{
public:
    CAudioResampler();
    CAudioResampler(const CAudioResampler&) = delete;
    CAudioResampler& operator=(const CAudioResampler&) = delete;
    ~CAudioResampler();
public:
    void Close();
    int  nConvert(AVFramePtr pInFrame,AVFramePtr& pOutFrame);
    bool bOpen(int nInSampleRate,int nInChannels,AVSampleFormat inFormat,
              int nOutSampleRate, int nOutChannels, AVSampleFormat outFormat);
private:
    SwrContext* m_pSwrContext;
    int m_nInSampleRate = 0;
    int m_nInChannels = 0;
    int m_nInBitsPerSample = 0;
    AVSampleFormat m_inFormat = AV_SAMPLE_FMT_NONE;

    int m_nOutSampleRate = 0;
    int m_nOutChannels = 0;
    int m_nOutBitsPerSample = 0;
    AVSampleFormat m_outFormat = AV_SAMPLE_FMT_NONE;
};


#endif // AUDIORESAMPLER_H

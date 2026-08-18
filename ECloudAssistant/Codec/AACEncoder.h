#ifndef AACENCODER_H
#define AACENCODER_H
#include "AVCommon.h"

class CAudioEncoder;
class CAACEncoder
{
public:
    CAACEncoder() ;
    CAACEncoder(const CAACEncoder&) = delete;
    CAACEncoder& operator=(const CAACEncoder&) = delete;
    ~CAACEncoder();
public:
    bool bOpen(int nSampleRate, int nChannels, int nFormat, int nBitRate_kbps);
    void Close();
    int  nGetFrames();
    int  nGetSpecificConfig(uint8_t* pBuf,int nMaxBufSize);
    AVPacketPtr pEncode(const uint8_t* pPcm,int nSamples);
    inline int  nGetChannel(){return m_nchannel;}
    inline int  nGetSamplerate(){return m_nSampleRate;}
private:
    int m_nchannel = 0;
    int m_nBitRate = 0;
    int m_nSampleRate = 0;
    AVSampleFormat m_format = AV_SAMPLE_FMT_NONE;
    std::unique_ptr<CAudioEncoder> m_pAACEncoder;
};

#endif // AACENCODER_H

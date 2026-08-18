#ifndef AUDIOENCODER_H
#define AUDIOENCODER_H
#include "AVCommon.h"

class CAudioResampler;
class CAudioEncoder : public CEncodBase
{
public:
    CAudioEncoder() ;
    CAudioEncoder(const CAudioEncoder&) = delete;
    CAudioEncoder& operator=(const CAudioEncoder&) = delete;
    ~CAudioEncoder();
public:
    virtual bool bOpen(AVConfig& config) override;
    virtual void Close() override;
    uint32_t     nGetFrameSamples();
    AVPacketPtr  pEncode(const uint8_t *pcm, int samples);
private:
    int64_t m_nPts = 0;
    std::unique_ptr<CAudioResampler> m_pAudioResampler;
};

#endif // AUDIOENCODER_H

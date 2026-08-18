#ifndef AUDIOCAPTURE_H
#define AUDIOCAPTURE_H

#include <thread>
#include <memory>


class CWASAPICapture;
class AudioBuffer;
class CAudioCapture
{
public:
    CAudioCapture();
    ~CAudioCapture();

    bool bInit(uint32_t nSize = 20480);
    void Close();
    uint32_t nGetSamples();
    uint32_t nRead(uint8_t* pData, uint32_t nSamples);

    inline bool bCaptureStarted() const { return m_bIsStarted;}
    inline uint32_t nGetChannels() const {return m_nChannels;}
    inline uint32_t nGetSampleRate() const {return m_nSamplerRate;}
    inline uint32_t nGetBitsPerSample() const {return m_nBitsPerSample;}

private:
    bool m_bIsInitialized = false;
    bool m_bIsStarted = false;
    uint32_t m_nChannels = 2;
    uint32_t m_nSamplerRate = 48000;
    uint32_t m_nBitsPerSample = 16;
    std::unique_ptr<CWASAPICapture> m_capture;
    std::unique_ptr<AudioBuffer> m_audioBuffer;

    int nStartCapture();
    int nStopCapture();
};

#endif // AUDIOCAPTURE_H

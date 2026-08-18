#ifndef WASAPICAPTURE_H
#define WASAPICAPTURE_H
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <wrl.h>
#include <cstdio>
#include <cstdint>
#include <functional>
#include <mutex>
#include <memory>
#include <thread>

class CWASAPICapture
{
public:
    typedef std::function<void(const WAVEFORMATEX *pMixFormat, uint8_t *pData, uint32_t nSamples)> PacketCallback;
    CWASAPICapture();
    CWASAPICapture(const CWASAPICapture&) = delete;
    CWASAPICapture& operator=(const CWASAPICapture&) = delete;
    ~CWASAPICapture();
    int nInit();
    int nExit();
    int nStart();
    int nStop();
    void setCallback(PacketCallback callback);
    WAVEFORMATEX *getAudioFormat() const
    {
        return m_pMixFormat;
    }
private:
    bool m_bInitialized = false;
    bool m_bIsEnabeld = false;
    int nAdjustFormatTo16Bits(WAVEFORMATEX *pwfx);
    int nCapture();
    const int REFTIMES_PER_SEC = 10000000;
    const int REFTIMES_PER_MILLISEC = 10000;
    const IID IID_IAudioClient = __uuidof(IAudioClient);
    const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
    const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
    const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);

    std::mutex m_mutex;
    uint32_t m_nPcmBufSize;
    uint32_t m_nBufferFrameCount;
    PacketCallback m_callback;
    WAVEFORMATEX *m_pMixFormat = NULL;
    std::shared_ptr<uint8_t> m_pPcmBuf; //捕获之后pcm缓存的这个pcmBuf中
    REFERENCE_TIME m_hnsActualDuration;
    std::shared_ptr<std::thread> m_threadPtr;
    Microsoft::WRL::ComPtr<IMMDevice> m_device;
    Microsoft::WRL::ComPtr<IAudioClient> m_audioClient;
    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> m_enumerator;
    Microsoft::WRL::ComPtr<IAudioCaptureClient> m_audioCaptureClient;
};

#endif // WASAPICAPTURE_H

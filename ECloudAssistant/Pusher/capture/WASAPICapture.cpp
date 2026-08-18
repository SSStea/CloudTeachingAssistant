#include "WASAPICapture.h"
#include <QDebug>

CWASAPICapture::CWASAPICapture()
{
    m_nPcmBufSize = 4096;
    m_pPcmBuf.reset(new uint8_t[m_nPcmBufSize], std::default_delete<uint8_t[]>());
}

CWASAPICapture::~CWASAPICapture()
{

}

int CWASAPICapture::nInit()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if(m_bInitialized)
    {
        return 0;
    }

    //初始化com库
    CoInitialize(NULL);

    HRESULT hr = S_OK;
    //创建实例
    hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)m_enumerator.GetAddressOf());
    if(FAILED(hr))
    {
        qDebug() << "CoCreateInstance fail";
        return -1;
    }

    //获取音频输出
    hr = m_enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, m_device.GetAddressOf());
    if(FAILED(hr))
    {
        qDebug() << "GetDefaultAudioEndpoint faile";
        return -1;
    }

    //激活音频设备
    hr = m_device->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)m_audioClient.GetAddressOf());
    if(FAILED(hr))
    {
        qDebug() << "Activate fail";
        return -1;
    }

    //获取音频格式
    hr = m_audioClient->GetMixFormat(&m_pMixFormat);
    if(FAILED(hr))
    {
        qDebug() << "GetMixFormat fail";
        return -1;
    }

    //调整输出格式为16位，方便后续编码
    nAdjustFormatTo16Bits(m_pMixFormat);

    //初始化音频客户端
    hr = m_audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK,
                                   REFTIMES_PER_SEC, 0, m_pMixFormat, NULL);
    if(FAILED(hr))
    {
        qDebug() << "Initialize fail";
        return -1;
    }

    //获取缓冲区大小
    hr = m_audioClient->GetBufferSize(&m_nBufferFrameCount);
    if(FAILED(hr))
    {
        qDebug() << "GetBufferSize fail";
        return -1;
    }

    //获取音频服务
    hr = m_audioClient->GetService(IID_IAudioCaptureClient, (void**)m_audioCaptureClient.GetAddressOf());
    if(FAILED(hr))
    {
        qDebug() << "GetService fail";
        return -1;
    }

    //计算buffer的时长
    m_hnsActualDuration = REFERENCE_TIME(REFTIMES_PER_SEC * m_nBufferFrameCount / m_pMixFormat->nSamplesPerSec);

    m_bInitialized = true;
    return 0;
}

int CWASAPICapture::nExit()
{
    if(m_bInitialized)
    {
        m_bInitialized = false;
        CoUninitialize();
    }

    return 0;
}

int CWASAPICapture::nStart()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if(!m_bInitialized)
    {
        return -1;
    }

    if(m_bIsEnabeld)
    {
        return 0;
    }

    HRESULT hr = m_audioClient->Start();
    if(FAILED(hr))
    {
        qDebug() << "Start fail";
        return -1;
    }

    m_bIsEnabeld = true;

    m_threadPtr.reset(new std::thread([this](){
        while(this->m_bIsEnabeld)
        {
            if(this->nCapture() < 0)
            {
                break;
            }
        }
    }));

    return 0;
}

int CWASAPICapture::nStop()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if(m_bIsEnabeld)
    {
        m_bIsEnabeld = false;
        m_threadPtr->join();
        m_threadPtr.reset();
        m_threadPtr = nullptr;

        HRESULT hr = m_audioClient->Stop();
        if(FAILED(hr))
        {
            qDebug() << "Stop fail";
            return -1;
        }
    }

    return 0;
}

void CWASAPICapture::setCallback(PacketCallback callback)
{
    m_callback = callback;
}

int CWASAPICapture::nAdjustFormatTo16Bits(WAVEFORMATEX *pwfx)
{
    //适配16位
    if(pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
    {
        pwfx->wFormatTag = WAVE_FORMAT_PCM;
    }
    else if(pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
        PWAVEFORMATEXTENSIBLE pEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(pwfx);
        if(IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pEx->SubFormat))
        {
            pEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
            pEx->Samples.wValidBitsPerSample = 16;
        }
    }
    else
    {
        return -1;
    }

    pwfx->wBitsPerSample = 16;
    pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
    pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;

    return 0;
}

int CWASAPICapture::nCapture()
{
    HRESULT hr = S_OK;
    uint32_t nPacketLen = 0;
    uint32_t nNumFrameAvailabel = 0;
    BYTE* pData;
    DWORD flags;

    //获取下一个包大小
    hr = m_audioCaptureClient->GetNextPacketSize(&nPacketLen);
    if(FAILED(hr))
    {
        qDebug() << "GetNextPacketSize fail";
        return -1;
    }

    if(nPacketLen == 0)//没有数据
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    //有数据
    while(nPacketLen > 0)
    {
        hr = m_audioCaptureClient->GetBuffer(&pData, &nNumFrameAvailabel, &flags, NULL, NULL);
        if(FAILED(hr))
        {
            qDebug() << "GetBuffer fail";
            return -1;
        }

        if(m_nPcmBufSize < nNumFrameAvailabel * m_pMixFormat->nBlockAlign)
        {//缓冲区大小不足，扩容
            m_nPcmBufSize = nNumFrameAvailabel * m_pMixFormat->nBlockAlign;
            m_pPcmBuf.reset(new uint8_t[m_nPcmBufSize], std::default_delete<uint8_t[]>());
        }

        if(flags & AUDCLNT_BUFFERFLAGS_SILENT)
        {//当前没有声音，传空
            memset(m_pPcmBuf.get(), 0, m_nPcmBufSize);
        }
        else
        {
            memcpy(m_pPcmBuf.get(), pData, nNumFrameAvailabel);
        }

        if(m_callback)
        {
            m_callback(m_pMixFormat, pData, nNumFrameAvailabel);
        }

        //释放缓冲区
        hr = m_audioCaptureClient->ReleaseBuffer(nNumFrameAvailabel);
        if(FAILED(hr))
        {
            qDebug() << "ReleaseBuffer fail";
            return -1;
        }

        //获取下一个包的大小
        hr = m_audioCaptureClient->GetNextPacketSize(&nPacketLen);
        if(FAILED(hr))
        {
            qDebug() << "GetNextPacketSize fail";
            return -1;
        }
    }

    return 0;
}

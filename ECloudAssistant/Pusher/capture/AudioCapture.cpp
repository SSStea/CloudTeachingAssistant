#include "AudioCapture.h"
#include "AudioBuffer.h"
#include "WASAPICapture.h"


CAudioCapture::CAudioCapture()
    : m_capture(nullptr), m_audioBuffer(nullptr)
{
    m_capture.reset(new CWASAPICapture());
}

CAudioCapture::~CAudioCapture()
{

}

bool CAudioCapture::bInit(uint32_t nSize)
{
    if(m_bIsInitialized)
    {
        return true;
    }

    if(m_capture->nInit() < 0)
    {
        return false;
    }
    else
    {
        WAVEFORMATEX* audoFmt = m_capture->getAudioFormat();
        m_nChannels = audoFmt->nChannels;
        m_nSamplerRate = audoFmt->nSamplesPerSec;
        m_nBitsPerSample = audoFmt->wBitsPerSample;
    }

    //创建buffer
    m_audioBuffer.reset(new AudioBuffer(nSize));
    //启动捕获器，捕获音频
    if(nStartCapture() < 0)
    {
        return false;
    }

    m_bIsInitialized = true;
    return true;
}

void CAudioCapture::Close()
{
    if(m_bIsInitialized)
    {
        nStopCapture();
        m_bIsInitialized = false;
    }
}

uint32_t CAudioCapture::nGetSamples()
{
    //从缓冲区获取当前有多少音频数据
    return m_audioBuffer->size() * 8 / m_nBitsPerSample / m_nChannels;
}

uint32_t CAudioCapture::nRead(uint8_t *pData, uint32_t nSamples)
{
    //从缓冲区读数据
    if(nSamples > this->nGetSamples())//说明数据不足
    {
        return 0;
    }

    m_audioBuffer->read((char*)pData, nSamples * m_nBitsPerSample / 8 * m_nChannels);

    return nSamples;
}

int CAudioCapture::nStartCapture()
{
    //开始捕获
    m_capture->setCallback([this](const WAVEFORMATEX *pMixFormat, uint8_t *pData, uint32_t nSamples){
        m_nChannels = pMixFormat->nChannels;
        m_nSamplerRate = pMixFormat->nSamplesPerSec;
        m_nBitsPerSample = pMixFormat->wBitsPerSample;
        //将数据写入缓冲区
        m_audioBuffer->write((char*)pData, pMixFormat->nBlockAlign * nSamples);
    });

    //清空音频缓冲区，缓冲音频数据
    m_audioBuffer->clear();
    //开始捕获音频
    if(m_capture->nStart() < 0)
    {
        return -1;
    }

    m_bIsStarted = true;
    return 0;
}

int CAudioCapture::nStopCapture()
{
    m_capture->nStop();
    m_bIsStarted = false;

    return 0;
}

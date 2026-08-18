#include "AACEncoder.h"
#include "AudioEncoder.h"
#include <QDebug>

CAACEncoder::CAACEncoder()
{
    m_pAACEncoder.reset(new CAudioEncoder());
}

CAACEncoder::~CAACEncoder()
{
    Close();
}

bool CAACEncoder::bOpen(int nSampleRate, int nChannels, int nFormat, int nBitRate_kbps)
{
    //初始化aac编码器
    if(m_pAACEncoder->pGetAVCodecContext())
    {
        qDebug() << "m_pAACEncoder is initialzed";
        return false;
    }

    AVConfig encoderConfig;
    encoderConfig.audio.nSamplerate = nSampleRate;
    encoderConfig.audio.nChannels = nChannels;
    encoderConfig.audio.nBitRate = nBitRate_kbps;
    encoderConfig.audio.format = (AVSampleFormat)nFormat;

    //初始化编码器
    if(!m_pAACEncoder->bOpen(encoderConfig))
    {
        qDebug() << "m_pAACEncoder->bOpen fail";
        return false;
    }

    return true;
}

void CAACEncoder::Close()
{
    m_nchannel = 0;
    m_nBitRate = 0;
    m_nSampleRate = 0;
    m_format = AV_SAMPLE_FMT_NONE;

    if(m_pAACEncoder)
    {
        m_pAACEncoder->Close();
        m_pAACEncoder.reset();
        m_pAACEncoder = nullptr;
    }
}

int CAACEncoder::nGetFrames()
{
    if(!m_pAACEncoder->pGetAVCodecContext())
    {
        qDebug() << "m_pAACEncoder is not exist";
        return -1;
    }

    return m_pAACEncoder->nGetFrameSamples();
}

int CAACEncoder::nGetSpecificConfig(uint8_t *pBuf, int nMaxBufSize)
{
    //获取编码器参数
    //先获取编码器上下文
    AVCodecContext* codecCtx = m_pAACEncoder->pGetAVCodecContext();
    if(!codecCtx)
    {
        qDebug() << "m_pAACEncoder is not exist";
        return -1;
    }
    //对比上下文参数大小跟输入大小，
    if(nMaxBufSize < codecCtx->extradata_size)
    {
        qDebug() << "nMaxBufSize is too small";
        return -1;
    }
    //将参数拷贝出去
    memcpy(pBuf, codecCtx->extradata, codecCtx->extradata_size);

    return codecCtx->extradata_size;
}

AVPacketPtr CAACEncoder::pEncode(const uint8_t *pPcm, int nSamples)
{
    //编码AAC
    //判断aac编码器是否存在
    if(!m_pAACEncoder)
    {
        qDebug() << "m_pAACEncoder is not exist";
        return nullptr;
    }

    return m_pAACEncoder->pEncode(pPcm, nSamples);
}

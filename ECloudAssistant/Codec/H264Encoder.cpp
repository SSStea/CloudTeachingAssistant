#include "H264Encoder.h"
#include "VideoEncoder.h"
#include <QDebug>

CH264Encoder::CH264Encoder()
    :m_config{}
{
    m_pH264Encoder.reset(new CVideoEncoder());
}

CH264Encoder::~CH264Encoder()
{
    Close();
}

bool CH264Encoder::bOPen(qint32 nWidth, qint32 nHeight, qint32 nFramerate, qint32 nBitRate, qint32 nFormat)
{
    //初始化编码器
    m_config.video.nWidth = nWidth;
    m_config.video.nHeight = nHeight;
    m_config.video.nFrameRate = nFramerate;
    m_config.video.nBitRate = nBitRate * 1000;
    m_config.video.nGop = nFramerate;
    m_config.video.format = (AVPixelFormat)nFormat;

    return m_pH264Encoder->bOpen(m_config);
}

void CH264Encoder::Close()
{
    m_pH264Encoder->Close();
}

qint32 CH264Encoder::nEncode(quint8 *pRgbaBuffer, quint32 nWidth, quint32 nHeight, quint32 nSize, std::vector<quint8> &vecOutFrame)
{
    //编码264
    vecOutFrame.clear();
    int nFrameSize = 0;
    int nMaxOutSize = m_config.video.nWidth * m_config.video.nHeight * 4;//编码数据不会大于原始数据rgba，所以设大一点
    //申请内存
    std::shared_ptr<quint8> pOutBuffer(new quint8[nMaxOutSize], std::default_delete<quint8[]>());
    //编码
    AVPacketPtr pPkt = m_pH264Encoder->pEncode(pRgbaBuffer, nWidth, nHeight, nSize);
    if(!pPkt)
    {
        qDebug() << "m_pH264Encoder->pEncode fail";
        return -1;
    }

    quint32 nExtraSize = 0;
    quint8* pExtraData = nullptr;
    //判断是否是关键帧，如果是关键帧需要再264帧前面添加编码信息
    if(bIsKeyFrame(pPkt))
    {
        //添加编码信息
        //获取编码信息
        pExtraData = m_pH264Encoder->pGetAVCodecContext()->extradata;
        nExtraSize = m_pH264Encoder->pGetAVCodecContext()->extradata_size;
        //编码信息放在包头解析
        memcpy(pOutBuffer.get(), pExtraData, nExtraSize);
        nFrameSize += nExtraSize;
    }
    memcpy(pOutBuffer.get() + nFrameSize, pPkt->data, pPkt->size);//264 [编码信息 + 264裸流]
    nFrameSize += pPkt->size;

    //将数据传出
    if(nFrameSize > 0)
    {
        vecOutFrame.resize(nFrameSize);
        vecOutFrame.assign(pOutBuffer.get(), pOutBuffer.get() + nFrameSize);
        return nFrameSize;
    }

    return 0;
}

qint32 CH264Encoder::nGetSequenceParams(quint8 *pOutBuffer, qint32 nOutBufferSize)
{
    //获取编码参数
    quint32 nSize = 0;
    if(!m_pH264Encoder->pGetAVCodecContext())
    {
        qDebug() << "!m_pH264Encoder->pGetAVCodecContext()";
        return -1;
    }

    AVCodecContext* codecCtx = m_pH264Encoder->pGetAVCodecContext();
    nSize = codecCtx->extradata_size;
    memcpy(pOutBuffer, codecCtx->extradata, codecCtx->extradata_size);

    return nSize;
}

bool CH264Encoder::bIsKeyFrame(AVPacketPtr pPkt)
{
    //判断是否位关键帧

    return pPkt->flags & AV_PKT_FLAG_KEY;
}

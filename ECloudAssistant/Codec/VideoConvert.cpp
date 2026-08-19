#include "VideoConvert.h"
extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
#include <QDebug>

CVideoConvert::CVideoConvert()
    :m_nWidth(0), m_nHeight(0), m_format(AV_PIX_FMT_NONE), m_pSwsContext(nullptr)
{

}

CVideoConvert::~CVideoConvert()
{
    Close();
}

bool CVideoConvert::bOpen(qint32 nInWidth, qint32 nInHeight, AVPixelFormat inFormat,
                          qint32 nOutWidth, qint32 nOutHeight, AVPixelFormat outFormat)
{
    //初始化转换器
    if(m_pSwsContext)
    {
        qDebug() << "m_pSwsContext is exist";
        return false;
    }

    m_pSwsContext = sws_getContext(nInWidth, nInHeight, inFormat,
                                   nOutWidth, nOutHeight, outFormat,
                                   SWS_BICUBIC, NULL, NULL, NULL);
    m_nWidth = nOutWidth;
    m_nHeight = nOutHeight;
    m_format = outFormat;

    return m_pSwsContext != nullptr;
}

void CVideoConvert::Close()
{
    if(m_pSwsContext)
    {
        sws_freeContext(m_pSwsContext);
        m_pSwsContext = nullptr;
    }
}

qint32 CVideoConvert::nConvert(AVFramePtr pInFrame, AVFramePtr &pOutFrame)
{
    //转换像素或格式
    if(!m_pSwsContext)
    {
        qDebug() << "m_pSwsContext is not exist";
        return -1;
    }

    //创建输出帧
    pOutFrame.reset(av_frame_alloc(), [](AVFrame* ptr){
        av_frame_free(&ptr);
    });

    //初始化输出帧
    pOutFrame->width = m_nWidth;
    pOutFrame->height = m_nHeight;
    pOutFrame->format = m_format;
    pOutFrame->pts = pInFrame->pts;
    pOutFrame->pkt_dts = pInFrame->pkt_dts;

    //获取内存
    if(av_frame_get_buffer(pOutFrame.get(), 0) != 0)
    {
        qDebug() << "av_frame_get_buffer fail";
        return -1;
    }

    //开始转换
    uint32_t nHeightSlice = sws_scale(m_pSwsContext, pInFrame->data, pInFrame->linesize, 0,
                                      pInFrame->height, pOutFrame->data, pOutFrame->linesize);
    if(nHeightSlice < 0)
    {
        qDebug() << "sws_scale fail";
        return -1;
    }

    return nHeightSlice;
}

#include "VideoEncoder.h"
#include "VideoConvert.h"
extern "C"
{
#include <libavutil/rational.h>
#include <libavutil/error.h>
#include <libavutil/opt.h>
}
#include <QDebug>

CVideoEncoder::CVideoEncoder()
    :m_nPts(0), m_nWidth(0), m_nHeight(0), m_bForceIdr(false),
    m_pRgbaFrame(nullptr), m_pH264Packet(nullptr), m_pConverter(nullptr)
{
    //创建AVFrame，AVPacket
    m_pRgbaFrame.reset(av_frame_alloc(), [](AVFrame* ptr){
        av_frame_free(&ptr);
    });
    m_pH264Packet.reset(av_packet_alloc(), [](AVPacket* ptr){
        av_packet_free(&ptr);
    });
}

CVideoEncoder::~CVideoEncoder()
{
    Close();
}

bool CVideoEncoder::bOpen(AVConfig &videoConfig)
{
    if(m_bIsInitialzed)
    {
        Close();
        qDebug() << "m_bIsInitialzed";
        return false;
    }

    m_config = videoConfig;

    //查找编码器H264
    m_pCodec = const_cast<AVCodec*>(avcodec_find_encoder(AV_CODEC_ID_H264));
    if(!m_pCodec)
    {
        Close();
        qDebug() << "avcodec_find_encoder fail";
        return false;
    }

    //创建编码器上下文
    m_pCodecContext = avcodec_alloc_context3(m_pCodec);
    if(!m_pCodecContext)
    {
        Close();
        qDebug() << "avcodec_alloc_context3 fail";
        return false;
    }

    //配置上下文参数
    m_pCodecContext->width = m_config.video.nWidth;
    m_pCodecContext->height = m_config.video.nHeight;
    m_pCodecContext->time_base = {1, (qint32)m_config.video.nFrameRate};//时间基：帧率的倒数
    m_pCodecContext->framerate = {(qint32)m_config.video.nFrameRate, 1};
    m_pCodecContext->gop_size = 30;
    m_pCodecContext->max_b_frames = 0;//降低延迟
    m_pCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    m_pCodecContext->bit_rate = m_config.video.nBitRate;

    //设置全局头
    m_pCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    //加参数，降低编码延迟
    m_pCodecContext->rc_min_rate = m_config.video.nBitRate;
    m_pCodecContext->rc_max_rate = m_config.video.nBitRate;
    m_pCodecContext->rc_buffer_size = m_config.video.nBitRate;

    //设置字典
    av_opt_set(m_pCodecContext->priv_data, "tune", "zerolatency", 0);
    av_opt_set(m_pCodecContext->priv_data, "preset", "ultrafast", 0);

    //打开编码器
    if(avcodec_open2(m_pCodecContext, m_pCodec, NULL) != 0)
    {
        Close();
        qDebug() << "avcodec_open2 fail";
        return false;
    }

    m_nWidth = m_config.video.nWidth;
    m_nHeight = m_config.video.nHeight;
    m_bIsInitialzed = true;

    return true;
}

void CVideoEncoder::Close()
{
    m_nWidth = 0;
    m_nHeight = 0;
    m_nPts = 0;
    m_bIsInitialzed = false;
    if(m_pConverter)
    {
        m_pConverter->Close();
        m_pConverter.reset();
        m_pConverter = nullptr;
    }
}

AVPacketPtr CVideoEncoder::pEncode(const quint8 *pData, quint32 nWidth, quint32 nHeight, quint32 nDataSize, quint64 nPts)
{
    if(!m_bIsInitialzed)
    {
        qDebug() << "not Initialzed";
        return nullptr;
    }

    //初始化转换器，如果输入的宽高与目标不一致，创建转换器
    if(m_nWidth != nWidth || m_nHeight != nHeight || !m_pConverter)
    {
        m_pConverter.reset(new CVideoConvert());
        //初始化
        if(!m_pConverter->bOpen(nWidth, nHeight, (AVPixelFormat)m_config.video.format,
                                 m_pCodecContext->width, m_pCodecContext->height, m_pCodecContext->pix_fmt))
        {
            //初始化失败
            m_pConverter.reset();
            qDebug() << "m_pConverter->bOpen fail";
            return nullptr;
        }

        m_pRgbaFrame->width = m_nWidth;
        m_pRgbaFrame->height = m_nHeight;

        //获取内存
        if(av_frame_get_buffer(m_pRgbaFrame.get(), 0) != 0)
        {
            qDebug() << "av_frame_get_buffer fail";
            return nullptr;
        }
    }

    //将输入数据转到rgbaframe中转换
    memcpy(m_pRgbaFrame->data[0], pData, nDataSize);

    //转换
    if(!m_pConverter)
    {
        qDebug() << "m_pConverter is not exist";
        return nullptr;
    }

    AVFramePtr pOutFrame = nullptr;
    if(m_pConverter->nConvert(m_pRgbaFrame, pOutFrame) <= 0)
    {
        qDebug() << "m_pConverter->nConvert fail";
        return nullptr;
    }

    //更新outframe参数
    pOutFrame->pts = nPts >= 0 ? nPts : m_nPts++;
    pOutFrame->pict_type = AV_PICTURE_TYPE_NONE;

    //编码数据
    if(avcodec_send_frame(m_pCodecContext, pOutFrame.get()) < 0)
    {
        qDebug() << "avcodec_send_frame fail";
        return nullptr;
    }
    int nRet = avcodec_receive_packet(m_pCodecContext, m_pH264Packet.get());
    if(nRet == AVERROR(EAGAIN) || nRet == AVERROR_EOF)
    {
        qDebug() << "avcodec_receive_packet: nRet == AVERROR(EAGAIN) || nRet == AVERROR_EOF, fail";
        return nullptr;
    }
    else if(nRet < 0)
    {
        qDebug() << "avcodec_receive_packet: nRet < 0, fail";
        return nullptr;
    }

    return m_pH264Packet;
}

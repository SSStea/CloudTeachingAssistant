#include "AudioEncoder.h"
#include "AudioResampler.h"
#include <QDebug>

CAudioEncoder::CAudioEncoder()
    :m_pAudioResampler(nullptr)
{

}

CAudioEncoder::~CAudioEncoder()
{
    Close();
}

bool CAudioEncoder::bOpen(AVConfig &config)
{
    //初始化编码器
    if(m_bIsInitialzed)
    {
        qDebug() << "encoder is initialzed";
        return false;
    }

    m_config = config;
    //创建编码器
    m_pCodec = const_cast<AVCodec*>(avcodec_find_encoder(AV_CODEC_ID_AAC));
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

    //设置编码器上下文参数
    m_pCodecContext->sample_rate = config.audio.nSamplerate;
    m_pCodecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
    av_channel_layout_default(&m_pCodecContext->ch_layout, config.audio.nChannels);
    m_pCodecContext->bit_rate = config.audio.nBitRate;

    //获取全局的AAC头部
    m_pCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    //打开编码器
    if(avcodec_open2(m_pCodecContext, m_pCodec, NULL) != 0)
    {
        Close();
        qDebug() << "avcodec_open2 fail";
        return false;
    }

    //创建重采样对象
    m_pAudioResampler.reset(new CAudioResampler());
    //初始化重采样对象，将格式转为FLTP
    if(!m_pAudioResampler->bOpen(config.audio.nSamplerate, config.audio.nChannels, config.audio.format,
                                  config.audio.nSamplerate, config.audio.nChannels, AV_SAMPLE_FMT_FLTP))
    {
        Close();
        qDebug() << "m_pAudioResampler->bOpen fail";
        return false;
    }

    m_bIsInitialzed = true;

    return true;
}

void CAudioEncoder::Close()
{
    if(m_pAudioResampler)
    {
        m_pAudioResampler->Close();
        m_pAudioResampler.reset();
        m_pAudioResampler = nullptr;
    }
}

uint32_t CAudioEncoder::nGetFrameSamples()
{
    //获取帧数（样品数）
    if(m_bIsInitialzed)
    {
        return m_pCodecContext->frame_size;
    }

    return 0;
}

AVPacketPtr CAudioEncoder::pEncode(const uint8_t *pcm, int samples)
{
    AVFramePtr pInFrame(av_frame_alloc(), [](AVFrame* ptr){
        av_frame_free(&ptr);
    });
    pInFrame->sample_rate = m_pCodecContext->sample_rate;
    pInFrame->format = AV_SAMPLE_FMT_FLT;
    av_channel_layout_default(&pInFrame->ch_layout, m_pCodecContext->ch_layout.nb_channels);
    pInFrame->nb_samples = samples;
    pInFrame->pts = m_nPts;
    pInFrame->pts = av_rescale_q(m_nPts, {1, m_pCodecContext->sample_rate}, m_pCodecContext->time_base);
    m_nPts += pInFrame->nb_samples;

    //申请内存创建
    if(av_frame_get_buffer(pInFrame.get(), 0) < 0)
    {
        qDebug() << "av_frame_get_buffer fail";
        return nullptr;
    }

    //计算位数
    int nBytesPerSamples = av_get_bytes_per_sample(m_config.audio.format);
    if(nBytesPerSamples == 0)
    {
        qDebug() << "av_get_bytes_per_sample fail";
        return nullptr;
    }

    //开始拷贝内存 pcm->inframe
    memcpy(pInFrame->data[0], pcm, nBytesPerSamples * pInFrame->ch_layout.nb_channels * samples);

    //开始重采样
    AVFramePtr pFltpFrame = nullptr;
    if(m_pAudioResampler->nConvert(pInFrame, pFltpFrame) <= 0)
    {
        qDebug() << "m_pAudioResampler->nConvert fail";
        return nullptr;
    }

    //开始编码数据
    int nRet = avcodec_send_frame(m_pCodecContext, pFltpFrame.get());
    if(nRet != 0)
    {
        qDebug() << "avcodec_send_frame fail";
        return nullptr;
    }

    AVPacketPtr pAvPacket(av_packet_alloc(), [](AVPacket* ptr){
        av_packet_free(&ptr);
    });
    //初始化packet
    av_init_packet(pAvPacket.get());
    //开始接收packet
    nRet = avcodec_receive_packet(m_pCodecContext, pAvPacket.get());
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

    return pAvPacket;
}

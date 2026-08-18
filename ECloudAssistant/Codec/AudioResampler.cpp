#include "AudioResampler.h"
extern "C"
{
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
}
#include <QDebug>

CAudioResampler::CAudioResampler()
    :m_pSwrContext(nullptr)
{

}

CAudioResampler::~CAudioResampler()
{
    Close();
}

void CAudioResampler::Close()
{
    if(m_pSwrContext)
    {
        if(swr_is_initialized(m_pSwrContext))
        {
            swr_close(m_pSwrContext);
            m_pSwrContext = nullptr;
        }
    }
}

int CAudioResampler::nConvert(AVFramePtr pInFrame, AVFramePtr &pOutFrame)
{
    //重采样
    if(!m_pSwrContext)
    {
        qDebug() << "no m_pSwrContext";
        return -1;
    }

    //更新输出参数
    pOutFrame.reset(av_frame_alloc(), [](AVFrame* ptr){
        av_frame_free(&ptr);
    });
    pOutFrame->sample_rate = m_nOutSampleRate;
    pOutFrame->format = m_outFormat;
    pOutFrame->ch_layout.nb_channels = m_nOutChannels;
    int64_t nDelay = swr_get_delay(m_pSwrContext, pInFrame->sample_rate);
    pOutFrame->nb_samples = av_rescale_rnd(nDelay + pInFrame->nb_samples, m_nOutSampleRate,
                                           pInFrame->sample_rate, AV_ROUND_UP);
    pOutFrame->pts = pOutFrame->pkt_dts = pInFrame->pts;

    //获取内存
    if(av_frame_get_buffer(pOutFrame.get(), 0) != 0)
    {
        qDebug() << "av_frame_get_buffer fail";
        return -1;
    }

    //开始重采样
    int nLen = swr_convert(m_pSwrContext, (uint8_t**)&pOutFrame->data, pOutFrame->nb_samples,
                           (const uint8_t**)&pInFrame->data, pInFrame->nb_samples);
    if(nLen < 0)
    {
        //失败，清空内存
        pOutFrame.reset();
        pOutFrame = nullptr;
        qDebug() << "swr_convert fail";
        return -1;
    }

    //更新实际样品数
    pOutFrame->nb_samples = nLen;
    return nLen;
}

bool CAudioResampler::bOpen(int nInSampleRate, int nInChannels, AVSampleFormat inFormat,
                            int nOutSampleRate, int nOutChannels, AVSampleFormat outFormat)
{
    if(m_pSwrContext)
    {
        qDebug() << "m_pSwrContext is already exist";
        return false;
    }

    //初始化转换器
    AVChannelLayout inChannelLayout = {};
    av_channel_layout_default(&inChannelLayout, nInChannels);
    AVChannelLayout outChannelLayout = {};
    av_channel_layout_default(&outChannelLayout, nOutChannels);

    //创建转换器
    m_pSwrContext = swr_alloc();

    //设置参数
    av_opt_set_chlayout(
        m_pSwrContext,
        "in_chlayout",
        &inChannelLayout,
        0);
    av_opt_set_int(m_pSwrContext, "in_sample_rate", nInSampleRate, 0);
    av_opt_set_int(m_pSwrContext, "in_sample_fmt", inFormat, 0);

    av_opt_set_chlayout(
        m_pSwrContext,
        "out_chlayout",
        &outChannelLayout,
        0);
    av_opt_set_int(m_pSwrContext, "out_sample_rate", nOutSampleRate, 0);
    av_opt_set_int(m_pSwrContext, "out_sample_fmt", outFormat, 0);

    //初始化转换器
    int nRet = swr_init(m_pSwrContext);
    if(nRet < 0)
    {
        qDebug() << "m_pSwrContext swr_init fail";
        return false;
    }

    //更新属性参数
    m_nInSampleRate = nInSampleRate;
    m_nInChannels = nInChannels;
    m_nInBitsPerSample = av_get_bytes_per_sample(inFormat);
    m_inFormat = inFormat;

    m_nOutSampleRate = nOutSampleRate;
    m_nOutChannels = nOutChannels;
    m_nOutBitsPerSample = av_get_bytes_per_sample(outFormat);
    m_outFormat = outFormat;

    return true;
}

#include "AACDecoder.h"
#include "AudioResampler.h"
#include <QDebug>

CAACDecoder::CAACDecoder(AVContext *ac, QObject *parent)
    :QThread(parent), m_pAvContext(ac)
{
    m_pAudioResampler.reset(new CAudioResampler());
}

CAACDecoder::~CAACDecoder()
{
    Close();
}

int CAACDecoder::nOpen(const AVCodecParameters *pCodecParamer)
{
    //初始化解码器，通过参数初始化
    if(m_bIsInitialzed || !pCodecParamer)
    {
        qDebug() << "m_bIsInitialzed || !pCodecParamer";
        return -1;
    }

    //创建解码器
    m_pCodec = const_cast<AVCodec*>(avcodec_find_decoder(pCodecParamer->codec_id));
    if(!m_pCodec)
    {
        qDebug() << "avcodec_find_decoder fail";
        return -1;
    }

    //创建解码器上下文
    m_pCodecCtx = avcodec_alloc_context3(m_pCodec);
    if(!m_pCodecCtx)
    {
        qDebug() << "avcodec_alloc_context3 fail";
        return -1;
    }
    //复制解码器参数pCodecParamer->m_pCodecCtx
    if(avcodec_parameters_to_context(m_pCodecCtx, pCodecParamer) < 0)
    {
        qDebug() << "avcodec_parameters_to_context fail";
        return -1;
    }

    //设置属性，加速解码速度
    m_pCodecCtx->flags |= AV_CODEC_FLAG2_FAST;

    //打开解码器
    if(avcodec_open2(m_pCodecCtx, m_pCodec, NULL) != 0)
    {
        qDebug() << "avcodec_open2 fail";
        return -1;
    }

    //初始化重采样
    m_pAvContext->nAudioChannelsLayout = AV_CH_LAYOUT_STEREO;//立体声
    m_pAvContext->audioFmt = AV_SAMPLE_FMT_S16;
    m_pAvContext->nAudioSampleRate = 44100;
    //打开重采样
    if(!m_pAudioResampler->bOpen(m_pCodecCtx->sample_rate, m_pCodecCtx->ch_layout.nb_channels,
                                 m_pCodecCtx->sample_fmt, 44100, 2, AV_SAMPLE_FMT_S16))
    {
        qDebug() << "m_pAudioResampler->bOpen fail";
        return -1;
    }

    m_bIsInitialzed = true;
    start();
    return 0;
}

void CAACDecoder::Close()
{
    m_bIsInitialzed = false;
    m_bQuit = true;
    if(isRunning())
    {
        this->quit();
        this->wait();
    }
}

void CAACDecoder::run()
{
    //解码线程
    int nRet = -1;
    //准备AVPacket
    AVPacketPtr pkt = nullptr;
    //输出帧
    AVFramePtr pOutFrame = nullptr; //重采样之后的音频帧
    AVFramePtr pFrame = AVFramePtr(av_frame_alloc(), [](AVFrame* p){
        av_frame_free(&p);
    });

    while(!m_bQuit && m_pAudioResampler)
    {
        //获取包队列
        if(!m_pAudioQueue.nSize())//为空，休眠
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        m_pAudioQueue.bPop(pkt);
        //开始解码数据
        if(avcodec_send_packet(m_pCodecCtx,pkt.get()) != 0)
        {
            qDebug() << "avcodec_send_packet fail";
            break;
        }

        //接收
        while(true)
        {
            nRet = avcodec_receive_frame(m_pCodecCtx, pFrame.get());
            if(nRet == AVERROR(EAGAIN) || nRet == AVERROR_EOF)
            {
                //qDebug() << "avcodec_receive_frame: nRet == AVERROR(EAGAIN) || nRet == AVERROR_EOF, fail";
                break;
            }
            else if(nRet < 0)
            {
                //qDebug() << "avcodec_receive_frame: nRet < 0, fail";
                break;
            }
            else
            {
                //处理帧
                //重采样
                if(m_pAudioResampler->nConvert(pFrame, pOutFrame) > 0)
                {
                    if(pOutFrame)
                    {
                        //再将帧数据填充
                        m_pAvContext->qAudioQueue.push(pOutFrame);
                    }
                }
            }
        }
    }
}

#include "H264Decoder.h"
#include "VideoConvert.h"
#include <QDebug>

CH264Decoder::CH264Decoder(AVContext *ac, QObject *parent)
    :QThread(parent), m_pAvContext(ac)
{
    m_pVideoConver.reset(new CVideoConvert());
    m_pYuvFrame = AVFramePtr(av_frame_alloc(), [](AVFrame* p){
        av_frame_free(&p);
    });
}

CH264Decoder::~CH264Decoder()
{
    Close();
}

int CH264Decoder::nOpen(const AVCodecParameters *pCodecParamer)
{
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

    //更新上下文
    m_pAvContext->nVideoWidth = m_pCodecCtx->width;
    m_pAvContext->nVideoHeight = m_pCodecCtx->height;
    m_pAvContext->videoFmt = AV_PIX_FMT_YUV420P;

    //初始化视频转换器
    if(!m_pVideoConver->bOpen(m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt,
                                m_pCodecCtx->width, m_pCodecCtx->height, AV_PIX_FMT_YUV420P))
    {
        qDebug() << "m_pVideoConver->bOpen fail";
        return -1;
    }

    m_bIsInitialzed = true;
    //启动线程
    start();

    return 0;
}

void CH264Decoder::Close()
{
    m_bQuit = true;
    m_bIsInitialzed = false;
    if(isRunning())
    {
        this->quit();
        this->wait();
    }
}

void CH264Decoder::run()
{
    AVPacketPtr pkt = nullptr;
    while(!m_bQuit && m_pVideoConver)
    {
        if(!m_qVideoQueue.nSize())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        //队列有数据，pop
        m_qVideoQueue.bPop(pkt);
        if(avcodec_send_packet(m_pCodecCtx, pkt.get()) != 0)
        {
            qDebug() << "avcodec_send_packet fail";
            break;
        }

        int nRet = 0;
        while(nRet >= 0)
        {
            //开始接受frame，解码
            nRet = avcodec_receive_frame(m_pCodecCtx, m_pYuvFrame.get());
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

            //转换视频帧
            AVFramePtr pOutFrame = nullptr;
            if(m_pVideoConver->nConvert(m_pYuvFrame, pOutFrame) > 0)
            {
                if(pOutFrame)
                {
                    //添加到视频帧队列
                    m_pAvContext->qVideoQueue.push(pOutFrame);
                }
            }
        }
    }
}

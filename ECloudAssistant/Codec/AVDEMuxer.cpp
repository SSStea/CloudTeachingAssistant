#include "AVDEMuxer.h"
#include <QDebug>

CAVDEMuxer::CAVDEMuxer(AVContext *ac)
    :m_pAvContext(ac), m_pAvDict(nullptr), m_pReadthread(nullptr)
{
    //设置字典
    av_dict_set(&m_pAvDict, "stimeout", "1000000", 0);
    av_dict_set(&m_pAvDict, "analyzeduration", "0", 0);//分析时长，快速播放
    av_dict_set(&m_pAvDict, "fflags", "nobuffer", 0);//快速播放
    av_dict_set(&m_pAvDict, "flags", "low_delay", 0);//无延迟
    av_dict_set(&m_pAvDict, "tune", "zerolatency", 0);//零延迟

    //创建封装格式上下文
    m_pFormateCtx = avformat_alloc_context();
    //设置参数
    m_pFormateCtx->max_ts_probe = 50;
    m_pFormateCtx->probesize = 500000;
    m_pFormateCtx->interrupt_callback.callback = nInterruptFouction;//通过回调函数退出
    m_pFormateCtx->interrupt_callback.opaque = this;
    m_pFormateCtx->flags |= AVFMT_FLAG_DISCARD_CORRUPT;//加速
}

CAVDEMuxer::~CAVDEMuxer()
{
    Close();
}

bool CAVDEMuxer::bOpen(const std::string &strPath)
{
    //启动线程去查询流信息
    m_pReadthread.reset(new std::thread([this, strPath](){
        this->FetchStream(strPath);
    }));

    return true;
}

void CAVDEMuxer::Close()
{
    m_bQuit = true;
    //释放字典
    if(m_pAvDict)
    {
        av_dict_free(&m_pAvDict);
    }
    //停止线程，回收资源
    if(m_pReadthread)
    {
        if(m_pReadthread->joinable())
        {
            m_pReadthread->join();
            m_pReadthread.reset();
            m_pReadthread = nullptr;
        }
    }

    //释放封装格式上下文
    if(m_pFormateCtx)
    {
        avformat_close_input(&m_pFormateCtx);
        m_pFormateCtx = nullptr;
    }
}

void CAVDEMuxer::FetchStream(const std::string &strPath)
{
    bool bRet = false;;
    //读流信息
    //返回流查询结果，通过回调函数回调出去
    if(m_streamCb)
    {
        bRet = bFetchStreamInfo(strPath);
        m_streamCb(bRet);
    }

    //如果结果为true，就是有流信息，开始读数据
    if(!bRet)
    {
        qDebug() << "bFetchStreamInfo ret false";
        return ;
    }

    AVPacketPtr pkt = nullptr;
    while(!m_bQuit && m_pFormateCtx)
    {
        pkt = AVPacketPtr(av_packet_alloc(), [](AVPacket* p){
            av_packet_free(&p);
        });

        int nRet = av_read_frame(m_pFormateCtx, pkt.get());
        if(nRet == 0)//读取成功
        {
            if(pkt->stream_index == m_nVideoIndex)//h264数据包
            {
                //将数据传到h264解码器队列中
            }
            else if(pkt->stream_index == m_nAudioIndex)
            {
                //将数据传到aac解码器队列中
            }
            else
            {
                //释放这个包
                av_packet_unref(pkt.get());
            }
        }
        else
        {
            //释放资源
            av_packet_unref(pkt.get());
            break;
        }
    }
}

bool CAVDEMuxer::bFetchStreamInfo(const std::string &strPath)
{
    //获取流信息
    if(avformat_open_input(&m_pFormateCtx, strPath.c_str(), nullptr, &m_pAvDict) != 0)
    {
        qDebug() << "avformat_open_input fail";
        return false;
    }

    //查询流信息
    if(avformat_find_stream_info(m_pFormateCtx, nullptr) < 0)
    {
        qDebug() << "avformat_find_stream_info fail";
        return false;
    }

    //遍历流
    for(int i = 0; i < m_pFormateCtx->nb_streams; i++)
    {
        if(m_pFormateCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            //更新视频索引
            m_nVideoIndex = i;
            //时长
            m_pAvContext->dVideoDuration = m_pFormateCtx->streams[i]->duration
                                           * av_q2d(m_pFormateCtx->streams[i]->time_base);
        }
        else if(m_pFormateCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            m_nAudioIndex = i;
            m_pAvContext->dVideoDuration = m_pFormateCtx->streams[i]->duration
                                           * av_q2d(m_pFormateCtx->streams[i]->time_base);
        }
    }

    if(m_nVideoIndex != -1)//存在视频流
    {
        //初始化视频编码器

    }
    if(m_nAudioIndex != -1)//存在音频流
    {
        //初始化音频解码器
    }

    return true;
}

double CAVDEMuxer::dAudioDuration()
{
    //音频时长

    return m_pAvContext->dAudioDuration;
}

double CAVDEMuxer::dVideoDuration()
{
    return m_pAvContext->dVideoDuration;
}

int CAVDEMuxer::nInterruptFouction(void *arg)
{
    //退出标志
    CAVDEMuxer* thiz = (CAVDEMuxer*)arg;
    return thiz->m_bQuit;
}

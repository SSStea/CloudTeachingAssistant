#include "GDIScreenScapture.h"
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
#include <QDebug>


CGDIScreenScapture::CGDIScreenScapture()
    :m_bStop(false),
    m_bIsInitialzed(false),
    m_nFrameSize(0),
    m_pRgbaFrame(nullptr),
    m_nWidth(0),
    m_nHeight(0),
    m_nVideoIndex(-1),
    m_nFrameRate(25),
    m_pInputFormat(nullptr),
    m_pCodecContext(nullptr),
    m_pFormatContext(nullptr),
    m_pAvFrame(nullptr),
    m_pAvPacket(nullptr)
{
    //初始化，注册设备，使用gdi采集
    avdevice_register_all();

    //创建frame和packet
    m_pAvFrame = std::shared_ptr<AVFrame>(av_frame_alloc(), [](AVFrame* ptr){av_frame_free(&ptr);});
    m_pAvPacket = std::shared_ptr<AVPacket>(av_packet_alloc(), [](AVPacket* ptr){av_packet_free(&ptr);});
}

CGDIScreenScapture::~CGDIScreenScapture()
{
    bClose();
}

quint32 CGDIScreenScapture::nGetWidth() const
{
    return m_nWidth;
}

quint32 CGDIScreenScapture::nGetHeight() const
{
    return m_nHeight;
}

bool CGDIScreenScapture::bInit(qint64 nDisplayIndex)
{
    if(m_bIsInitialzed)
    {
        return true;
    }

    AVDictionary* options = nullptr;
    //设置属性
    //设置采集帧率
    av_dict_set_int(&options, "framerate", m_nFrameRate, AV_DICT_MATCH_CASE);
    //绘制鼠标
    av_dict_set_int(&options, "draw_mouse", 0, AV_DICT_MATCH_CASE);
    //设置开始录制的坐标偏移
    av_dict_set_int(&options, "offset_x", 0, AV_DICT_MATCH_CASE);
    av_dict_set_int(&options, "offset_y", 0, AV_DICT_MATCH_CASE);
    //设置录制的分辨率
    av_dict_set(&options, "vedio_size", "1920*1080", 1);

    //创建输入format
    m_pInputFormat = const_cast<AVInputFormat*>(av_find_input_format("gdigrab"));
    if(!m_pInputFormat)
    {
        qDebug() << "av_find_input_format fail";
        return false;
    }

    //创建format context
    m_pFormatContext = avformat_alloc_context();
    if(avformat_open_input(&m_pFormatContext, "desktop", m_pInputFormat, &options) != 0)
    {
        qDebug() << "avformat_open_input fail";
        return false;
    }

    //查询流信息
    if(avformat_find_stream_info(m_pFormatContext, nullptr) < 0)
    {
        qDebug() << "avformat_find_stream_info fail";
        return false;
    }

    //找到视频流来采集视频
    int nVideoIndex = -1;
    //遍历流
    for(uint32_t i = 0; i < m_pFormatContext->nb_streams; i++)
    {
        if(m_pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            nVideoIndex = i;
        }
    }
    if(nVideoIndex == -1)
    {
        //没有流
        return false;
    }

    //创建解码器
    AVCodec* pCodec = const_cast<AVCodec*>(avcodec_find_decoder(
        m_pFormatContext->streams[nVideoIndex]->codecpar->codec_id));
    if(!pCodec)
    {
        qDebug() << "avcodec_find_decoder fail";
        return false;
    }

    //创建解码器上下文
    m_pCodecContext = avcodec_alloc_context3(pCodec);
    if(!m_pCodecContext)
    {
        qDebug() << "avcodec_alloc_context3 fail";
        return false;
    }

    //为解码器上下文设置参数
    m_pCodecContext->pix_fmt = AV_PIX_FMT_RGBA;
    //复制解码器上下文
    avcodec_parameters_to_context(m_pCodecContext, m_pFormatContext->streams[nVideoIndex]->codecpar);
    //打开解码器
    if(avcodec_open2(m_pCodecContext, pCodec, NULL) != 0)
    {
        qDebug() << "avcodec_open2 fail";
        return false;
    }

    //初始化成功
    m_nVideoIndex = nVideoIndex;
    m_bIsInitialzed = true;
    //启动线程捕获视频流
    this->start();//因为继承了QThread类，所以可以通过start执行run函数

    return true;
}

bool CGDIScreenScapture::bClose()
{
    if(m_bIsInitialzed)
    {
        StopCapture();
    }

    if(m_pCodecContext)
    {
        avcodec_free_context(&m_pCodecContext);
        m_pCodecContext = nullptr;
    }

    if(m_pFormatContext)
    {
        avformat_close_input(&m_pFormatContext);
        m_pFormatContext = nullptr;
    }

    m_pInputFormat = nullptr;
    m_nVideoIndex = -1;
    m_bIsInitialzed = false;
    m_bStop = true;//停掉线程

    return true;
}

//获取帧数据
bool CGDIScreenScapture::bCaptureFrame(FrameContainer &rgba, quint32 &nWidth, quint32 &nHeight)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if(m_bStop)
    {
        rgba.clear();
    }

    //帧错误
    if(m_pRgbaFrame == nullptr || m_nFrameSize == 0)
    {
        rgba.clear();
        return false;
    }

    if(rgba.capacity() < m_nFrameSize)
    {
        //扩容
        rgba.reserve(m_nFrameSize);
    }

    //拷贝帧数据
    rgba.assign(m_pRgbaFrame.get(), m_pRgbaFrame.get() + m_nFrameSize);
    nWidth = m_nWidth;
    nHeight = m_nHeight;
}

void CGDIScreenScapture::run()
{
    //在线程中，使用gdi获取视频数据
    if(m_bIsInitialzed && !m_bStop)
    {
        while(!m_bStop)
        {
            //获取帧数据，每秒25张
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / m_nFrameRate));
            bGetOneFrame();
        }
    }
}

void CGDIScreenScapture::StopCapture()
{
    if(m_bIsInitialzed)
    {
        m_bStop = true;
        if(this->isRunning())
        {
            this->quit();
            this->wait();
        }
        //清空帧数据
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pRgbaFrame.reset();
        m_nFrameSize = 0;
        m_nWidth = 0;
        m_nHeight = 0;
    }
}

//获取一帧数据
bool CGDIScreenScapture::bGetOneFrame()
{
    if(m_bStop)
    {
        return false;
    }

    //读一帧
    int nRet = av_read_frame(m_pFormatContext, m_pAvPacket.get());
    if(nRet < 0)
    {
        qDebug() << "av_read_frame fail";
        return false;
    }

    //视频流
    if(m_pAvPacket->stream_index == m_nVideoIndex)
    {
        //解码数据
        if(!bDecode(m_pAvFrame.get(), m_pAvPacket.get()))
        {
            qDebug() << "bDecode fail";
            return false;
        }
    }

    av_packet_unref(m_pAvPacket.get());

    return true;
}

bool CGDIScreenScapture::bDecode(AVFrame *pAvFrame, AVPacket *pAvPacket)
{
    int nRet = avcodec_send_packet(m_pCodecContext, pAvPacket);
    if(nRet < 0)
    {
        qDebug() << "avcodec_send_packet fail";
        return false;
    }
    else
    {
        //接收帧
        nRet = avcodec_receive_frame(m_pCodecContext, pAvFrame);
        if(nRet == AVERROR(EAGAIN) || nRet == AVERROR_EOF)
        {
            qDebug() << "avcodec_receive_frame: nRet = AVERROR(EAGAIN) || nRet = AVERROR_EOF, fail";
            return false;
        }
        if(nRet < 0)
        {
            qDebug() << "avcodec_receive_frame: nRet = 0, fail";
            return false;
        }

        //接收成功，更新帧
        std::lock_guard<std::mutex> lock(m_mutex);
        m_nFrameSize = av_image_get_buffer_size(
            AV_PIX_FMT_RGBA,
            pAvFrame->width,
            pAvFrame->height,
            1);;
        m_pRgbaFrame.reset(new uint8_t[m_nFrameSize], std::default_delete<uint8_t[]>());
        m_nWidth = pAvFrame->width;
        m_nHeight = pAvFrame->height;
        //将frame数据拷贝出去
        for(uint32_t i = 0; i < m_nHeight; i++)
        {
            memcpy(m_pRgbaFrame.get() + i * m_nWidth * 4,
                   pAvFrame->data[0] + i * pAvFrame->linesize[0],
                   pAvFrame->linesize[0]);
        }
        av_frame_unref(pAvFrame);
    }
}

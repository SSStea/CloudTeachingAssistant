#include "AVPlayer.h"

Q_DECLARE_METATYPE(AVFramePtr)

CAVPlayer::~CAVPlayer()
{
    Close();
}

CAVPlayer::CAVPlayer(QWidget *parent)
    :COpenGLRender(parent)
{
    //设置窗口属性
    this->resize(parentWidget()->size());
    //设置无边框
    this->setWindowFlags(Qt::FramelessWindowHint);
    //背景颜色
    this->setAttribute(Qt::WA_StyledBackground);
    Init();
}

void CAVPlayer::Init()
{
    //准备一个音频上下文
    m_pAvContext = new AVContext();
    //创建解封装器
    m_pAVDEMuxer.reset(new CAVDEMuxer(m_pAvContext));
    //初始化音频播放器
    this->bInitAudio(2, 44100, 16);
    //绑定信号与槽 去播放视频
    connect(this, &CAVPlayer::sig_repaint, this, &COpenGLRender::Repaint, Qt::QueuedConnection);
}

void CAVPlayer::Open(const QString &strStreamAddr)
{
    //拉流
    if(m_pAVDEMuxer->bOpen(strStreamAddr.toStdString()))
    {
        //开始视频播放线程
        m_pVideoThread.reset(new std::thread([this](){
            this->VideoPlay();
        }));
        //音频播放线程
        m_pAudioThread.reset(new std::thread([this](){
            this->AudioPlay();
        }));
    }
}

void CAVPlayer::AudioPlay()
{
    //音频播放，从音频帧队列取PCM数据播放
    AVFramePtr frame = nullptr;

    while(!m_bStop && m_pAVDEMuxer && m_pAvContext)
    {
        //判断音频播放大小
        if(nAvailableBytes() < 0 || m_pAvContext->qAudioQueue.bEmpty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        //pop一个帧出来播放
        m_pAvContext->qAudioQueue.bPop(frame);
        Write(frame);
    }
}

void CAVPlayer::VideoPlay()
{
    AVFramePtr frame = nullptr;

    while(!m_bStop && m_pAVDEMuxer && m_pAvContext)
    {
        //判断视频帧队列
        if(m_pAvContext->qVideoQueue.bEmpty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        //pop一个帧出来播放
        m_pAvContext->qVideoQueue.bPop(frame);
        sig_repaint(frame);
    }
}

void CAVPlayer::resizeEvent(QResizeEvent *event)
{
    COpenGLRender::resizeEvent(event);
}

void CAVPlayer::Close()
{
    m_bStop = true;
    if(m_pAudioThread->joinable())
    {
        m_pAudioThread->join();
        m_pAudioThread.reset();
        m_pAudioThread = nullptr;
    }
    if(m_pVideoThread->joinable())
    {
        m_pVideoThread->join();
        m_pVideoThread.reset();
        m_pVideoThread = nullptr;
    }

    if(m_pAVDEMuxer)
    {
        m_pAVDEMuxer.reset();
        m_pAVDEMuxer = nullptr;
    }
}

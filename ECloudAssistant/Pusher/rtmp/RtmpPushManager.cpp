#include "RtmpPushManager.h"
#include "GDIScreenScapture.h"
#include "AACEncoder.h"
#include "H264Encoder.h"
#include "AudioCapture.h"
#include <QDebug>
#include "H264Paraser.h"

CRtmpPushManager::~CRtmpPushManager()
{
    Close();
}

CRtmpPushManager::CRtmpPushManager()
    :m_pLoop(nullptr), m_pAacEncoder(nullptr), m_pH264Encoder(nullptr),
    m_pPusher(nullptr), m_pAudioCapture(nullptr), m_pScreenCapture(nullptr)
{

}

bool CRtmpPushManager::bOpen(const QString &str)
{
    if(!bInit())
    {
        qDebug() << "not init";
        return false;
    }

    //通过推流器打开url
    if(m_pPusher->nOpenUrl(str.toStdString(), 1000) < 0)
    {
        qDebug() << "m_pPusher->nOpenUrl fail";
        return false;
    }

    m_bIsConnect = true;

    //开始采集视频
    m_pVideoCaptureThread.reset(new std::thread([this](){
        this->EncodeVideo();
    }));
    //开始采集音频
    m_pAudioCaptureThread.reset(new std::thread([this](){
        this->EncodeAudio();
    }));

    return true;
}

bool CRtmpPushManager::bInit()
{
    //初始化
    if(!m_pLoop)
    {
        m_pLoop.reset(new CEventLoop(2));
    }

    //创建一个推流器
    m_pPusher = CRtmpPublisher::pCreate(m_pLoop.get());
    //设置块大小
    m_pPusher->SetChunkSize(60000);

    //创建视频采集
    m_pScreenCapture.reset(new CGDIScreenScapture());//采集器像素要跟编码器像素初始化一致
    if(!m_pScreenCapture->bInit())
    {
        qDebug() << "m_pScreenCapture->bInit() fail";
        return false;
    }

    //视频编码
    m_pH264Encoder.reset(new CH264Encoder());
    if(!m_pH264Encoder->bOPen(1920, 1080, 30, 80000, AV_PIX_FMT_BGRA))
    {
        qDebug() << "m_pH264Encoder->bOPen fail";
        return false;
    }

    //音频采集
    m_pAudioCapture.reset(new CAudioCapture());
    if(!m_pAudioCapture->bInit())
    {
        qDebug() << "m_pAudioCapture->bInit() fail";
        return false;
    }

    //音频编码
    m_pAacEncoder.reset(new CAACEncoder());
    if(!m_pAacEncoder->bOpen(m_pAudioCapture->nGetSampleRate(), m_pAudioCapture->nGetChannels(),
                             AV_SAMPLE_FMT_S16, 64))
    {
        qDebug() << "m_pAacEncoder->bOpen fail";
        return false;
    }

    //获取音频、视频编码参数
    RtmpMediaInfo mediaInfo;
    uint8_t nExtraData[1024] = {0};
    int nExtraDataSize = 0;

    //获取h264编码参数
    nExtraDataSize = m_pH264Encoder->nGetSequenceParams(nExtraData, 1024);
    if(nExtraDataSize < 0)
    {
        qDebug() << "m_pH264Encoder->nGetSequenceParams fail";
        return false;
    }
    //获取sps，pps
    CH264Paraser::Nal sps = CH264Paraser::findNal(nExtraData, nExtraDataSize);
    if(sps.first != nullptr && sps.second != nullptr && (*sps.first & 0x1f) == 7)//sps数据
    {
        mediaInfo.m_nSpsSize = sps.second - sps.first + 1;
        mediaInfo.m_pSps.reset(new uint8_t[mediaInfo.m_nSpsSize], std::default_delete<uint8_t[]>());
        memcpy(mediaInfo.m_pSps.get(), sps.first, mediaInfo.m_nSpsSize);

        CH264Paraser::Nal pps = CH264Paraser::findNal(sps.second, nExtraDataSize - (sps.second - (uint8_t*)nExtraData));
        if(pps.first != nullptr && pps.second != nullptr && (*pps.first & 0x1f) == 8)
        {
            mediaInfo.m_nPpsSize = pps.second - pps.first + 1;
            mediaInfo.m_pPps.reset(new uint8_t[mediaInfo.m_nPpsSize], std::default_delete<uint8_t[]>());
            memcpy(mediaInfo.m_pPps.get(), pps.first, mediaInfo.m_nPpsSize);
        }
    }

    //添加音频参数
    uint32_t nAudioExtraSize = m_pAacEncoder->nGetSpecificConfig(nExtraData, 1024);

    mediaInfo.m_nAudioSpecificConfigSize = nAudioExtraSize;
    mediaInfo.m_pAudioSpecificConfig.reset(new uint8_t[mediaInfo.m_nAudioSpecificConfigSize], std::default_delete<uint8_t[]>());
    //拷贝数据
    memcpy(mediaInfo.m_pAudioSpecificConfig.get(), nExtraData, nAudioExtraSize);

    //发送编码参数
    m_pPusher->nSetMediaInfo(mediaInfo);

    return true;
}

void CRtmpPushManager::Close()
{
    //释放资源
    m_bExit = true;
    m_bIsConnect = false;

    if(m_pPusher && m_pPusher->bIsConnected())
    {
        m_pPusher->Close();
        m_pPusher.reset();
        m_pPusher = nullptr;
    }

    StopEncoder();
    StopCapture();
}

void CRtmpPushManager::EncodeVideo()
{
    //控制发送速率，每秒30张
    static CTimeStamp timeStamp;
    uint32_t nFrameRate = 30;
    while(!m_bExit && m_bIsConnect)
    {
        uint32_t nElapsed = timeStamp.nElapsed();
        //获取延迟时间
        uint32_t nDelay = nFrameRate;
        if(nElapsed > nDelay)
        {
            //重置延迟
            nDelay = 0;
        }
        else
        {
            nDelay -= nElapsed;
        }
        //休眠延迟时间
        std::this_thread::sleep_for(std::chrono::milliseconds(nDelay));
        //重新获取时间
        timeStamp.Reset();

        FrameContainer bgraIamge;
        uint32_t nWidth = 0, nHeight = 0;

        //采集
        if(m_pScreenCapture && m_pH264Encoder && m_pPusher)
        {
            if(m_pScreenCapture->bCaptureFrame(bgraIamge, nWidth, nHeight))
            {
                //编码数据
                FrameContainer outFrame;
                if(m_pH264Encoder->nEncode(&bgraIamge[0], nWidth, nHeight, bgraIamge.size(), outFrame) > 0)
                {
                    //编码后推送
                    if(outFrame.size() > 0)
                    {
                        PushVideo(&outFrame[0], outFrame.size());
                    }
                }
            }
        }
    }
}

void CRtmpPushManager::EncodeAudio()
{
    //准备buffer，存放音频数据
    std::shared_ptr<uint8_t> pPcmBuffer(new uint8_t[48000 * 8], std::default_delete<uint8_t[]>());

    //获取样本数
    uint32_t nFrameSamples = m_pAacEncoder->nGetFrames();

    while(!m_bExit && m_bIsConnect)
    {
        if(m_pAudioCapture->nGetSamples() >= (int)nFrameSamples)
        {
            if(m_pAudioCapture->nRead(pPcmBuffer.get(), nFrameSamples) != nFrameSamples)
            {
                continue;//数据不全
            }
            //编码aac
            AVPacketPtr ptr = m_pAacEncoder->pEncode(pPcmBuffer.get(), nFrameSamples);
            if(ptr)
            {
                //推送
                PushAudio(ptr->data, ptr->size);
            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void CRtmpPushManager::StopEncoder()
{
    if(m_pAudioCaptureThread)
    {
        //结束
        m_pAudioCaptureThread->join();
        m_pAudioCaptureThread.reset();
        m_pAudioCaptureThread = nullptr;
    }

    if(m_pVideoCaptureThread)
    {
        m_pVideoCaptureThread->join();
        m_pVideoCaptureThread.reset();
        m_pVideoCaptureThread = nullptr;
    }

    if(m_pH264Encoder)
    {
        m_pH264Encoder->Close();
        m_pH264Encoder.reset();
        m_pH264Encoder = nullptr;
    }

    if(m_pAacEncoder)
    {
        m_pAacEncoder->Close();
        m_pAacEncoder.reset();
        m_pAacEncoder = nullptr;
    }
}

void CRtmpPushManager::StopCapture()
{
    if(m_pAudioCapture)
    {
        m_pAudioCapture->Close();
        m_pAudioCapture.reset();
        m_pAudioCapture = nullptr;
    }

    if(m_pScreenCapture)
    {
        m_pScreenCapture->bClose();
        m_pScreenCapture.reset();
        m_pScreenCapture = nullptr;
    }
}

bool CRtmpPushManager::bIsKeyFrame(const uint8_t *pData, uint32_t nSize)
{
    int nStartCodeSize = 0;
    if (pData[0] == 0 && pData[1] == 0 && pData[2] == 0)
    {
        nStartCodeSize = 3;
    }
    else if (pData[0] == 0 && pData[1] == 0 && pData[2] == 0 && pData[3] == 0)
    {
        nStartCodeSize = 4;
    }

    int nType = pData[nStartCodeSize] & 0x1f;
    return nType == 5 || nType == 7;
}

void CRtmpPushManager::PushVideo(const quint8 *pData, quint32 nSize)
{
    //准备buffer，size = video size - 4
    std::shared_ptr<uint8_t> pFrame(new uint8_t[nSize - 4], std::default_delete<uint8_t[]>());

    //拷贝数据
    memcpy(pFrame.get(), pData + 4, nSize - 4);
    if(nSize > 0)
    {
        if(m_pPusher && m_pPusher->bIsConnected())
        {
            m_pPusher->nPushVideoFrame(pFrame.get(), nSize - 4);
        }
    }
}

void CRtmpPushManager::PushAudio(const quint8 *pData, quint32 nSize)
{
    std::shared_ptr<uint8_t> pFrame(new uint8_t[nSize], std::default_delete<uint8_t[]>());

    //拷贝数据
    memcpy(pFrame.get(), pData, nSize);
    if(nSize > 0)
    {
        if(m_pPusher && m_pPusher->bIsConnected())
        {
            m_pPusher->nPushAudioFrame(pFrame.get(), nSize);
        }
    }
}

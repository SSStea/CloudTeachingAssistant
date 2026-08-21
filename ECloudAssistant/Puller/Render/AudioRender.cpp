#include "AudioRender.h"
#include <QDebug>

CAudioRender::CAudioRender()
{
    m_audioFmt.setCodec("audio/pcm");
    m_audioFmt.setByteOrder(QAudioFormat::LittleEndian);
    m_audioFmt.setSampleType(QAudioFormat::SignedInt);
}

CAudioRender::~CAudioRender()
{

}

int CAudioRender::nAvailableBytes()
{
    //获取pcm大小
    if(!m_pAudioOut)
    {
        qDebug() << "no m_pAudioOut";
        return -1;
    }

    //剩余空间 - 每次周期需要填充的字节数
    return m_pAudioOut->bytesFree() - m_pAudioOut->periodSize();
}

bool CAudioRender::bInitAudio(int nChannels, int nSampleRate, int nSampleSize)
{
    //初始化音频输出
    if(m_pAudioOut || m_bIsInitialzed || m_pDevice)
    {
        qDebug() << "m_pAudioOut || m_bIsInitialzed || m_pDevice";
        return true;
    }

    m_nSampleSize = nSampleSize;
    //设置格式
    m_audioFmt.setChannelCount(nChannels);
    m_audioFmt.setSampleRate(nSampleRate);
    m_audioFmt.setSampleSize(nSampleSize);

    //创建输出
    m_pAudioOut = new QAudioOutput(m_audioFmt);
    //设置缓冲区大小
    m_pAudioOut->setBufferSize(409600);
    //设置音量
    m_pAudioOut->setVolume(m_nVolume);
    //创建接入设备
    m_pDevice = m_pAudioOut->start();

    m_bIsInitialzed = true;
    return true;
}

void CAudioRender::Write(AVFramePtr pFrame)
{
    //播放音频
    if(m_pDevice && m_pAudioOut)
    {
        //获取frame数据
        QByteArray audioData(reinterpret_cast<char*>(pFrame->data[0]),
                             (pFrame->nb_samples * pFrame->ch_layout.nb_channels) *(m_nSampleSize / 8));

        //开始播放
        m_pDevice->write(audioData.data(), audioData.size());
        //释放frame
        av_frame_unref(pFrame.get());
    }
}

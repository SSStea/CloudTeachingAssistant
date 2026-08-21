#ifndef AUDIO_RENDER_H
#define AUDIO_RENDER_H
#include <QAudioOutput>
#include <QAudioFormat>
#include "AVCommon.h"

class CAudioRender
{
public:
    CAudioRender();
    ~CAudioRender();
    inline bool bIsInit(){return m_bIsInitialzed;}
    //获取缓冲区pcm大小
    int nAvailableBytes();
    bool bInitAudio(int nChannels,int nSampleRate,int nSampleSize);
    //播放音频
    void Write(AVFramePtr pFrame);
private:
    bool m_bIsInitialzed = false;
    int  m_nSampleSize = -1;
    int  m_nVolume = 50; //音量值
    //接入设备
    QAudioFormat  m_audioFmt;
    QIODevice*    m_pDevice = nullptr;
    QAudioOutput* m_pAudioOut = nullptr;
};

#endif

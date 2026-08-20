#ifndef RTMPPUSHMANAGER_H
#define RTMPPUSHMANAGER_H
#include <thread>
#include <memory>
#include "RtmpPublisher.h"
#include "H264Encoder.h"
#include <QObject>

class CAACEncoder;
class CAudioCapture;
class CGDIScreenScapture;
class CRtmpPushManager : public QObject
{
    Q_OBJECT
public:
    virtual ~CRtmpPushManager();
    CRtmpPushManager();
public:
    bool bOpen(const QString& str);
    bool bIsClose(){return m_bIsConnect == false;}
protected:
    bool bInit();
    void Close();
    void EncodeVideo();
    void EncodeAudio();
    void StopEncoder();
    void StopCapture();
    bool bIsKeyFrame(const uint8_t* pData, uint32_t nSize);
    void PushVideo(const quint8* pData, quint32 nSize);
    void PushAudio(const quint8* pData, quint32 nSize);
private:
    bool m_bExit = false;
    bool m_bIsConnect = false;
    std::unique_ptr<CEventLoop> m_pLoop;
    std::unique_ptr<CAACEncoder>  m_pAacEncoder;
    std::unique_ptr<CH264Encoder> m_pH264Encoder;
    std::shared_ptr<CRtmpPublisher> m_pPusher;
    std::unique_ptr<CAudioCapture> m_pAudioCapture;
    std::unique_ptr<CGDIScreenScapture> m_pScreenCapture;
    std::unique_ptr<std::thread>  m_pAudioCaptureThread = nullptr;
    std::unique_ptr<std::thread>  m_pVideoCaptureThread = nullptr;
};

#endif // RTMPPUSHMANAGER_H

#ifndef GDISCREENSCAPTURE_H
#define GDISCREENSCAPTURE_H
#include <QThread>
#include <memory>
#include <mutex>
struct AVFrame;
struct AVPacket;
struct AVInputFormat;
struct AVCodecContext;
struct AVFormatContext;
using FrameContainer = std::vector<quint8>;

class CGDIScreenScapture : public QThread
{
public:
    CGDIScreenScapture();
    CGDIScreenScapture(const CGDIScreenScapture&) = delete;
    CGDIScreenScapture& operator=(const CGDIScreenScapture&) = delete;
    virtual ~CGDIScreenScapture();
public:
    virtual quint32 nGetWidth() const;
    virtual quint32 nGetHeight() const;
    virtual bool bInit(qint64 nDisplayIndex = 0);
    virtual bool bClose();
    virtual bool bCaptureFrame(FrameContainer& rgba,quint32& nWidth,quint32& nHeight);
protected:
    virtual void run() override;
private:
    void StopCapture();
    bool bGetOneFrame();
    bool bDecode(AVFrame* pAvFrame,AVPacket* pAvPacket);
private:
    using framPtr = std::shared_ptr<quint8>;
    bool    m_bStop;
    bool    m_bIsInitialzed;
    quint32 m_nFrameSize;
    framPtr m_pRgbaFrame;
    quint32 m_nWidth;
    quint32 m_nHeight;
    qint64  m_nVideoIndex;
    qint64  m_nFrameRate;
    std::mutex m_mutex;
    AVInputFormat* m_pInputFormat;
    AVCodecContext* m_pCodecContext;
    AVFormatContext* m_pFormatContext;
    std::shared_ptr<AVFrame> m_pAvFrame;
    std::shared_ptr<AVPacket> m_pAvPacket;
};

#endif // GDISCREENSCAPTURE_H

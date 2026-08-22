#ifndef AVPLAYER_H
#define AVPLAYER_H
#include "OpenGLRender.h"
#include "AudioRender.h"
#include "AVDEMuxer.h"

class CAVPlayer : public COpenGLRender, public CAudioRender
{
    Q_OBJECT
public:
    ~CAVPlayer();
    explicit CAVPlayer(QWidget* parent = nullptr);

    void Open(const QString& strStreamAddr);

signals:
    void sig_repaint(AVFramePtr frame);

protected:
    void AudioPlay();
    void VideoPlay();
    void Init();
    virtual void resizeEvent(QResizeEvent* event) override;
    void Close();

private:
    bool m_bStop = false;
    AVContext* m_pAvContext = nullptr;
    std::unique_ptr<CAVDEMuxer> m_pAVDEMuxer = nullptr;
    std::unique_ptr<std::thread> m_pAudioThread = nullptr;
    std::unique_ptr<std::thread> m_pVideoThread = nullptr;
};


#endif // AVPLAYER_H

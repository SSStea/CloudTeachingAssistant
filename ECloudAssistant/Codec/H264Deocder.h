#ifndef H264DEOCDER_H
#define H264DEOCDER_H
#include <QThread>
#include "AVCommon.h"

class CVideoConvert;
class CH264Deocder : public QThread ,public CDecodBase
{
    Q_OBJECT
public:
    CH264Deocder(AVContext* ac,QObject* parent = nullptr);
    ~CH264Deocder();
    int  nOpen(const AVCodecParameters* pCodecParamer);
    inline bool bIsFull(){return m_qVideoQueue.nSize() > 10;}
    inline void PutPacket(const AVPacketPtr packet){m_qVideoQueue.push(packet);}
protected:
    void Close();
    virtual void run()override;
private:
    bool m_bQuit = false;
    AVFramePtr m_pYuvFrame = nullptr;
    CAVQueue<AVPacketPtr> m_qVideoQueue;
    AVContext*    m_pAvContext = nullptr;
    std::unique_ptr<CVideoConvert> m_pVideoConver;
};


#endif // H264DEOCDER_H

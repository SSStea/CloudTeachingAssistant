#ifndef AACDECODER_H
#define AACDECODER_H
#include <QThread>
#include "AVCommon.h"

class CAudioResampler;
class CAACDecoder : public QThread ,public CDecodBase
{
    Q_OBJECT
public:
    CAACDecoder(AVContext* ac,QObject* parent = nullptr);
    CAACDecoder(const CAACDecoder&) = delete;
    CAACDecoder& operator=(const CAACDecoder&) = delete;
    ~CAACDecoder();
    int  nOpen(const AVCodecParameters* pCodecParamer);
    inline void PutPacket(const AVPacketPtr packet){m_pAudioQueue.push(packet);}
    inline bool bOsFull(){return m_pAudioQueue.nSize() > 50;}
protected:
    void Close();
    virtual void run()override;
private:
    bool            m_bQuit = false;
    CAVQueue<AVPacketPtr>   m_pAudioQueue;
    AVContext*      m_pAvContext = nullptr;
    std::unique_ptr<CAudioResampler> m_pAudioResampler;
};

#endif // AACDECODER_H

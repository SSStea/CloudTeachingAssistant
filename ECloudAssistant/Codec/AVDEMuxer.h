#ifndef AVDEMUXER_H
#define AVDEMUXER_H
#include <functional>
#include "AVCommon.h"
#include <thread>

class CAVDEMuxer
{
public:
    CAVDEMuxer(AVContext* ac);
    ~CAVDEMuxer();
    bool bOpen(const std::string& strPath);
    using StreamCallBack = std::function<void(bool)>;
    inline void SetStreamCallBack(const StreamCallBack& cb){m_streamCb = cb;}
protected:
    void Close();
    void FetchStream(const std::string& strPath);
    bool bFetchStreamInfo(const std::string& strPath);
    double dAudioDuration();
    double dVideoDuration();
    static int nInterruptFouction(void* arg);
private:
    int m_nVideoIndex = -1;
    int m_nAudioIndex = -1;
    AVContext* m_pAvContext;
    AVDictionary* m_pAvDict;
    std::atomic_bool m_bQuit = false;
    StreamCallBack m_streamCb = [](bool){};
    AVFormatContext* m_pFormateCtx = nullptr;
    std::unique_ptr<std::thread> m_pReadthread;
    //解码器
    // std::unique_ptr<AAC_Decoder> aacDecoder_;
    // std::unique_ptr<H264_Decoder> h264Decoder_;
};

#endif // AVDEMUXER_H

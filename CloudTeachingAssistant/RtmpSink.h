#pragma once
#include <cstdint>
#include <memory>
#include "Amf.h"

class CRtmpSink
{
public:
    CRtmpSink() {}
    virtual ~CRtmpSink() {}

    //发送消息，通知后面的数据为元数据
    virtual bool bSendMetaData(mapAmfObjects metaData) { return true; }
    virtual bool bSendMediaData(uint8_t nType, uint64_t nTimeStamp, std::shared_ptr<char> pPlayload, uint32_t nPlayloadSize) = 0;

    virtual bool bIsPlayer() { return false; }
    virtual bool bIsPublisher() { return false; }
    virtual bool bIsPlaying() { return false; }
    virtual bool bIsPublishing() { return false; }

    //客户端id
    virtual uint32_t nGetId() = 0;

};


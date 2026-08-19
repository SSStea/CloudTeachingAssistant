#pragma once

#include "ReactorBase.h"
#include "TcpSocket.h"

#include <mutex>
#include <unordered_map>

class CReactorSelect : public CReactorBase
{
public:
	CReactorSelect(int nId = 0);
	virtual ~CReactorSelect();

    virtual void UpdateChannel(ChannelPtr pChannel) override;//新增或更新 Socket 监听事件
    virtual void RemoveChannel(ChannelPtr& pChannel) override;//从监听集合移除 Socket
    virtual bool bHandleEvent() override;//调用 select()，把读、写、异常事件分发给 CChannel

private:
    fd_set m_fdReadBackup;//读事件集合
    fd_set m_fdWriteBackup;//写事件集合
    fd_set m_fdExceptBackup;//异常事件集合
    SOCKET m_socketMax = 0;//当前最大 Socket

    bool m_bIsReadReset = false;//是否需要重建读集合
    bool m_bIsWriteReset = false;//是否需要重建写集合
    bool m_bIsExceptReset = false;//是否需要重建异常集合

    std::mutex m_mutex;//保护 Channel 映射
    std::unordered_map<SOCKET, ChannelPtr> m_mapChannels;//SOCKET → ChannelPtr 映射
};

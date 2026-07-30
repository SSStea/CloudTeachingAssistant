#pragma once
#include <functional>
#include <memory>
#include "Channel.h"
#include "TcpSocket.h"
#include "EventLoop.h"

class CEventLoop;

//创建新连接的回调函数，当有新连接进来，通过该函数将套接字描述符返回给server
typedef std::function<void(int)> newConnectCallback;

class CAcceptor
{
public:
	CAcceptor(CEventLoop* eventLoop);
	~CAcceptor();

	//设置新连接的回调函数
	inline void SetNewConnectCallback(const newConnectCallback& cb) { m_newConnectCb = cb; }
	int nListen(std::string strIP, uint16_t nPort);
	void Close();

private:
	CEventLoop* m_eventLoop = nullptr;
	std::shared_ptr<CTcpSocket> m_tcpSocket;
	ChannelPtr m_channelPtr = nullptr;
	newConnectCallback m_newConnectCb;

	void onAccept();
};


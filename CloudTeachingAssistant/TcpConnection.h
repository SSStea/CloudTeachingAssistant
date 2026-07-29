#pragma once
#include "BufferReader.h"
#include "BufferWriter.h"
#include "Channel.h"
#include "TcpSocket.h"
#include "ReactorBase.h"
#include <unistd.h>

class CTcpConnection : public std::enable_shared_from_this<CTcpConnection>
{
public:
	CTcpConnection(CReactorBase* reactor, int nSockFd);
	virtual ~CTcpConnection();

	using ptr = std::shared_ptr<CTcpConnection>;
	using disConnectCallback = std::function<void(std::shared_ptr<CTcpConnection>)>;
	using closeCallback = std::function<void(std::shared_ptr<CTcpConnection>)>;
	using readCallback = std::function<bool(std::shared_ptr<CTcpConnection>, CBufferReader& buffer)>;

	inline CReactorBase* getReactor() const { return m_reactor; }

	//这里的读是把网络套接字中的数据读进缓冲区里
	inline void setReadCallback(const readCallback& cb) { m_readCb = cb; }
	inline void setCoseCallback(const closeCallback& cb) { m_closeCb = cb; }
	
	inline bool bIsClosed() const { return m_bIsClosed; }

	inline int nGetSocket() const { return m_ptrChannel->GetSocket(); }

	void Send(std::shared_ptr<char> data, uint32_t nSize);
	void Send(const char* data, uint32_t nSize);

	void disConnect();

protected:
	bool m_bIsClosed;
	CReactorBase* m_reactor;
	std::unique_ptr<CBufferReader> m_readBuf;
	std::unique_ptr<CBufferWriter> m_writeBuf;

protected:
	//这里的读操作是把缓冲区里的数据读进实际对象
	virtual void HandleRead();
	virtual void HandleWrite();
	virtual void HandleClose();
	virtual void HandleError();

private:
	std::shared_ptr<CChannel> m_ptrChannel = nullptr;
	disConnectCallback m_disConnectCb;
	closeCallback m_closeCb;
	readCallback m_readCb;

	void Close();
};


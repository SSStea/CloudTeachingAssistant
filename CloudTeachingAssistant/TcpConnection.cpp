#include "TcpConnection.h"

CTcpConnection::CTcpConnection(CReactorBase* reactor, int nSockFd)
	: m_reactor(reactor), m_readBuf(new CBufferReader()),
	m_writeBuf(new CBufferWriter(500)), m_ptrChannel(new CChannel(nSockFd))
{
	m_bIsClosed = false;

	m_ptrChannel->SetReadCallback([this]() {this->HandleRead(); });
	m_ptrChannel->SetWriteCallback([this]() {this->HandleWrite(); });
	m_ptrChannel->SetCloseCallback([this]() {this->HandleClose(); });
	m_ptrChannel->SetErrorCallback([this]() {this->HandleError(); });

	//设置套接字属性
	CSocketUtil::SetNonBlock(nSockFd);
	CSocketUtil::SetSendBufSize(nSockFd, 100 * 1024);
	CSocketUtil::SetKeepAlive(nSockFd);

	//设置通道属性
	m_ptrChannel->EnableReading();

	//设置反应器通道
	m_reactor->UpdateChannel(m_ptrChannel);
}

CTcpConnection::~CTcpConnection()
{
	int nFD = m_ptrChannel->GetSocket();
	if (nFD > 0)
	{
		::close(nFD);
	}
}

void CTcpConnection::Send(std::shared_ptr<char> data, uint32_t nSize)
{
	if (!m_bIsClosed)
	{
		m_writeBuf->bAppend(data, nSize);
		this->HandleWrite();
	}

}

void CTcpConnection::Send(const char* data, uint32_t nSize)
{
	if (!m_bIsClosed)
	{
		m_writeBuf->bAppend(data, nSize);
		this->HandleWrite();
	}
}

void CTcpConnection::disConnect()
{
	this->Close();
}

void CTcpConnection::HandleRead()
{
	if (m_bIsClosed)
	{
		return;
	}

	//从套接字中读数据
	int nReadLen = m_readBuf->nReadFromSocket(m_ptrChannel->GetSocket());
	if (nReadLen < 0)
	{
		this->Close();
		return;
	}

	//如果套接字中读到了数据，并且有读缓存的回调函数
	if (m_readCb)
	{
		//把读缓存的数据通过回调函数读给外部资源
		bool bRet = m_readCb(shared_from_this(), *m_readBuf);
		if (!bRet)
		{
			this->Close();
		}
	}
}

void CTcpConnection::HandleWrite()
{
	if (m_bIsClosed)
	{
		return;
	}

	int nWriteLen = 0;
	bool bEmpty = false;

	do 
	{
		nWriteLen = m_writeBuf->nSend(m_ptrChannel->GetSocket());
		if (nWriteLen < 0)
		{
			this->Close();
			return;
		}

		bEmpty = m_writeBuf->bIsEmpty();
	} while (false);

	if (bEmpty)//如果写缓存中的数据已经发送完
	{
		if (m_ptrChannel->IsWriting())//且这个通道还关心写事件
		{
			m_ptrChannel->DisableWriting();//设为不关心写事件
			m_reactor->UpdateChannel(m_ptrChannel);//更新reactor的通道
		}
	}
	else if (!m_ptrChannel->IsWriting())//如果缓存中的数据没发送完，且不关心写事件
	{
		m_ptrChannel->EnableWriting();//设为关心写事件，因为还没发送完，还得继续
		m_reactor->UpdateChannel(m_ptrChannel);//更新reactor的通道
	}
}

void CTcpConnection::HandleClose()
{
	this->Close();
}

void CTcpConnection::HandleError()
{
	this->Close();
}

void CTcpConnection::Close()
{
	if (!m_bIsClosed)
	{
		m_bIsClosed = true;
		m_reactor->RemoveChannel(m_ptrChannel);
		if (m_closeCb)//如果有关闭的回调
		{
			m_closeCb(shared_from_this());//通过回调传递给外部资源（实际对象）去释放
		}
		if (m_disConnectCb)//如果有断开连接的回调
		{
			m_disConnectCb(shared_from_this());
		}
	}
}
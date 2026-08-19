#include "TcpSocket.h"

void CSocketUtil::SetNonBlock(int sockfd)
{
	unsigned long nMode = 1;
	ioctlsocket(sockfd, FIONBIO, &nMode);
}

void CSocketUtil::SetBlock(int sockfd)
{
	unsigned long nMode = 0;
	ioctlsocket(sockfd, FIONBIO, &nMode);
}

void CSocketUtil::SetReuseAddr(int sockfd)
{
	int nOpt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&nOpt, sizeof(nOpt));
}

void CSocketUtil::SetReusePort(int sockfd)
{
}

void CSocketUtil::SetKeepAlive(int sockfd)
{
	int nOpt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&nOpt, sizeof(nOpt));
}

void CSocketUtil::SetSendBufSize(int sockfd, int nSize)
{
	setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSize, sizeof(nSize));
}

void CSocketUtil::SetRecvBufSize(int sockfd, int nSize)
{
	setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (const char*)&nSize, sizeof(nSize));
}

CTcpSocket::CTcpSocket()
{
}

CTcpSocket::~CTcpSocket()
{
}

int CTcpSocket::nCreate()
{
	sockfd_ = (int)::socket(AF_INET, SOCK_STREAM, 0);
	return sockfd_;
}

bool CTcpSocket::bBind(std::string ip, short port)
{
	if (sockfd_ == -1)
	{
		return false;
	}

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	addr.sin_port = htons(port);

	if(::bind(sockfd_, (const sockaddr*)&addr, sizeof(addr)) == -1)
	{
		return false;
	}

	return true;
}

bool CTcpSocket::bListen(int backlog)
{
	if (sockfd_ == -1)
	{
		return false;
	}

	if (::listen(sockfd_, backlog) == -1)
	{
		return false;
	}

	return true;
}

bool CTcpSocket::bConnect(std::string strIp, uint16_t nPort, int nTimeout)
{
	(void)nTimeout;

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);
	addr.sin_addr.s_addr = inet_addr(strIp.c_str());

	if (::connect(sockfd_, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		return false;
	}

	return true;
}

int CTcpSocket::nAccept()
{
	struct sockaddr_in addr = { 0 };
	int addrLen = sizeof(addr);

	return (int)::accept(sockfd_, (sockaddr*)&addr, &addrLen);
}

void CTcpSocket::Close()
{
	if (sockfd_ != -1)
	{
		::closesocket(sockfd_);
		sockfd_ = -1;
	}
}

void CTcpSocket::ShutdownWrite()
{
	if (sockfd_ != -1)
	{
		shutdown(sockfd_, SD_SEND);
		sockfd_ = -1;
	}
}

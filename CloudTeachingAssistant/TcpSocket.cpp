#include "TcpSocket.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

void CSocketUtil::SetNonBlock(int sockfd)
{
	int nFlags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, nFlags | O_NONBLOCK);
}

void CSocketUtil::SetBlock(int sockfd)
{
	int nFlags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, nFlags | (~O_NONBLOCK));
}

void CSocketUtil::SetReuseAddr(int sockfd)
{
	int nOpt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&nOpt, sizeof(nOpt));
}

void CSocketUtil::SetReusePort(int sockfd)
{
	int nOpt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const void*)&nOpt, sizeof(nOpt));
}

void CSocketUtil::SetKeepAlive(int sockfd)
{
	int nOpt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (const void*)&nOpt, sizeof(nOpt));
}

void CSocketUtil::SetSendBufSize(int sockfd, int nSize)
{
	setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (const void*)&nSize, sizeof(nSize));
}

void CSocketUtil::SetRecvBufSize(int sockfd, int nSize)
{
	setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (const void*)&nSize, sizeof(nSize));
}

CTcpSocket::CTcpSocket()
{
}

CTcpSocket::~CTcpSocket()
{
}

int CTcpSocket::nCreate()
{
	sockfd_ = ::socket(PF_INET, SOCK_STREAM, 0);
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

int CTcpSocket::nAccept()
{
	struct sockaddr_in addr = { 0 };
	socklen_t addrLen = sizeof(addr);

	return ::accept(sockfd_, (sockaddr*)&addr, &addrLen);
}

void CTcpSocket::Close()
{
	if (sockfd_ != -1)
	{
		close(sockfd_);
		sockfd_ = -1;
	}
}

void CTcpSocket::ShutdownWrite()
{
	if (sockfd_ != -1)
	{
		shutdown(sockfd_, SHUT_WR);
		sockfd_ = -1;
	}
}

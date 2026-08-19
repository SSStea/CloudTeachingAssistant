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

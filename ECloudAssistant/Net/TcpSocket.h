#pragma once
#include <string>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#ifdef GetObject
#undef GetObject
#endif

class CSocketUtil
{
public:
	static void SetNonBlock(int sockfd);
	static void SetBlock(int sockfd);
	static void SetReuseAddr(int sockfd);
	static void SetReusePort(int sockfd);
	static void SetKeepAlive(int sockfd);
	static void SetSendBufSize(int sockfd, int nSize);
	static void SetRecvBufSize(int sockfd, int nSize);
};

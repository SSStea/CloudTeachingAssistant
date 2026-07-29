#pragma once
#include <string>

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

class CTcpSocket
{
public:
	CTcpSocket();
	virtual ~CTcpSocket();
	int nCreate();
	bool bBind(std::string ip, short port);
	bool bListen(int backlog);
	int  nAccept();
	void Close();
	void ShutdownWrite();
	int nGetSocket() const { return sockfd_; }
private:
	int sockfd_ = -1;
};


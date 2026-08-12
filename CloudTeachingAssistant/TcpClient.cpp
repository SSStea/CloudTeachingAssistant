#include "TcpClient.h"
#include <iostream>
#include "string.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

CTcpClient::CTcpClient()
	: m_file(nullptr), m_bIsConnect(false), m_nSockfd(-1)
{
}

CTcpClient::~CTcpClient()
{
	Close();
}

void CTcpClient::Create()
{
	//获取内存
	m_file = fopen("/proc/meminfo", "r");
	if (!m_file)
	{
		std::cout << "open file fail" << std::endl;
		return;
	}

	memset(&m_info, 0, sizeof(m_info));

	//初始化info
	sysinfo(&m_info);

	//创建socket
	m_nSockfd = ::socket(PF_INET, SOCK_STREAM, 0);
	m_monitorInfo.SetIp("172.20.108.206");
	m_monitorInfo.nPort = 9867;
}

bool CTcpClient::bConnect(std::string strIP, uint16_t nPort)
{
	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);
	addr.sin_addr.s_addr = inet_addr(strIP.c_str());

	socklen_t nLen = sizeof(sockaddr_in);

	if (::connect(m_nSockfd, (const sockaddr*)&addr, nLen) == -1)
	{
		std::cout << "connect fail" << std::endl;
		return false;
	}
	m_bIsConnect = true;

	return true;
}

void CTcpClient::Close()
{
	m_bIsConnect = false;
	fclose(m_file);
	if (m_nSockfd)
	{
		::close(m_nSockfd);
	}
}

void CTcpClient::GetMonitorInfo()
{
	GetMemUsage();
}

void CTcpClient::GetMemUsage()
{
	//获取内存再去发送
	size_t nBytesUsed = 0;
	ssize_t nRead = 0;
	char* pLine = nullptr;
	int nIndex = 0;
	unsigned long long nAvailableMemKb = 0;

	while ((nRead = getline(&pLine, &nBytesUsed, m_file)) != -1)
	{
		if (++nIndex <= 2)
		{
			continue;
		}

		if (strstr(pLine, "MemAvailable") != nullptr)
		{
			sscanf(pLine, "%*s%llu%*s", &nAvailableMemKb);
			break;
		}
	}

	double dTotalMemKb = ((double)m_info.totalram) / 1024.0;
	double dMemUsage = (dTotalMemKb - (double)nAvailableMemKb) * 100 / dTotalMemKb;
	m_monitorInfo.nMem = (uint8_t)dMemUsage;

	Send((uint8_t*)&m_monitorInfo, (size_t)m_monitorInfo.nLen);
}

void CTcpClient::Send(uint8_t* pData, size_t nSize)
{
	ssize_t nLen = 0;
	size_t nIndex = 0;

	while (nIndex != nSize)
	{
		nLen = ::send(m_nSockfd, pData + nIndex, nSize - nIndex, 0);
		if (nLen < 0)
		{
			break;
		}

		nIndex += (size_t)nLen;
	}
}

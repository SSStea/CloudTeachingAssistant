#pragma once
#include "logindefine.h"

class CTcpClient
{
public:
	CTcpClient();
	~CTcpClient();
	void Create();
	bool bConnect(std::string strIP, uint16_t nPort);
	void Close();
	void GetMonitorInfo();
protected:
	void GetMemUsage();
	void Send(uint8_t* pData, size_t nSize);
private:
	FILE* m_file;
	bool m_bIsConnect;
	int m_nSockfd;
	struct sysinfo m_info;
	MonitorBody m_monitorInfo;
};

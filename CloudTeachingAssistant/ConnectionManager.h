#pragma once
#include <map>
#include <memory>
#include <mutex>
#include "TcpConnection.h"

class CConnectionManager
{
public:
	~CConnectionManager();
	static CConnectionManager* GetInstance();

	void AddConn(const std::string& strIdefy, const CTcpConnection::ptr conn);
	void RemoveConn(const std::string& strIdefy);
	CTcpConnection::ptr QueryConn(const std::string& strIdefy);
	uint32_t nSize() const { return (uint32_t)m_mapConnMaps.size(); };

private:
	CConnectionManager();

	void Close();

	std::mutex m_mutex;
	static std::unique_ptr<CConnectionManager> m_instance;
	std::unordered_map<std::string, CTcpConnection::ptr> m_mapConnMaps;
};


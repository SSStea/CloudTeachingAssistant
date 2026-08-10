#include "ConnectionManager.h"

std::unique_ptr<CConnectionManager> CConnectionManager::m_instance = nullptr;

CConnectionManager::CConnectionManager()
{

}

CConnectionManager::~CConnectionManager()
{
	Close();
}

CConnectionManager* CConnectionManager::GetInstance()
{
	static std::once_flag flag;
	std::call_once(flag, [&]() {
		m_instance.reset(new CConnectionManager()); //只会获取/创建一次
		});
	return m_instance.get();
}

void CConnectionManager::AddConn(const std::string& strIdefy, const CTcpConnection::ptr conn)
{
	if (strIdefy.empty())
	{
		return;
	}

	std::lock_guard<std::mutex> lock(m_mutex);
	
	auto it = m_mapConnMaps.find(strIdefy);
	if (it == m_mapConnMaps.end())//说明连接器未加入
	{
		m_mapConnMaps.emplace(strIdefy, conn);
	}
}

void CConnectionManager::RemoveConn(const std::string& strIdefy)
{
	if (strIdefy.empty())
	{
		return;
	}

	std::lock_guard<std::mutex> lock(m_mutex);

	auto it = m_mapConnMaps.find(strIdefy);
	if (it != m_mapConnMaps.end())//说明连接器未加入
	{
		m_mapConnMaps.erase(strIdefy);
	}
}

CTcpConnection::ptr CConnectionManager::QueryConn(const std::string& strIdefy)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	auto it = m_mapConnMaps.find(strIdefy);
	if (it != m_mapConnMaps.end())//说明连接器未加入
	{
		return it->second;
	}

	return nullptr;
}

void CConnectionManager::Close()
{
	m_mapConnMaps.clear();
}

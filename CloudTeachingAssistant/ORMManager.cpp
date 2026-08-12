#include "ORMManager.h"
#include <iostream>
#include <chrono>

std::unique_ptr<CORMManager> CORMManager::m_instance = nullptr;

CORMManager::CORMManager()
{
	//初始化mysql
	mysql_init(&m_mysql);

	//连接数据库
	if (mysql_real_connect(&m_mysql, "172.20.108.206", "root",
		"123456", "users", 3306, NULL, 0) == NULL)//失败
	{
		std::cout << "connect server sql database fail: "<< mysql_error(&m_mysql) << std::endl;
		return;
	}
	std::cout << "connect sql database success" << std::endl;
}

CORMManager::~CORMManager()
{

}

CORMManager* CORMManager::GetInstance()
{
	static std::once_flag flag;
	std::call_once(flag, [&]() {
		m_instance.reset(new CORMManager()); //只会获取/创建一次
		});
	return m_instance.get();
}

void CORMManager::UserRegister(const char* pName, const char* pAcount, const char* pPassword, const char* pUserCode, const char* pSigServer)
{
	auto now = std::chrono::system_clock::now();
	auto nowTimestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
	return insertClient(pName, pAcount, pPassword, pUserCode, 0, nowTimestamp, pSigServer);
}

MYSQL_ROW CORMManager::UserLogin(const char* pUserCode)
{
	return selectClientByUsercode(pUserCode);
}

void CORMManager::UserDestroy(const char* pUserCode)
{
	return deleteClientByUsercode(pUserCode);
}

void CORMManager::insertClient(const char* pName, const char* pAcount, const char* pPassword, const char* pUserCode, int nOnline, long lRecentlyLogin, const char* pSigServer)
{
	//新增一个行
	char cQuery[1024];
	sprintf(cQuery, "INSERT INTO clients (USER_NAME, USER_ACOUNT, USER_PASSWD, USER_CODE, USER_ONLINE, USER_RECENTLY_LOGIN, USER_SVR_MOUNT) VALUES ('%s', '%s', '%s', '%s', '%d', '%ld', '%s')",
		pName, pAcount, pPassword, pUserCode, nOnline, lRecentlyLogin, pSigServer);
	if (mysql_query(&m_mysql, cQuery))
	{
		//大于0失败
		std::cout << "insert fail: " << mysql_error(&m_mysql) << std::endl;
		return;
	}
	else
	{
		std::cout << "insert successful" << std::endl;
	}
}

void CORMManager::deleteClientByUsercode(const char* pUserCode)
{
	char cQuery[1024];
	sprintf(cQuery, "DELETE FROM clients WHERE USER_CODE = '%s'", pUserCode);
	if (mysql_query(&m_mysql, cQuery))
	{
		//大于0失败
		std::cout << "delete fail: %s" << mysql_error(&m_mysql) << std::endl;
		return;
	}
	else
	{
		std::cout << "delete successful" << std::endl;
	}
}

MYSQL_ROW CORMManager::selectClientByUsercode(const char* pUserCode)
{
	char cQuery[1024];
	sprintf(cQuery, "SELECT * FROM clients WHERE USER_CODE = '%s'", pUserCode);

	MYSQL_ROW row;
	MYSQL_RES* res;

	if (mysql_query(&m_mysql, cQuery))
	{
		//大于0失败
		std::cout << "select fail: %s" << mysql_error(&m_mysql) << std::endl;
		return NULL;
	}
	else
	{
		//获取查询结果
		res = mysql_store_result(&m_mysql);
		if (res)
		{
			//从结果获取行
			row = mysql_fetch_row(res);
			//释放结果
			mysql_free_result(res);
			std::cout << "select successful" << std::endl;

			return row;
		}
	}
	return NULL;
}

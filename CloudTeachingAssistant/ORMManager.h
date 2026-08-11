#pragma once
#include <memory>
#include <mutex>
#include <mysql/mysql.h>

class CORMManager
{
public:
	~CORMManager();
	static CORMManager* GetInstance(); //全局实例
	void UserRegister(const char* pName, const char* pAcount, const char* pPassword, const char* pUserCode, const char* pSigServer);
	MYSQL_ROW UserLogin(const char* pUserCode);
	void UserDestroy(const char* pUserCode);
	void insertClient(const char* pName, const char* pAcount, const char* pPassword, const char* pUserCode, int nOnline, long lRecentlyLogin, const char* pSigServer);
protected:
	void deleteClientByUsercode(const char* pUserCode);
	MYSQL_ROW selectClientByUsercode(const char* pUserCode);
private:
	CORMManager();
	MYSQL m_mysql;
private:
	static std::unique_ptr<CORMManager> m_instance;
};

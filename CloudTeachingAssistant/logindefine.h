#pragma once
#include <cstdint>
#include <string>
#include <array>
#include <sys/sysinfo.h>
#include <string.h>

#pragma pack(push,1)
enum Cmd : uint16_t
{
	Monitor,
	ERROR,
	Login,
	Register,
	Destory,
};

enum ResultCode
{
	S_OK = 0,
	SERVER_ERROR,
	REQUEST_TIMEOUT,
	ALREADY_REGISTERED,
	USER_DISAPPEAR,
	ALREADY_LOGIN,
	VERFICATE_FAILED
};

struct PacketHead {
	PacketHead()
		:nLen(-1)
		, nCmd(-1) {
	}
	uint16_t nLen;
	uint16_t nCmd;
};

struct UserRegister : public PacketHead
{
	UserRegister() :PacketHead()
	{
		nCmd = Register;
		nLen = sizeof(UserRegister);
	}
	void SetCode(const std::string& str)
	{
		str.copy(arrCode.data(), arrCode.size(), 0);
	}
	std::string strGetCode()
	{
		return std::string(arrCode.data());
	}
	void SetName(const std::string& str)
	{
		str.copy(arrName.data(), arrName.size(), 0);
	}
	std::string strGetName()
	{
		return std::string(arrName.data());
	}
	void SetCount(const std::string& str)
	{
		str.copy(arrCount.data(), arrCount.size(), 0);
	}
	std::string strGetCount()
	{
		return std::string(arrCount.data());
	}
	void SetPasswd(const std::string& str)
	{
		str.copy(arrPasswd.data(), arrPasswd.size(), 0);
	}
	std::string strGetPasswd()
	{
		return std::string(arrPasswd.data());
	}
	std::array<char, 20> arrCode;
	std::array<char, 20> arrName;
	std::array<char, 12> arrCount;
	std::array<char, 20> arrPasswd;
	uint64_t nTimestamp;
};

struct UserLogin : public PacketHead
{
	UserLogin() :PacketHead()
	{
		nCmd = Login;
		nLen = sizeof(UserLogin);
	}
	void SetCode(const std::string& str)
	{
		str.copy(arrCode.data(), arrCode.size(), 0);
	}
	std::string strGetCode()
	{
		return std::string(arrCode.data());
	}
	void SetCount(const std::string& str)
	{
		str.copy(arrCount.data(), arrCount.size(), 0);
	}
	std::string strGetCount()
	{
		return std::string(arrCount.data());
	}
	void SetPasswd(const std::string& str)
	{
		str.copy(arrPasswd.data(), arrPasswd.size(), 0);
	}
	std::string strGetPasswd()
	{
		return std::string(arrPasswd.data());
	}
	std::array<char, 20> arrCode;
	std::array<char, 12> arrCount;
	std::array<char, 33> arrPasswd; //Md5
	uint64_t nTimestamp;
};

struct RegisterResult : public PacketHead
{
	RegisterResult() :PacketHead()
	{
		nCmd = Register;
		nLen = sizeof(RegisterResult);
	}
	ResultCode resultCode;
};

struct LoginResult : public PacketHead
{
	LoginResult() : PacketHead()
	{
		nCmd = Login;
		nLen = sizeof(LoginResult);
	}
	void SetIp(const std::string& str)
	{
		//str.copy(ctrSvrIp.data(),ctrSvrIp.size()+1,0);
		strncpy(arrCtrSvrIp.data(), str.c_str(), arrCtrSvrIp.size() - 1);
		arrCtrSvrIp.back() = '\0'; // 强制最后一个字符为终止符
	}
	std::string strGetIp()
	{
		return std::string(arrCtrSvrIp.data());
	}
	ResultCode resultCode;
	uint16_t nPort;
	std::array<char, 16> arrCtrSvrIp;
};

struct UserDestory : public PacketHead
{
	UserDestory() : PacketHead()
	{
		nCmd = Destory;
		nLen = sizeof(UserDestory);
	}
	void SetCode(const std::string& str)
	{
		str.copy(arrCode.data(), arrCode.size(), 0);
	}
	std::string strGetCode()
	{
		return std::string(arrCode.data());
	}
	std::array<char, 20> arrCode;
};

struct MonitorBody : public PacketHead {
	MonitorBody()
		:PacketHead()
	{
		nCmd = Monitor;
		nLen = sizeof(MonitorBody);
		arrIP.fill('\0');
	}
	void SetIp(const std::string& str)
	{
		str.copy(arrIP.data(), arrIP.size(), 0);
	}
	std::string strGetIp()
	{
		return std::string(arrIP.data());
	}
	uint8_t nMem;
	std::array<char, 16> arrIP;
	uint16_t nPort;
};
#pragma pack(pop)
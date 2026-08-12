#include "LoginConnection.h"
#include <iostream>
#include "ORMManager.h"
#include <chrono>

#define TIMEOUT 60

CLoginConnection::CLoginConnection(CReactorBase* reactor, int nSocket)
	: CTcpConnection(reactor, nSocket)
{
	this->setReadCallback([this](std::shared_ptr<CTcpConnection>, CBufferReader& buffer) {
		return this->bOnRead(buffer);
		});
}

CLoginConnection::~CLoginConnection()
{
	Clear();
}

bool CLoginConnection::bIsTimeout(uint64_t nTimestamp)
{
	//获取当前时间
	auto now = std::chrono::system_clock::now();
	auto nowTimestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

	//计算差值
	uint64_t nTime = nowTimestamp - nTimestamp;

	return nTime > TIMEOUT;
}

bool CLoginConnection::bOnRead(CBufferReader& buffer)
{
	if (buffer.nReadableBytes() > 0)
	{
		HandleMessage(buffer);
	}

	return true;
}

void CLoginConnection::HandleMessage(CBufferReader& buffer)
{
	if (buffer.nReadableBytes() < sizeof(PacketHead))
	{
		return;
	}

	PacketHead* head = (PacketHead*)buffer.pPeek();
	if (buffer.nReadableBytes() < head->nLen)
	{
		return;
	}

	switch (head->nCmd)
	{
	case Login:
		HandleLogin(head);
		break;
	case Register:
		HandleRegister(head);
		break;
	case Destory:
		HandleDestory(head);
		break;
	default:
		std::cout << "cmd error" << std::endl;
		break;
	}

	buffer.Retrieve(head->nLen);
}

void CLoginConnection::Clear()
{

}

void CLoginConnection::HandleRegister(const PacketHead* pData)
{
	//判断用户是否已经注册
	UserRegister* reg = (UserRegister*)pData;
	RegisterResult reply;
	uint64_t nTime = reg->nTimestamp;
	if (bIsTimeout(nTime))
	{
		std::cout << "register timeout" << std::endl;
		reply.resultCode = REQUEST_TIMEOUT;
	}
	else
	{
		//我们需要判断用户是否存在
		std::string strCode = reg->strGetCode();
		//通过数据库来查询code是否存在
		MYSQL_ROW row = CORMManager::GetInstance()->UserLogin(strCode.c_str());
		if (row == NULL)
		{
			CORMManager::GetInstance()->UserRegister(reg->strGetName().c_str(), reg->strGetCount().c_str(),
				reg->strGetPasswd().c_str(), reg->strGetCode().c_str(), "172.20.108.206");
			reply.resultCode = S_OK;
		}
		else
		{
			reply.resultCode = ALREADY_REGISTERED;
		}
	}
	this->Send((const char*)&reply, reply.nLen);
}

void CLoginConnection::HandleLogin(const PacketHead* pData)
{
	UserLogin* login = (UserLogin*)pData;
	LoginResult reply;
	uint64_t nTime = login->nTimestamp;
	if (bIsTimeout(nTime))
	{
		std::cout << "login timeout" << std::endl;
		reply.resultCode = REQUEST_TIMEOUT;
	}
	else
	{
		//我们需要判断用户是否存在
		std::string code = login->strGetCode();
		//通过数据库来查询code是否存在
		MYSQL_ROW row = CORMManager::GetInstance()->UserLogin(code.c_str());
		if (row == NULL) //用户未注册
		{
			reply.resultCode = SERVER_ERROR;
		}
		else
		{
			//判断用户是否一级登录
			if (atoi(row[4])) //在线
			{
				std::cout << "online \n";
				reply.resultCode = ALREADY_LOGIN;
			}
			else
			{
				std::cout << "login \n";
				reply.resultCode = S_OK;
				reply.SetIp("172.20.108.206");
				reply.nPort = 6539;
				//修改记录
				//我们先获取当前用户消息
				//获取当前时间
				auto now = std::chrono::system_clock::now();
				auto nowTimestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
				CORMManager::GetInstance()->insertClient(row[0], row[1], row[2], row[3], 1, nowTimestamp, "172.20.108.206");
			}
		}
	}

	this->Send((const char*)&reply, reply.nLen);
}

void CLoginConnection::HandleDestory(const PacketHead* pData)
{
	UserDestory* destroy = (UserDestory*)pData;

	CORMManager::GetInstance()->UserDestroy(destroy->strGetCode().c_str());
}

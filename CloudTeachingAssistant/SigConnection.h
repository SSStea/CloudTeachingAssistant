#pragma once
#include <map>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <iostream>
#include "define.h"
#include "TcpConnection.h"
#include "ConnectionManager.h"

class CSigConnection : public CTcpConnection
{
public:
	CSigConnection(CReactorBase* reactor, int nSocket);
	~CSigConnection();

	bool bIsAlive();	//是否存活
	bool bIsIdle();		//是否空闲
	bool bIsBusy();		//是否忙碌
	bool bIsNoJoin();	//没有加入房间
	void Disconnected();//断开连接
	void AddClient(const std::string& strCode); //添加客户端
	void RemoveClient(const std::string& strCode);//删除客户端
	RoleState GetRoleState() const;//获取用户状态
	std::string strGetCode() const;//
	std::string strGetStreamAddr() const;//获取流地址

protected:
	bool bOnRead(CBufferReader& buffer); //处理读消息
	void HanldeMessage(CBufferReader& buffer);
	void Clear();

private:
	void HandleJoin(const PacketHead* data);			//处理创建房间
	void HandleObtainStream(const PacketHead* data);	//处理获取流
	void HandleCreateStream(const PacketHead* data);	//处理创建流
	void HandleDeleteStream(const PacketHead* data);	//处理删除流
	void HandleOtherMessage(const PacketHead* data);	//处理其他消息

	void DoObtainStream(const PacketHead* data);		//获取流
	void DoCreateStream(const PacketHead* data);		//创建流

	RoleState m_state;//用户状态
	std::string m_strCode;//用户Code
	std::string m_strStreamAddr;//流地址
	CTcpConnection::ptr m_conn; //连接器
	std::vector<std::string> m_vecObjectes; //客户端数组
};


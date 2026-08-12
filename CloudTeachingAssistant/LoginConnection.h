#pragma once
#include "TcpConnection.h"
#include "logindefine.h"

class CLoginConnection : public CTcpConnection
{
public:
	CLoginConnection(CReactorBase* reactor, int nSocket);
	~CLoginConnection();
protected:
	bool bIsTimeout(uint64_t nTimestamp);
	bool bOnRead(CBufferReader& buffer);
	void HandleMessage(CBufferReader& buffer);
	void Clear();
private:
	void HandleRegister(const PacketHead* pData);
	void HandleLogin(const PacketHead* pData);
	void HandleDestory(const PacketHead* pData);
};

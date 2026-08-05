#pragma once
#include "BufferReader.h"
#include <random>
#include <string.h>
#include <iostream>

class CRtmpHandshake
{
public:
	enum State
	{
		HANDSHAKE_C0C1,
		HANDSHAKE_S0S1S2,
		HANDSHAKE_C2,
		HANDSHAKE_COMPLETE
	};

	CRtmpHandshake(State state);
	virtual ~CRtmpHandshake();

	int nParse(CBufferReader& inBuffer, char* pResBuf, uint32_t nResBufSize);

	bool bHandshakeIsCompleted() const
	{
		return m_handshakeState == HANDSHAKE_COMPLETE;
	}

	//客户端创建C0、C1
	int nBuildC0C1(char* pBuf, uint32_t nBufSize);

private:
	State m_handshakeState;
};


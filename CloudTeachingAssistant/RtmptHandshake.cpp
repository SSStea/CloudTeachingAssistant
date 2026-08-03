#include "RtmptHandshake.h"

CRtmpHandshake::CRtmpHandshake(State state)
{
	m_handshakeState = state;
}

CRtmpHandshake::~CRtmpHandshake()
{
}

int CRtmpHandshake::nParse(CBufferReader& inBuffer, char* pResBuf, uint32_t nResBufSize)
{
	uint8_t* buf = (uint8_t*)inBuffer.pPeek();
	uint32_t nBufSize = inBuffer.nReadableBytes();
	uint32_t nPos = 0;
	uint32_t nResSize = 0;
	std::random_device rd;

	if (m_handshakeState == HANDSHAKE_S0S1S2)//由客户端处理
	{
		if (nBufSize < (1 + 1536 + 1536))
		{
			std::cout << "receive [S0 S1 S2] is not complete" << std::endl;
			return nResSize;
		}

		if (buf[0] != 3)//判断版本
		{
			std::cout << "version is not 3, error" << std::endl;
			return -1;
		}

		nPos += 1 + 1536 + 1536;
		nResSize = 1536; //返回的需要发送的数据（C2）大小

		//准备C2
		memcpy(pResBuf, buf + 1, 1546); //将S1回传
		m_handshakeState = HANDSHAKE_COMPLETE;
	}
	else if (m_handshakeState == HANDSHAKE_C0C1)//由服务端处理
	{
		if (nBufSize < (1 + 1536))//C0 C1
		{
			std::cout << "receive [C0 C1] is not complete" << std::endl;
			return nResSize;
		}

		if (buf[0] != 3)
		{
			std::cout << "version is not 3, error" << std::endl;
			return -1;
		}

		nPos += 1537;
		nResSize = 1 + 1536 + 1536;

		memset(pResBuf, 0, nResSize); //返回S0 S1 S2
		pResBuf[0] = 3; // S1

		char* p = pResBuf;
		p += 9;
		for (int i = 0; i < 1528; i++)
		{
			*p++ = (char)rd();
		}//S1的随机数

		memcpy(p, buf + 1, 1536);//把C1拷贝进S2

		m_handshakeState = HANDSHAKE_C2;
	}
	else if(m_handshakeState == HANDSHAKE_C2)//服务器处理C2
	{
		if (nBufSize < 1536) //C2不完整
		{
			std::cout << "receive [C2] is not complete" << std::endl;
			return nResSize;
		}

		if (buf[0] != 3)
		{
			std::cout << "version is not 3, error" << std::endl;
			return -1;
		}

		nPos += 1536;
		m_handshakeState = HANDSHAKE_COMPLETE;
	}

	inBuffer.Retrieve(nPos);

	return nResSize;
}

int CRtmpHandshake::nBuildC0C1(char* pBuf, uint32_t nBufSize)
{
	uint32_t nSize = 1 + 1536;
	memset(pBuf, 0, nSize);

	pBuf[0] = 3;//版本为3

	std::random_device rd;
	uint8_t* p = (uint8_t*)pBuf;
	p += 9;
	for (int i = 0; i < 1528; i++)
	{
		*p++ = (char)rd();
	}//C1中的随机数

	return nSize;
}

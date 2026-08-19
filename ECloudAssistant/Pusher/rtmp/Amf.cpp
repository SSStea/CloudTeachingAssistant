#include "Amf.h"

int CAmfDecoder::nDecode(const char* pData, int nSize, int n)
{
	int nBytesUsed = 0;


	while (nBytesUsed < nSize)
	{
		int nRet = 0;

		char pType = pData[nBytesUsed];
		nBytesUsed += 1;

		switch (pType)
		{
		case AMF0_NUMBER:
			m_obj.type = AMF_NUMBER;
			nRet = nDecodeNumber(pData + nBytesUsed, nSize - nBytesUsed, m_obj.fAmfNumber);
			break;

		case AMF0_BOOLEAN:
			m_obj.type = AMF_BOOLEAN;
			nRet = nDecodeBoolean(pData + nBytesUsed, nSize - nBytesUsed, m_obj.bAmfBoolean);
			break;

		case AMF0_STRING:
			m_obj.type = AMF_STRING;
			nRet = nDecodeString(pData + nBytesUsed, nSize - nBytesUsed, m_obj.strAmfString);
			break;

		case AMF0_OBJECT:
			nRet = nDecodeObject(pData + nBytesUsed, nSize - nBytesUsed, m_objs);
			break;

		case AMF0_ECMA_ARRAY:
			nRet = nDecodeObject(pData + nBytesUsed + 4, nSize - nBytesUsed - 4, m_objs);

		default:
			break;
		}

		if (nRet < 0)
		{
			break;
		}
		nBytesUsed += nRet;
		n--;
		if (n == 0)
		{
			break;
		}

	}

	return nBytesUsed;
}


int CAmfDecoder::nDecodeBoolean(const char* pData, int nSize, bool& bAmfboolean)
{
	if (nSize < 1) // bool类型要大于1
	{
		std::cout << "bool size is less than 1" << std::endl;
		return -1;
	}

	bAmfboolean = (pData[0] != 0);

	return 1;
}

int CAmfDecoder::nDecodeNumber(const char* pData, int nSize, double& fAmfNumber)
{
	if (nSize < 8)
	{
		std::cout << "double size is less than 8" << std::endl;
		return -1;
	}
	char* pCin = (char*)pData;
	char* pCout = (char*)&fAmfNumber;

	//大小端转换
	pCout[0] = pCin[7];
	pCout[1] = pCin[6];
	pCout[2] = pCin[5];
	pCout[3] = pCin[4];
	pCout[4] = pCin[3];
	pCout[5] = pCin[2];
	pCout[6] = pCin[1];
	pCout[7] = pCin[0];

	return 8;
}

int CAmfDecoder::nDecodeString(const char* pData, int nSize, std::string& strAmfString)
{
	if (nSize < 2)
	{
		std::cout << "string size is less than 2" << std::endl;
		return -1;
	}

	int nByteUsed = 0;

	//获取字符串长度，pData的前两个字节是存储字符串长度
	int nStrLen = nDecodeInt16(pData, nSize);
	nByteUsed += 2;

	//获取string
	if (nStrLen > (nSize - nByteUsed))
	{
		std::cout << "string data is not complete" << std::endl;
		return -2;
	}
	strAmfString = std::string(&pData[nByteUsed], 0, nStrLen);
	nByteUsed += nStrLen;

	return nByteUsed;
}

int CAmfDecoder::nDecodeObject(const char* pData, int nSize, mapAmfObjects& mapAmfObjs)
{
	mapAmfObjs.clear();
	int nBytesUsed = 0;

	while (nSize > 0)
	{
		int nStrLen = nDecodeInt16(pData + nBytesUsed, nSize);
		nSize -= 2;
		if (nSize < nStrLen)
		{
			std::cout << "obj data is not complete" << std::endl;
			return nBytesUsed;
		}

		//获取键、值
		std::string strKey(pData + nBytesUsed + 2, 0, nStrLen);
		nSize -= nStrLen;

		CAmfDecoder dec;
		//每次解码一个对象，返回值为本次解码消耗了多少字节数
		int nRet = dec.nDecode(pData + nBytesUsed + 2 + nStrLen, nSize, 1);
		nBytesUsed += 2 + nStrLen + nRet;
		if (nRet <= 1)
		{
			std::cout << "decode fail" << std::endl;
			break;
		}

		mapAmfObjs.emplace(strKey, dec.GetObject());
	}

	return nBytesUsed;
}

uint16_t CAmfDecoder::nDecodeInt16(const char* pData, int nSize)
{
	return ReadUint16BE((char*)pData);
}

uint32_t CAmfDecoder::nDecodeInt24(const char* pData, int nSize)
{
	return ReadUint24BE((char*)pData);
}

uint32_t CAmfDecoder::nDecodeInt32(const char* pData, int nSize)
{
	return ReadUint32BE((char*)pData);
}

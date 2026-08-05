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
			break;

		default:
			break;
		}

		if (nRet < 0)
		{
			std::cout << "decode fail" << std::endl;
			return -1;
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
		std::cout << "data is not complete" << std::endl;
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
			std::cout << "data is not complete" << std::endl;
			return nBytesUsed;
		}
		nBytesUsed += 2;

		//获取键、值
		std::string strKey(pData + nBytesUsed, 0, nStrLen);
		nSize -= nStrLen;

		CAmfDecoder dec;
		//每次解码一个对象，返回值为本次解码消耗了多少字节数
		int nRet = dec.nDecode(pData + nBytesUsed + nStrLen, nSize, 1);
		if (nRet <= 1)
		{
			std::cout << "decode fail" << std::endl;
			break;
		}
		nBytesUsed += nStrLen + nRet;

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


CAmfEncoder::CAmfEncoder(uint32_t nSize)
	: m_pData(new char[nSize], std::default_delete<char[]>()), m_nSize(nSize)
{

}

CAmfEncoder::~CAmfEncoder()
{
}

void CAmfEncoder::encodeString(const char* pStr, int nLen, bool bIsObject)
{
	//编码字符串
	if (nLen < 0)
	{
		std::cout << "string length is negative, error" << std::endl;
		return ;
	}

	if ((m_nSize - m_nIndex) < (uint32_t)(nLen + 1 + 2 + 2)) // 1：类型，2：长度，2：字符串长度
	{
		//当前内存剩余空间不足，扩容
		this->realloc(m_nSize + nLen + 5);
	}

	if (nLen < 65536)//afm0_string
	{
		if (bIsObject)
		{
			//将类型赋值
			m_pData.get()[m_nIndex++] = AMF0_STRING;
		}
		encodeInt16((int16_t)nLen);//编码长度， 1字节类型 + 2字节长度 + 具体数值（string number bool）
	}
	else//amf0_long_string
	{
		if (bIsObject)
		{
			m_pData.get()[m_nIndex++] = AMF0_LONG_STRING;
		}
		encodeInt32(nLen);
	}

	memcpy(m_pData.get() + m_nIndex, pStr, nLen);
	m_nIndex += nLen;
}

void CAmfEncoder::encodeNumber(double fValue)
{
	if ((m_nSize - m_nIndex) < 9) // 1字节类型 8字节double数据
	{
		this->realloc(m_nSize + 1024);
	}

	m_pData.get()[m_nIndex++] = AMF0_NUMBER;

	//写入value
	char* pCin = (char*)&fValue;
	char* pCout = m_pData.get();

	pCout[m_nIndex++] = pCin[7];
	pCout[m_nIndex++] = pCin[6];
	pCout[m_nIndex++] = pCin[5];
	pCout[m_nIndex++] = pCin[4];
	pCout[m_nIndex++] = pCin[3];
	pCout[m_nIndex++] = pCin[2];
	pCout[m_nIndex++] = pCin[1];
	pCout[m_nIndex++] = pCin[0];
}

void CAmfEncoder::encodeBoolean(int nValue)
{
	if ((m_nSize - m_nIndex) < 2) // 1字节类型 1字节bool数据
	{
		this->realloc(m_nSize + 1024);
	}

	m_pData.get()[m_nIndex++] = AMF0_BOOLEAN;
	m_pData.get()[m_nIndex++] = nValue ? 0x01 : 0x00;
}

void CAmfEncoder::encodeObjects(mapAmfObjects& mapObjs)
{
	if (mapObjs.size() == 0)
	{
		encodeInt8(AMF0_NULL);
		std::cout << "amf obj is null" << std::endl;
		return;
	}

	encodeInt8(AMF0_OBJECT);
	for (auto it : mapObjs) //遍历对象进行编码 key->string value->object
	{
		//编码key
		encodeString(it.first.c_str(), (int)it.first.size(), false);
		//编码value
		switch (it.second.type)
		{
		case AMF_NUMBER:
			encodeNumber(it.second.fAmfNumber);
			break;

		case  AMF_STRING:
			encodeString(it.second.strAmfString.c_str(), (int)it.second.strAmfString.size());
			break;

		case AMF_BOOLEAN:
			encodeBoolean(it.second.bAmfBoolean);
			break;

		default:
			break;
		}
	}

	//结尾
	encodeString("", 0, false);
	encodeInt8(AMF0_OBJECT_END);
}

void CAmfEncoder::encodeECMA(mapAmfObjects& mapObjs)
{
	//对象数组
	encodeInt8(AMF0_ECMA_ARRAY);
	encodeInt32(0);

	//编码对象数组
	for (auto it : mapObjs)
	{
		//编码key
		encodeString(it.first.c_str(), (int)it.first.size(), false);
		//编码value
		switch (it.second.type)
		{
		case AMF_NUMBER:
			encodeNumber(it.second.fAmfNumber);
			break;

		case  AMF_STRING:
			encodeString(it.second.strAmfString.c_str(), (int)it.second.strAmfString.size());
			break;

		case AMF_BOOLEAN:
			encodeBoolean(it.second.bAmfBoolean);
			break;

		default:
			break;
		}
	}

	//结尾
	encodeString("", 0, false);
	encodeInt8(AMF0_OBJECT_END);
}

void CAmfEncoder::encodeInt8(int8_t nValue)
{
	if ((m_nSize - m_nIndex) < 1)//不足1个字节
	{
		this->realloc(m_nSize + 1024);
	}

	m_pData.get()[m_nIndex++] = nValue;
}

void CAmfEncoder::encodeInt16(int16_t nValue)
{
	if ((m_nSize - m_nIndex) < 2)//不足2个字节
	{
		this->realloc(m_nSize + 1024);
	}

	WriteUint16BE(m_pData.get() + m_nIndex, nValue);
	m_nIndex += 2;
}

void CAmfEncoder::encodeInt24(int32_t nValue)
{
	if ((m_nSize - m_nIndex) < 3)//不足3个字节
	{
		this->realloc(m_nSize + 1024);
	}

	WriteUint24BE(m_pData.get() + m_nIndex, nValue);
	m_nIndex += 3;
}

void CAmfEncoder::encodeInt32(int32_t nValue)
{
	if ((m_nSize - m_nIndex) < 4)//不足4个字节
	{
		this->realloc(m_nSize + 1024);
	}

	WriteUint32BE(m_pData.get() + m_nIndex, nValue);
	m_nIndex += 4;
}

void CAmfEncoder::realloc(uint32_t nSize)
{
	if (nSize <= m_nSize)
	{
		std::cout << "expand size is less than current size" << std::endl;
		return;
	}

	std::shared_ptr<char> pData(new char[nSize], std::default_delete<char[]>());
	memcpy(pData.get(), m_pData.get(), m_nIndex);
	m_nSize = nSize;
	m_pData = pData;
}

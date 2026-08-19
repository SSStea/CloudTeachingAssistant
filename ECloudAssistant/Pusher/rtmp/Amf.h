#pragma once
#include <string>
#include <memory>
#include <map>
#include <unordered_map>
#include <iostream>
#include "BufferReader.h"
#include "BufferWriter.h"
#include <string.h>

typedef enum
{
	AMF0_NUMBER = 0,
	AMF0_BOOLEAN,
	AMF0_STRING,
	AMF0_OBJECT,
	AMF0_MOVIECLIP,		/* reserved, not used */
	AMF0_NULL,
	AMF0_UNDEFINED,
	AMF0_REFERENCE,
	AMF0_ECMA_ARRAY,
	AMF0_OBJECT_END,
	AMF0_STRICT_ARRAY,
	AMF0_DATE,
	AMF0_LONG_STRING,
	AMF0_UNSUPPORTED,
	AMF0_RECORDSET,		/* reserved, not used */
	AMF0_XML_DOC,
	AMF0_TYPED_OBJECT,
	AMF0_AVMPLUS,		/* switch to AMF3 */
	AMF0_INVALID = 0xff
} AMF0DataType;

typedef enum
{
	AMF_NUMBER,
	AMF_BOOLEAN,
	AMF_STRING,
} AmfObjectType;

struct AmfObject
{
	AmfObjectType type;

	std::string strAmfString;
	double fAmfNumber;
	bool bAmfBoolean;

	AmfObject()
	{

	}

	AmfObject(std::string str)
	{
		this->type = AMF_STRING;
		this->strAmfString = str;
	}

	AmfObject(double number)
	{
		this->type = AMF_NUMBER;
		this->fAmfNumber = number;
	}
};

typedef std::unordered_map<std::string, AmfObject> mapAmfObjects;

class CAmfDecoder
{
public:
	/* n: 解码次数 */
	int nDecode(const char* pData, int nSize, int n = -1);

	void Reset()
	{
		m_obj.strAmfString = "";
		m_obj.fAmfNumber = 0;
		m_objs.clear();
	}

	std::string strGetString() const
	{
		return m_obj.strAmfString;
	}

	double fGetNumber() const
	{
		return m_obj.fAmfNumber;
	}

	bool bHasObject(std::string strKey) const
	{
		return (m_objs.find(strKey) != m_objs.end());
	}

	AmfObject GetObject(std::string strKey)
	{
		return m_objs[strKey];
	}

	AmfObject GetObject()
	{
		return m_obj;
	}

	mapAmfObjects GetObjects()
	{
		return m_objs;
	}

private:
	static int nDecodeBoolean(const char* pData, int nSize, bool& bAmfboolean);
	static int nDecodeNumber(const char* pData, int nSize, double& fAmfNumber);
	static int nDecodeString(const char* pData, int nSize, std::string& strAmfString);
	static int nDecodeObject(const char* pData, int nSize, mapAmfObjects& amfObjs);
	static uint16_t nDecodeInt16(const char* pData, int nSize);
	static uint32_t nDecodeInt24(const char* pData, int nSize);
	static uint32_t nDecodeInt32(const char* pData, int nSize);

	AmfObject m_obj;
	mapAmfObjects m_objs;
};

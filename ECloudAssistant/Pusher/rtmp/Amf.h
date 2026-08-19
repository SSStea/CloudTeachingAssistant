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

#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <sys/socket.h>

uint32_t ReadUint32BE(char* data);//大端读32位
uint32_t ReadUint32LE(char* data);//小端读32位
uint32_t ReadUint24BE(char* data);//24位
uint32_t ReadUint24LE(char* data);
uint16_t ReadUint16BE(char* data);//16位
uint16_t ReadUint16LE(char* data);

class CBufferReader
{
public:
	CBufferReader(uint32_t nInitSize = 2048);
	virtual ~CBufferReader();

	uint32_t nReadableBytes() const;
	uint32_t nWritableBytes() const;

	//获取可读数据的首地址
	char* pPeek();
	const char* pPeek() const;

	//重新初始化缓冲区读写索引
	void RetrieveAll();

	//更新读缓冲
	void Retrieve(size_t nLen);

	//从套接字中读数据到缓冲区
	int nReadFromSocket(int nFd);
	//从缓冲区中读数据到data
	uint32_t nReadFromBuffer(std::string& data);
	uint32_t nSize() const;

private:
	//获取缓冲区的首地址
	char* pBegin();
	const char* pBegin() const;

	//获取可写的位置的首地址
	char* pBeginWrite();
	const char* pBeginWrite() const;

private:
	std::vector<char> m_vecBuffer_;
	uint32_t m_nReadIndex = 0;
	uint32_t m_nWriteIndex = 0;
	static const uint32_t MAX_BYTES_PER_READ = 4096;
	static const uint32_t MAX_BUFFER_SIZE = 1000000 * 1024;
};


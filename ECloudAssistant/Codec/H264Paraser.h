#ifndef H264PARASER_H
#define H264PARASER_H
#include <cstdint>
#include <utility>

class CH264Paraser
{
public:
    typedef std::pair<uint8_t*, uint8_t*> Nal;//这两个指针分别指向这个nal的头和尾
public:
    CH264Paraser();
    static Nal findNal(const uint8_t* pData, uint32_t nSize);
};

#endif // H264PARASER_H

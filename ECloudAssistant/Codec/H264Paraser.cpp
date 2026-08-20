#include "H264Paraser.h"
#include <cstring>

CH264Paraser::CH264Paraser() {}

CH264Paraser::Nal CH264Paraser::findNal(const uint8_t *pData, uint32_t nSize)
{
    Nal nal(nullptr, nullptr);

    if(nSize < 5)//因为sps或pps会大于5
    {
        return nal;
    }

    nal.second = const_cast<uint8_t*>(pData) + (nSize - 1);

    uint32_t nStartCode = 0;
    uint32_t nPos = 0;
    uint8_t prefix[3] = {0};

    memcpy(prefix, pData, 3);

    nSize -= 3;
    pData += 2;

    while(nSize--)
    {
        if(prefix[nPos % 3] == 0 && prefix[(nPos + 1) % 3] == 0 && prefix[(nPos + 2) % 3] == 1)
        {
            //00 00 01
            if(nal.first == nullptr)
            {
                nal.first = const_cast<uint8_t*>(pData) + 1;//偏移startcode
                nStartCode = 3;
            }

            //退出
            else if(nStartCode == 3)//说明找到了下一个startcode
            {
                //更新尾部
                nal.second = const_cast<uint8_t*>(pData) - 3;//减去前缀
            }
        }
        else if(prefix[nPos % 3] == 0 && prefix[(nPos + 1) % 3] == 0 && prefix[(nPos + 2) % 3] == 0)
        {
            //00 00 00 01
            if(*(pData + 1) == 0x01)
            {
                if(nal.first == nullptr)
                {
                    if(nSize >= 1)
                    {
                        nal.first = const_cast<uint8_t*>(pData) + 2;//偏移startcode
                    }
                    else
                    {
                        break;
                    }
                    nStartCode = 4;
                }
                else if(nStartCode == 4)//找到下一个nal头
                {
                    //更新尾部
                    nal.second = const_cast<uint8_t*>(pData) - 3;//减去前缀
                    break;
                }
            }
        }
        //更新前缀
        prefix[(nPos++) % 3] = *(++pData);
    }

    if(nal.first == nullptr)
    {
        nal.second = nullptr;
    }

    return nal;
}

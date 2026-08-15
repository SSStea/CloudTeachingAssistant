#ifndef DEFINE_H
#define DEFINE_H
#include <cstdint>
#include <string>
#include <array>
#include <string.h>

#pragma pack(push,1)
enum Cmd : uint16_t
{
    Minotor,
    ERROR,
    Login,
    Register,
    Destory,
};

enum ResultCode
{
    S_OK = 0,
    SERVER_ERROR ,
    REQUEST_TIMEOUT ,
    ALREADY_REDISTERED ,
    USER_DISAPPEAR,
    ALREADY_LOGIN,
    VERFICATE_FAILED
};

struct packet_head {
    packet_head()
        :len(-1)
        , cmd(-1) {}
    uint16_t len;
    uint16_t cmd;
};

struct Login_Info : public packet_head
{
    Login_Info():packet_head()
    {
        cmd = Login;
        len = sizeof(Login_Info);
        timestamp = -1;
    }
    uint64_t timestamp;
};

//登录应答
struct LoginReply : public packet_head
{
    LoginReply():packet_head()
    {
        cmd = Login;  //如果请求超时，将这个cmd置为ERROR
        len = sizeof(LoginReply);
        port = -1;
        ip.fill('\0');
    }
    uint16_t port;
    std::array<char,16> ip;
};


struct UserRegister : public packet_head
{
    UserRegister():packet_head()
    {
        cmd = Register;
        len = sizeof(UserRegister);
    }
    std::string GetCode()
    {
        return std::string(code.data());
    }
    std::string GetName()
    {
        return std::string(name.data());
    }
    std::string GetCount()
    {
        return std::string(count.data());
    }
    std::string GetPasswd()
    {
        return std::string(passwd.data());
    }
    void SetName(const std::string& strName)
    {
        name.fill('\0');
        strName.copy(name.data(), name.size() - 1);
    }

    void SetCode(const std::string& strCode)
    {
        code.fill('\0');
        strCode.copy(code.data(), code.size() - 1);
    }

    void SetCount(const std::string& strCount)
    {
        count.fill('\0');
        strCount.copy(count.data(), count.size() - 1);
    }

    void SetPasswd(const std::string& strPasswd)
    {
        passwd.fill('\0');
        strPasswd.copy(passwd.data(), passwd.size() - 1);
    }
    std::array<char,20> code;
    std::array<char,20> name;
    std::array<char,12> count;
    std::array<char,20> passwd;
    uint64_t timestamp;
};

struct UserLogin : public packet_head
{
    UserLogin():packet_head()
    {
        cmd = Login;
        len = sizeof(UserLogin);
    }
    void SetCode(const std::string& str)
    {
        str.copy(code.data(),code.size(),0);
    }
    std::string GetCode()
    {
        return std::string(code.data());
    }
    void SetCount(const std::string& str)
    {
        str.copy(count.data(),count.size(),0);
    }
    std::string GetCount()
    {
        return std::string(count.data());
    }
    void SetPasswd(const std::string& str)
    {
        str.copy(passwd.data(),passwd.size(),0);
    }
    std::string GetPasswd()
    {
        return std::string(passwd.data());
    }
    std::array<char,20> code;
    std::array<char,12> count;
    std::array<char,33> passwd; //Md5
    uint64_t timestamp;
};

struct RegisterResult : public packet_head
{
    RegisterResult():packet_head()
    {
        cmd = Register;
        len = sizeof(RegisterResult);
    }
    ResultCode resultCode;
};

struct LoginResult : public packet_head
{
    LoginResult() : packet_head()
    {
        cmd = Login;
        len = sizeof(LoginResult);
    }
    void SetIp(const std::string& str)
    {
        //str.copy(ctrSvrIp.data(),ctrSvrIp.size()+1,0);
        strncpy(ctrSvrIp.data(), str.c_str(), ctrSvrIp.size() - 1);
        ctrSvrIp.back() = '\0'; // 强制最后一个字符为终止符
    }
    std::string GetIp()
    {
        return std::string(ctrSvrIp.data());
    }
    ResultCode resultCode;
    uint16_t port;
    std::array<char, 16> ctrSvrIp;
};

struct UserDestory : public packet_head
{
    UserDestory(): packet_head()
    {
        cmd = Destory;
        len = sizeof(UserDestory);
    }
    void SeCode(const std::string& str)
    {
        str.copy(code.data(),code.size(),0);
    }
    std::string GetCode()
    {
        return std::string(code.data());
    }
    std::array<char,20> code;
};

struct Monitor_body : public packet_head {
    Monitor_body()
        :packet_head()
    {
        cmd = Minotor;
        len = sizeof(Monitor_body);
        ip.fill('\0');
    }
    void SetIp(const std::string& str)
    {
        str.copy(ip.data(), ip.size(), 0);
    }
    std::string GetIp()
    {
        return std::string(ip.data());
    }
    uint8_t mem;
    std::array<char, 16> ip;
    uint16_t port;
};

typedef std::pair<int,Monitor_body*> MinotorPair;

struct CmpByValue
{
    bool operator()(const MinotorPair& l,const MinotorPair& r)
    {
        return l.second->mem < r.second->mem;//排序，从小到大排序
    }
};
#pragma pack(pop)

#endif // DEFINE_H

#pragma once
#include <cstdint>
#include <array>
#include <string>

//cmd命令
enum Cmd : uint16_t
{
	JOIN = 5,			//加入房间
	OBTAINSTREAM,		//获取流
	CREATESTREAM,		//创建流
	PLAYSTREAM,			//播放流
	DELETESTREAM,		//删除流
	MOUSE,				//鼠标指令
	MOUSEMOVE,			//鼠标移动指令
	KEY,				//键盘指令
	WHEEL,				//滚轮
};

//应答
enum ResultCode
{
	SUCCESSFUL,			//成功
	ERROR,				//错误
	REQUEST_TIMEOUT,
	ALREADY_REDISTERED,
	USER_DISAPPEAR,
	ALREADY_LOGIN,
	VERFICATE_FAILED,
};

//客户端角色状态
enum RoleState
{
	IDLE,		//说明客户端已经创建房间，而且没有去拉流也没有推流
	NONE,		//说明当前客户端没有创建房间
	CLOSE,		//客户端断开连接
	PULLER,		//客户端进入拉流模式
	PUSHER,		//客户端开始推流
};

//需要1字节对齐
#pragma pack(push, 1)
struct PacketHead
{
	PacketHead() : nLen(-1), nCmd(-1){}
	uint16_t nLen;
	uint16_t nCmd;
};

//包体
//创建房间
struct JoinBody : public PacketHead
{
	JoinBody()
	{
		nCmd = JOIN;
		nLen = sizeof(JoinBody);
		arrID.fill('\0');
	}

	void SetID(const std::string& str)
	{
		str.copy(arrID.data(), arrID.size(), 0);
	}

	std::string strGetID()
	{
		return std::string(arrID.data());
	}

	std::array<char, 10> arrID;
};

//创建房间的应答
struct JoinReplyBody : public PacketHead
{
	JoinReplyBody()
	{
		nCmd = JOIN;
		nLen = sizeof(JoinReplyBody);
		result = ERROR;
	}
	//设置结果
	void SetCode(const ResultCode code)
	{
		result = code;
	}
	ResultCode result;
};

//获取流
struct ObtainStreamBody : public PacketHead
{
	ObtainStreamBody()
	{
		nCmd = OBTAINSTREAM;
		nLen = sizeof(ObtainStreamBody);
		arrID.fill('\0');
	}

	void SetID(const std::string& str)
	{
		str.copy(arrID.data(), arrID.size(), 0);
	}

	std::string strGetID()
	{
		return std::string(arrID.data());
	}

	std::array<char, 10> arrID;
};

//获取流应答
struct ObtainStreamReplyBody : public PacketHead
{
	ObtainStreamReplyBody()
	{
		nCmd = OBTAINSTREAM;
		nLen = sizeof(ObtainStreamReplyBody);
		result = ERROR;
	}

	//设置结果
	void SetCode(const ResultCode code)
	{
		result = code;
	}
	ResultCode result;
};

//创建流
struct CreateStreamBody : public PacketHead
{
	CreateStreamBody()
	{
		nCmd = CREATESTREAM;
		nLen = sizeof(CreateStreamBody);
	}
};

//创建流应答 返回流地址和结果
struct CreateStreamReplyBody : public PacketHead
{
	CreateStreamReplyBody()
	{
		nCmd = CREATESTREAM;
		nLen = sizeof(CreateStreamReplyBody);
		result = ERROR;
		arrStreamAddress.fill('\0');
	}

	void SetStreamAddr(const std::string& str)
	{
		str.copy(arrStreamAddress.data(), arrStreamAddress.size(), 0);
	}

	std::string strGetStreamAddr()
	{
		return std::string(arrStreamAddress.data());
	}

	//设置结果
	void SetCode(const ResultCode code)
	{
		result = code;
	}

	ResultCode result;
	std::array<char, 70> arrStreamAddress;
};

//播放流，提供播放流地址
struct PlayStreamBody : public PacketHead
{
	PlayStreamBody()
	{
		nCmd = PLAYSTREAM;
		nLen = sizeof(PlayStreamBody);
		result = ERROR;
		arrStreamAddress.fill('\0');
	}

	void SetStreamAddr(const std::string& str)
	{
		str.copy(arrStreamAddress.data(), arrStreamAddress.size(), 0);
	}

	std::string strGetStreamAddr()
	{
		return std::string(arrStreamAddress.data());
	}

	//设置结果
	void SetCode(const ResultCode code)
	{
		result = code;
	}

	ResultCode result;
	std::array<char, 70> arrStreamAddress;
};

//播放流应答
struct PlayStreamReplayBody : public PacketHead
{
	PlayStreamReplayBody()
	{
		nCmd = PLAYSTREAM;
		nLen = sizeof(PlayStreamReplayBody);
		result = ERROR;
	}
	void SetCode(const ResultCode code)
	{
		result = code;
	}
	ResultCode result;
};

//删除流
struct DeleteStreamBody : public PacketHead
{
	DeleteStreamBody() 
	{
		nCmd = DELETESTREAM;
		nStreamCount = -1;
		nLen = sizeof(DeleteStreamBody);
	}
	void SetStreamCount(const int nCount)
	{
		nStreamCount = nCount;
	}
	int nStreamCount;	//推流的时候，如果发现拉流数量为0,我们就需要停止推流，
						//如果流数量不为0，就说明还有客户端连接，不能停止推流
};

//鼠标键盘信息
enum MouseType : uint8_t
{
	NoButton = 0,
	LeftButton = 1,
	RightButton = 2,
	MiddleButton = 4,
	XButton1 = 8,
	XButton2 = 16,
};

//鼠标键盘按下还是松开
enum MouseKeyType : uint8_t
{
	PRESS,
	RELESE,
};

//键盘消息
struct KeyBody : public PacketHead
{
	KeyBody() : PacketHead() {
		nCmd = KEY;
		nLen = sizeof(KeyBody);
	}
	//键值和类型
	uint16_t nKey;
	MouseKeyType type;
};

//滚轮消息
struct WheelBody : public PacketHead
{
	WheelBody() :PacketHead() {
		nCmd = WHEEL;
		nLen = sizeof(WheelBody);
	}
	//值
	uint8_t nWheel;
};

//鼠标移动
struct MouseMoveBody : public PacketHead
{
	MouseMoveBody() :PacketHead() {
		nCmd = MOUSEMOVE;
		nLen = sizeof(MouseMoveBody);
	}
	//需要x,y比值
	uint8_t xl_ratio;
	uint8_t xr_ratio;
	uint8_t yl_ratio;
	uint8_t yr_ratio;
};

//鼠标事件
struct MouseBody : public PacketHead
{
	MouseBody() :PacketHead()
	{
		nCmd = MOUSE;
		nLen = sizeof(MouseBody);
	}
	MouseKeyType type;
	MouseType mouseType;
};

#pragma pack(pop)

#pragma once

#include "define.h"
#include "BufferReader.h"
#include "TcpConnection.h"

#include <QCursor>
#include <QScreen>
#include <QString>

#include <functional>

class CSigConnection : public CTcpConnection
{
public:
	enum UserType
	{
		CONTROLLED,
		CONTROLLING
	};

	enum State
	{
		NONE,
		IDLE,
		PULLER,
		PUSHER
	};

	using StopStreamCallback = std::function<void()>;
	using StartStreamCallback = std::function<bool(const QString& strStreamAddress)>;

	CSigConnection(
		CReactorBase* pReactor,
		int nSocket,
		const QString& strCode,
		const UserType& type = CONTROLLED);
	virtual ~CSigConnection();

	inline bool bIsIdle() const
	{
		return m_state == IDLE;
	}

    //新增状态查询
	inline bool bIsPusher() const
	{
		return m_state == PUSHER;
	}

	inline bool bIsPuller() const
	{
		return m_state == PULLER;
	}

	inline bool bIsNone() const
	{
		return m_state == NONE;
	}

    //新增回调设置：
	inline void SetStartStreamCallback(const StartStreamCallback& callback)
	{
		m_startStreamCallback = callback;
	}

	inline void SetStopStreamCallback(const StopStreamCallback& callback)
	{
		m_stopStreamCallback = callback;
	}

protected:
	bool bOnRead(CBufferReader& buffer);
    void OnClose(); //连接关闭后设置退出状态。
	void HandleMessage(CBufferReader& buffer);

private:
    qint32 nJoin(); //发送加入请求
    qint32 nObtainStream(); //发送获取流请求。
    //新增客户端消息处理
	void DoJoin(const packet_head* pData);
	void DoPlayStream(const packet_head* pData);
	void DoCreateStream(const packet_head* pData);
	void DoDeleteStream(const packet_head* pData);
	void DoMouseEvent(const packet_head* pData);
	void DoMouseMoveEvent(const packet_head* pData);
	void DoKeyEvent(const packet_head* pData);
	void DoWheelEvent(const packet_head* pData);

private:
	bool m_bQuit = false;
	State m_state = NONE;
	QString m_strCode;
	const UserType m_type;
	QScreen* m_pScreen = nullptr;
	StopStreamCallback m_stopStreamCallback = []() {};
	StartStreamCallback m_startStreamCallback =
		[](const QString& strStreamAddress) -> bool
		{
			(void)strStreamAddress;
			return true;
		};
};

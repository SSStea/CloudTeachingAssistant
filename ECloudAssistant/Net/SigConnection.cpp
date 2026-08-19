#include "SigConnection.h"

#include <QDebug>
#include <QGuiApplication>
#include <windows.h>

int nStreamIndex = 1;

CSigConnection::CSigConnection(
	CReactorBase* pReactor,
	int nSocket,
	const QString& strCode,
	const UserType& type)
	: CTcpConnection(pReactor, nSocket)
	, m_strCode(strCode)
	, m_type(type)
{
	setReadCallback([this](std::shared_ptr<CTcpConnection> pConnection, CBufferReader& buffer)
	{
		(void)pConnection;
		return bOnRead(buffer);
	});

	setCloseCallback([this](std::shared_ptr<CTcpConnection> pConnection)
	{
		(void)pConnection;
		OnClose();
	});

	m_pScreen = QGuiApplication::primaryScreen();
	nJoin();
}

CSigConnection::~CSigConnection()
{
}

bool CSigConnection::bOnRead(CBufferReader& buffer)
{
	while (buffer.nReadableBytes() > 0)
	{
		HandleMessage(buffer);
	}

	return true;
}

void CSigConnection::OnClose()
{
	m_bQuit = true;
}

void CSigConnection::HandleMessage(CBufferReader& buffer)
{
	packet_head* pHead = (packet_head*)buffer.pPeek();
	if (buffer.nReadableBytes() < pHead->len)
	{
		return;
	}

	switch (pHead->cmd)
	{
	case JOIN:
		DoJoin(pHead);
		break;
	case PLAYSTREAM:
		DoPlayStream(pHead);
		break;
	case CREATESTREAM:
		DoCreateStream(pHead);
		break;
	case DELETESTREAM:
		DoDeleteStream(pHead);
		break;
	case MOUSE:
		DoMouseEvent(pHead);
		break;
	case MOUSEMOVE:
		DoMouseMoveEvent(pHead);
		break;
	case KEY:
		DoKeyEvent(pHead);
		break;
	case WHEEL:
		DoWheelEvent(pHead);
		break;
	default:
		break;
	}

	buffer.Retrieve(pHead->len);
}

qint32 CSigConnection::nJoin()
{
	if (m_state != NONE)
	{
		return -1;
	}

	Join_body body;
	if (m_type == CONTROLLED)
	{
		body.SetId(m_strCode.toStdString());
	}
	else
	{
		body.SetId("154564");
	}

	Send((const char*)&body, body.len);
	return 0;
}

qint32 CSigConnection::nObtainStream()
{
	if (m_state == IDLE && m_type == CONTROLLING)
	{
		ObtainStream_body body;
		body.SetId(m_strCode.toStdString());
		Send((const char*)&body, body.len);
		return 0;
	}

	return -1;
}

void CSigConnection::DoJoin(const packet_head* pData)
{
	const JoinReply_body* pReply = (const JoinReply_body*)pData;
    if (pReply->result == S_OK)//加入成功
	{
        m_state = IDLE;//状态置为空闲
        if (m_type == CONTROLLING)//控制方
		{
            if (nObtainStream() != 0)//获取流
			{
				qDebug() << "获取流请求发送失败";
			}
			else
			{
                m_state = PULLER;//状态置为拉流
				qDebug() << "获取流请求发送成功";
			}
		}
	}
}

void CSigConnection::DoPlayStream(const packet_head* pData)
{
    if (m_state == PULLER && m_type == CONTROLLING)//拉流状态，并且是控制方
	{
		PlayStream_body* pPlayStream = (PlayStream_body*)pData;
		if (pPlayStream->result == S_OK)
		{
			qDebug() << "开始播放流";
			if (m_startStreamCallback)
			{
				m_startStreamCallback(
					QString::fromStdString(pPlayStream->GetstreamAddres()));
			}
		}
		else
		{
			qDebug() << "播放流失败";
		}
	}
}

void CSigConnection::DoCreateStream(const packet_head* pData)
{
	(void)pData;

    //状态空闲且为被控方
	if (m_state == IDLE && m_type == CONTROLLED)
	{
		CreateStreamReply_body reply;
		QString strStreamAddress =
            "rtmp://172.20.108.206:1935/live/" +
			QString::number(++nStreamIndex);
		if (m_startStreamCallback)
		{
            if (m_startStreamCallback(strStreamAddress))//调用回调函数推流
			{
				reply.SetstreamAddres(strStreamAddress.toStdString());
				reply.SetCode((ResultCode)0);
                Send((const char*)&reply, reply.len);//发送推流应答
                m_state = PUSHER;//设置状态为推流方
			}
			else
			{
				reply.SetCode(SERVER_ERROR);
				Send((const char*)&reply, reply.len);
			}
		}
	}
}

void CSigConnection::DoDeleteStream(const packet_head* pData)
{
	const DeleteStream_body* pBody = (const DeleteStream_body*)pData;
	if (pBody->streamCount == 0 && m_stopStreamCallback)
	{
		m_stopStreamCallback();
	}
}

void CSigConnection::DoMouseEvent(const packet_head* pData)
{
	const Mouse_Body* pBody = (const Mouse_Body*)pData;
	DWORD nFlags = 0;
	if (pBody->type == PRESS)
	{
		nFlags |=
			(pBody->mouseButtons & MouseType::LeftButton) ?
			MOUSEEVENTF_LEFTDOWN : 0;
		nFlags |=
			(pBody->mouseButtons & MouseType::RightButton) ?
			MOUSEEVENTF_RIGHTDOWN : 0;
		nFlags |=
			(pBody->mouseButtons & MouseType::MiddleButton) ?
			MOUSEEVENTF_MIDDLEDOWN : 0;
	}
	else if (pBody->type == RELEASE)
	{
		nFlags |=
			(pBody->mouseButtons & MouseType::LeftButton) ?
			MOUSEEVENTF_LEFTUP : 0;
		nFlags |=
			(pBody->mouseButtons & MouseType::RightButton) ?
			MOUSEEVENTF_RIGHTUP : 0;
		nFlags |=
			(pBody->mouseButtons & MouseType::MiddleButton) ?
			MOUSEEVENTF_MIDDLEUP : 0;
	}

	if (nFlags != 0)
	{
		INPUT input = { 0 };
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = nFlags;
		SendInput(1, &input, sizeof(input));
	}
}

void CSigConnection::DoMouseMoveEvent(const packet_head* pData)
{
	const MouseMove_Body* pBody = (const MouseMove_Body*)pData;
	double dRatioX =
		((double)pBody->xl_ratio + ((double)pBody->xr_ratio / 100.0)) /
		100.0;
	double dRatioY =
		((double)pBody->yl_ratio + ((double)pBody->yr_ratio / 100.0)) /
		100.0;
	int nPositionX = (int)(
		dRatioX * m_pScreen->size().width() /
		m_pScreen->devicePixelRatio());
	int nPositionY = (int)(
		dRatioY * m_pScreen->size().height() /
		m_pScreen->devicePixelRatio());
	QCursor::setPos(nPositionX, nPositionY);
}

void CSigConnection::DoKeyEvent(const packet_head* pData)
{
	const Key_Body* pBody = (const Key_Body*)pData;
	DWORD nVirtualKey = pBody->key;
	INPUT input[1];
	ZeroMemory(input, sizeof(input));
	DWORD nOperation = pBody->type ? KEYEVENTF_KEYUP : 0;
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = (WORD)nVirtualKey;
	input[0].ki.dwFlags = nOperation;
	input[0].ki.wScan = (WORD)MapVirtualKey(nVirtualKey, 0);
	SendInput(1, input, sizeof(INPUT));
}

void CSigConnection::DoWheelEvent(const packet_head* pData)
{
	const Wheel_Body* pBody = (const Wheel_Body*)pData;
	INPUT input = { 0 };
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_WHEEL;
	input.mi.mouseData = pBody->wheel * 240;
	SendInput(1, &input, sizeof(input));
}

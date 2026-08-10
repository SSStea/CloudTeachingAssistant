#include "SigConnection.h"

CSigConnection::CSigConnection(CReactorBase* reactor, int nSocket)
	: CTcpConnection(reactor, nSocket), m_state(NONE)
{
	//设置回调函数，处理读数据
	this->setReadCallback([this](std::shared_ptr<CTcpConnection> conn, CBufferReader& buffer)
		{
			return this->bOnRead(buffer);
		});

	//设置关闭回调，释放资源
	this->setCloseCallback([this](std::shared_ptr<CTcpConnection> conn)
		{
			this->Disconnected();
		});
}

CSigConnection::~CSigConnection()
{
	Clear();
}

bool CSigConnection::bIsAlive()
{
	return m_state != CLOSE;
}

bool CSigConnection::bIsIdle()
{
	return m_state == IDLE;
}

bool CSigConnection::bIsBusy()
{
	return (m_state == PUSHER || m_state == PULLER);
}

bool CSigConnection::bIsNoJoin()
{
	return m_state == NONE;
}

void CSigConnection::Disconnected()
{
	Clear();
}

void CSigConnection::AddClient(const std::string& strCode)
{
	//添加客户端，客户可能是拉流器可能是推流器
	for (const auto& idefy : m_vecObjectes)
	{
		if (idefy == strCode) //目标客户端已经添加
		{
			return;
		}
	}

	m_vecObjectes.push_back(strCode);
}

void CSigConnection::RemoveClient(const std::string& strCode)
{
	if (m_vecObjectes.empty())
	{
		return;
	}
	m_vecObjectes.erase(std::remove(m_vecObjectes.begin(), m_vecObjectes.end(), strCode), 
		m_vecObjectes.end());

	if (m_vecObjectes.empty())
	{
		m_state = IDLE; //说明当前没有控制端，自己也不是控制端
	}
}

RoleState CSigConnection::GetRoleState() const
{
	return m_state;
}

std::string CSigConnection::strGetCode() const
{
	return m_strCode;
}

std::string CSigConnection::strGetStreamAddr() const
{
	return m_strStreamAddr;
}

bool CSigConnection::bOnRead(CBufferReader& buffer)
{
	if (buffer.nReadableBytes() > 0)
	{
		HanldeMessage(buffer);
	}

	return true;
}

void CSigConnection::HanldeMessage(CBufferReader& buffer)
{
	if (buffer.nReadableBytes() < sizeof(PacketHead))
	{
		//数据不完整
		return;
	}

	//获取数据
	PacketHead* data = (PacketHead*)buffer.pPeek();
	if (buffer.nReadableBytes() < data->nLen)
	{
		//数据不完整
		return;
	}

	switch (data->nCmd)
	{
	case JOIN:
		HandleJoin(data);
		break;
	case OBTAINSTREAM:
		HandleObtainStream(data);
		break;
	case CREATESTREAM:
		HandleCreateStream(data);
		break;
	case DELETESTREAM:
		HandleDeleteStream(data);
		break;
	case MOUSE:
	case MOUSEMOVE:
	case KEY:
	case WHEEL:
		HandleOtherMessage(data);
		break;
	default:
		break;
	}

	//更新缓冲区
	buffer.Retrieve(data->nLen);
}

void CSigConnection::Clear()
{
	m_state = CLOSE;
	m_conn = nullptr;
	DeleteStreamBody body;//需要通知所有关心者这个删除流通知

	//当前 CSigConnection 断开时，通知所有与它有关联的客户端，并从这些客户端的关联列表中删除自己。
	for (auto& it : m_vecObjectes)//遍历当前连接关联的每一个客户端Code
	{
		//根据客户端Code从连接管理器中查询对应的TCP连接
		CTcpConnection::ptr conn = CConnectionManager::GetInstance()->QueryConn(it);
		if (conn)
		{
			//将通用的 CTcpConnection 智能指针转换为 CSigConnection，
			// 因为只有 CSigConnection 才有 RemoveClient() 和 m_vecObjectes
			auto ctrConn = std::dynamic_pointer_cast<CSigConnection>(conn);
			if (ctrConn)
			{
				//从对方的关联列表中删除当前断开连接的Code。
				ctrConn->RemoveClient(m_strCode);
			}
			//将对方当前剩余的关联客户端数量写入删除流通知
			body.SetStreamCount((int)(ctrConn->m_vecObjectes.size()));
			//向对方发送 DeleteStreamBody，通知它相关流已经被删除，同时告诉它还剩多少关联客户端
			conn->Send((const char*)&body, body.nLen);

		}
	}
	m_vecObjectes.clear();
	CConnectionManager::GetInstance()->RemoveConn(m_strCode);

	std::cout << "conn size: " << CConnectionManager::GetInstance()->nSize() << std::endl;
}

void CSigConnection::HandleJoin(const PacketHead* data)
{
	//准备一个应答
	JoinReplyBody replyBody;
	JoinBody* body = (JoinBody*)data;

	if (this->bIsNoJoin())//客户端没有创建房间
	{
		std::string strCode = body->strGetID();
		CTcpConnection::ptr conn = CConnectionManager::GetInstance()->QueryConn(strCode);
		if (conn)//连接管理器中有这个客户端id的连接器，报错
		{
			replyBody.SetCode(ERROR);
			this->Send((const char*)&replyBody, replyBody.nLen);
			return;
		}

		m_strCode = strCode;
		m_state = IDLE;
		//把这个客户端加入连接管理器
		CConnectionManager::GetInstance()->AddConn(strCode, shared_from_this());
		std::cout << "Join Count: " << CConnectionManager::GetInstance()->nSize() << std::endl;
		replyBody.SetCode(SUCCESSFUL);
		this->Send((const char*)&replyBody, replyBody.nLen);
		return;
	}

	//已经创建，返回错误
	replyBody.SetCode(ERROR);
	this->Send((const char*)&replyBody, replyBody.nLen);
}

void CSigConnection::HandleObtainStream(const PacketHead* data)
{
	//获取流
	ObtainStreamBody* body = (ObtainStreamBody*)data;

	return this->DoObtainStream(body);
}

void CSigConnection::HandleCreateStream(const PacketHead* data)
{
	CreateStreamBody* body = (CreateStreamBody*)data;

	return this->DoCreateStream(body);
}

void CSigConnection::HandleDeleteStream(const PacketHead* data)
{
	if (this->bIsBusy())
	{
		Clear();
	}
}

void CSigConnection::HandleOtherMessage(const PacketHead* data)
{
	//鼠标键盘消息，转发
	if (m_conn && m_state == PULLER) ////说明当前是在拉流
	{
		m_conn->Send((const char*)data, data->nLen);
	}
}

void CSigConnection::DoObtainStream(const PacketHead* data)
{
	ObtainStreamReplyBody replyBody;
	CreateStreamBody creatBody;
	std::string strCode = ((ObtainStreamBody*)data)->strGetID();
	CTcpConnection::ptr tcpConn = CConnectionManager::GetInstance()->QueryConn(strCode);
	if (!tcpConn)//连接器不存在
	{
		std::cout << "remote target not exist " << std::endl;
		replyBody.SetCode(ERROR);
		this->Send((const char*)&replyBody, replyBody.nLen);
		return;
	}
	if (tcpConn == shared_from_this())//连接器等于本身，不能控制自己
	{
		std::cout << "can not control self " << std::endl;
		replyBody.SetCode(ERROR);
		this->Send((const char*)&replyBody, replyBody.nLen);
		return;
	}
	if (this->bIsIdle())
	{
		auto conn = std::dynamic_pointer_cast<CSigConnection>(tcpConn);

		switch (conn->GetRoleState())
		{
		case IDLE://目标空闲，通知他去推流
			std::cout << "target IDLE" << std::endl;
			this->m_state = PULLER;
			this->AddClient(strCode); // 添加被控端
			conn->AddClient(m_strCode);//被控端需要添加控制端
			replyBody.SetCode(SUCCESSFUL);
			m_conn = tcpConn;//我们就可以通过这个目标(被控端)连接器来转发消息
			//通知被控端来创建流
			conn->Send((const char*)&creatBody, creatBody.nLen);
			break;

		case NONE:
			std::cout << "target is not online" << std::endl;
			replyBody.SetCode(ERROR);
			this->Send((const char*)&replyBody, replyBody.nLen);
			break;

		case CLOSE:
			std::cout << "target is offline" << std::endl;
			replyBody.SetCode(ERROR);
			this->Send((const char*)&replyBody, replyBody.nLen);
			break;

		case PULLER://目标拉流(控制端) 控制端不能控制控制端
			std::cout << "target is busy" << std::endl;
			replyBody.SetCode(ERROR);
			this->Send((const char*)&replyBody, replyBody.nLen);
			break;

		case PUSHER://推流说明他是被控端，所以我们可以去拉流
			if (conn->strGetStreamAddr().empty())//地址为空，异常
			{
				std::cout << "target is pushing, stream address exception" << std::endl;
				replyBody.SetCode(ERROR);
				this->Send((const char*)&replyBody, replyBody.nLen);
			}
			else//在推流，而且流地址正常
			{
				std::cout << "target is pushing, stream address ok" << std::endl;
				this->m_state = PUSHER;
				this->AddClient(strCode);
				conn->AddClient(m_strCode);
				//在推流 流已经存在，就不需要重新创建流，我们只需要播放流；
				PlayStreamBody playBody;
				playBody.SetCode(SUCCESSFUL);
				playBody.SetStreamAddr(conn->strGetStreamAddr());
				this->Send((const char*)&playBody, playBody.nLen);
			}
			break;

		default:
			break;
		}
	}
	else //忙碌
	{
		replyBody.SetCode(ERROR);
		this->Send((const char*)&replyBody, replyBody.nLen);
	}
}

void CSigConnection::DoCreateStream(const PacketHead* data)
{
	PlayStreamBody body;
	//判断所有连接的状态，如果连接器状态是空闲，我们就去回应
	for (auto idefy : m_vecObjectes)
	{
		CTcpConnection::ptr tcpConn = CConnectionManager::GetInstance()->QueryConn(idefy);
		if (!tcpConn)
		{
			this->RemoveClient(idefy);
			continue;
		}
		auto conn = std::dynamic_pointer_cast<CSigConnection>(tcpConn);
		if (m_strStreamAddr.empty())
		{
			conn->m_state = IDLE;
			body.SetCode(ERROR);
			this->Send((const char*)&body, body.nLen);
			continue;
		}

		switch (conn->GetRoleState())
		{
		case NONE:
		case IDLE:
		case CLOSE:
		case PUSHER:
			body.SetCode(ERROR);
			this->RemoveClient(conn->strGetCode());
			conn->Send((const char*)&body, body.nLen);
			break;

		case PULLER:
			this->m_state = PULLER;
			body.SetCode(SUCCESSFUL);
			body.SetStreamAddr(m_strStreamAddr);
			std::cout << "stream address: " << m_strStreamAddr.c_str() << std::endl;
			conn->Send((const char*)&body, body.nLen);
			break;

		default:
			break;
		}
	}
}

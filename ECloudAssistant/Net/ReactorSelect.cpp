#include "ReactorSelect.h"

#include <forward_list>

CReactorSelect::CReactorSelect(int nId)
	: CReactorBase(nId)
{
	FD_ZERO(&m_fdReadBackup);
	FD_ZERO(&m_fdWriteBackup);
	FD_ZERO(&m_fdExceptBackup);
}

CReactorSelect::~CReactorSelect()
{
}

void CReactorSelect::UpdateChannel(ChannelPtr pChannel)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	SOCKET socket = (SOCKET)pChannel->GetSocket();
	auto itChannel = m_mapChannels.find(socket);
	if (itChannel != m_mapChannels.end())
	{
		if (pChannel->IsNoneEvent())
		{
			m_bIsReadReset = true;
			m_bIsWriteReset = true;
			m_bIsExceptReset = true;
			m_mapChannels.erase(socket);
		}
		else
		{
			m_bIsReadReset = true;
			m_bIsWriteReset = true;
		}
	}
    else if (!pChannel->IsNoneEvent())//当这个 Channel 还没有加入监听集合，并且确实有需要监听的事件时，把它添加进去
	{
		m_mapChannels.emplace(socket, pChannel);
		m_bIsReadReset = true;
		m_bIsWriteReset = true;
		m_bIsExceptReset = true;
	}
}

void CReactorSelect::RemoveChannel(ChannelPtr& pChannel)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	SOCKET socket = (SOCKET)pChannel->GetSocket();
	auto itChannel = m_mapChannels.find(socket);
	if (itChannel != m_mapChannels.end())
	{
		m_bIsReadReset = true;
		m_bIsWriteReset = true;
		m_bIsExceptReset = true;
		m_mapChannels.erase(itChannel);
	}
}

//根据当前所有 Channel 生成 select() 需要的 Socket 集合，调用 select() 检测事件，再把就绪事件分发给对应的 Channel。
/*Channel是否发生变化？
        ↓
按需重建fd_set
        ↓
从备份恢复完整fd_set
        ↓
调用select()
        ↓
检查哪些Socket就绪
        ↓
转换成EVENT_IN / EVENT_OUT / EVENT_HUP
        ↓
      释放锁
        ↓
调用Channel::HandleEvent()
 */
bool CReactorSelect::bHandleEvent()
{
    fd_set fdRead;//读事件
    fd_set fdWrite;//写事件
    fd_set fdExcept;//异常事件
	FD_ZERO(&fdRead);
	FD_ZERO(&fdWrite);
    FD_ZERO(&fdExcept);//清空

	bool bIsReadReset = false;
	bool bIsWriteReset = false;
	bool bIsExceptReset = false;
    //判断是否需要重建集合
    //有为true，表示 m_mapChannels 中的监听事件发生了变化。
	if (m_bIsReadReset || m_bIsWriteReset || m_bIsExceptReset)
	{
        if (m_bIsExceptReset)//新增或者删除 Socket 时，需要重新计算最大的 Socket。
		{
			m_socketMax = 0;
		}

		std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& itemChannel : m_mapChannels)//遍历所有 Channel
		{
            int nEvents = itemChannel.second->GetEvents();//返回这个 Channel 当前监听的事件掩码
			SOCKET socket = (SOCKET)itemChannel.second->GetSocket();

			if (m_bIsReadReset && (nEvents & EVENT_IN))
			{
				FD_SET(socket, &fdRead);
			}

			if (m_bIsWriteReset && (nEvents & EVENT_OUT))
			{
				FD_SET(socket, &fdWrite);
			}

			if (m_bIsExceptReset)
			{
				FD_SET(socket, &fdExcept);
                if (socket > m_socketMax)//所有已注册 Socket 都加入异常集合，同时重新计算 m_socketMax。
				{
					m_socketMax = socket;
				}
			}
		}

        //最后保存本轮到底重建了哪些集合，然后清除成员标记，表示集合已经更新完毕
		bIsReadReset = m_bIsReadReset;
		bIsWriteReset = m_bIsWriteReset;
		bIsExceptReset = m_bIsExceptReset;
		m_bIsReadReset = false;
		m_bIsWriteReset = false;
		m_bIsExceptReset = false;
	}

    //保存或恢复备份集合
    if (bIsReadReset)//是读事件，将读集合中的所有描述符保存备份，避免下次循环时只监听这次发生变化的描述符
	{
		memcpy(&m_fdReadBackup, &fdRead, sizeof(fd_set));
	}
    else//不是读事件，将上一次循环备份的描述符恢复备份
	{
		memcpy(&fdRead, &m_fdReadBackup, sizeof(fd_set));
	}

	if (bIsWriteReset)
	{
		memcpy(&m_fdWriteBackup, &fdWrite, sizeof(fd_set));
	}
	else
	{
		memcpy(&fdWrite, &m_fdWriteBackup, sizeof(fd_set));
	}

	if (bIsExceptReset)
	{
		memcpy(&m_fdExceptBackup, &fdExcept, sizeof(fd_set));
	}
	else
	{
		memcpy(&fdExcept, &m_fdExceptBackup, sizeof(fd_set));
	}

    //调用 select() 检测事件
    //timeout = {0, 0} 表示：不等待，立即返回。当前没有事件时返回 0。所以这个函数是非阻塞轮询
	struct timeval timeout = { 0, 0 };
	int nReadyCount = select((int)m_socketMax + 1, &fdRead, &fdWrite, &fdExcept, &timeout);
    if (nReadyCount < 0)//< 0：select() 调用失败；== 0：本轮没有事件；> 0：存在就绪的 Socket。
	{
		return false;
	}

    //将就绪 Socket 转换成项目事件
    //这里先保存“Channel + 已发生事件”，暂时不直接调用回调
	std::forward_list<std::pair<ChannelPtr, int>> listEvents;
	if (nReadyCount > 0)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (const auto& itemChannel : m_mapChannels)
		{
			int nEvents = 0;
			SOCKET socket = (SOCKET)itemChannel.second->GetSocket();
            //判断是什么事件，一个 Socket 可以同时出现多个事件，因此使用按位或
			if (FD_ISSET(socket, &fdRead))
			{
				nEvents |= EVENT_IN;
			}

			if (FD_ISSET(socket, &fdWrite))
			{
				nEvents |= EVENT_OUT;
			}

			if (FD_ISSET(socket, &fdExcept))
			{
				nEvents |= EVENT_HUP;
			}

            //如果确实有事件，就保存
			if (nEvents != 0)
			{
				listEvents.emplace_front(itemChannel.second, nEvents);
			}
		}
	}

    /* 这里特意先收集事件，再释放 m_mutex，最后才调用 HandleEvent()。
        因为事件回调中可能会：
        删除 Channel。
        更新 Channel。
        关闭 Socket。
        再次调用 UpdateChannel() 或 RemoveChannel()。
        如果持有 m_mutex 时执行回调，回调再次申请同一个锁就可能死锁。
     */
	for (const auto& itemEvent : listEvents)
	{
		itemEvent.first->HandleEvent(itemEvent.second);
	}

	return true;
}

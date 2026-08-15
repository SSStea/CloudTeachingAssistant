#include "MainWidget.h"
#include "RemoteWidget.h"
#include "LoginWidget.h"

CMainWidget::CMainWidget(QWidget *parent)
    : QWidget{parent}
{
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground);
    setFixedSize(700, 610);

    m_stackWgt = new QStackedWidget(this);
    m_stackWgt->setFixedSize(700, 610);

    m_login = new CLoginWidget(this);
    m_login->setFixedSize(700, 610);
    //m_login->setStyleSheet("background-color: #FFFFFF");

    m_remoteWgt = new CRemoteWidget(this);
    m_remoteWgt->setFixedSize(700, 610);
    //m_remoteWgt->setStyleSheet("background-color: #344522");

    m_deviceWgt = new QWidget(this);
    m_deviceWgt->setFixedSize(700, 610);
    m_deviceWgt->setStyleSheet("background-color: #664764");

    m_settingWgt = new QWidget(this);
    m_settingWgt->setFixedSize(700, 610);
    m_settingWgt->setStyleSheet("background-color: #957522");

    m_stackWgt->addWidget(m_login);
    m_stackWgt->addWidget(m_remoteWgt);
    m_stackWgt->addWidget(m_deviceWgt);
    m_stackWgt->addWidget(m_settingWgt);

    //指定显示哪个窗口
    m_stackWgt->setCurrentWidget(m_login);
}

void CMainWidget::SlotItemClicked(int nIndex)
{
    QWidget* widget = m_stackWgt->widget(nIndex);
    if(widget)
    {
        m_stackWgt->setCurrentWidget(widget);
    }
}

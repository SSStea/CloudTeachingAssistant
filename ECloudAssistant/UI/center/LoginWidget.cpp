#include "LoginWidget.h"
#include <QVBoxLayout>
#include "StyleLoader.h"
#include <QDateTime>

uint64_t GetTimeStamp()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    qint64 timestamp = currentTime.toSecsSinceEpoch();
    return static_cast<uint64_t>(timestamp);
}

CLoginWidget::CLoginWidget(QWidget *parent)
    : QWidget{parent}
{
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground);
    setFixedSize(700, 610);

    m_accountEdit = new QLineEdit(this);
    m_passwordEdit = new QLineEdit(this);
    m_loginBtn = new QPushButton(QString("登录"), this);

    connect(m_loginBtn, &QPushButton::clicked, this, [this](){
        if(m_socket && m_bIsConnect)
        {
            Login_Info info;
            info.timestamp = GetTimeStamp();
            m_socket->write((const char*)&info, info.len);
            m_socket->flush();
        }
    });

    this->setObjectName("Loginer");
    m_accountEdit->setObjectName("AcountEdit");
    m_passwordEdit->setObjectName("PasswdEdit");
    m_loginBtn->setObjectName("login_Btn");

    m_accountEdit->setPlaceholderText(QString("请输入账号"));
    m_passwordEdit->setPlaceholderText(QString("请输入密码"));

    m_passwordEdit->setEchoMode(QLineEdit::Password);

    //创建套接字
    m_socket = new QTcpSocket(this);
    //关联信号与槽
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(ReadData()));
    //连接负载
    m_socket->connectToHost("172.20.108.206", 8523);
    if(m_socket->waitForConnected(1000))
    {
        m_bIsConnect = true;
        qDebug() << "连接负载成功";
    }

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addStretch(3);
    layout->addWidget(m_accountEdit, 0, Qt::AlignCenter);
    layout->addSpacing(40);
    layout->addWidget(m_passwordEdit, 1, Qt::AlignCenter);
    layout->addWidget(m_loginBtn, 2, Qt::AlignCenter);
    layout->addStretch(1);

    setLayout(layout);

    CStyleLoader::GetInstance()->LoadStyle(":/UI/brown/main.css", this);
}

void CLoginWidget::ReadData()
{
    QByteArray buf = m_socket->readAll();
    if(!buf.isEmpty())
    {
        HandleMessage((packet_head*)buf.data());
    }
}

void CLoginWidget::HandleMessage(const packet_head *data)
{
    switch (data->cmd) {
    case Login:
        HandleLogin((LoginResult*)data);
        break;

    case Register:
        HandleRegister((RegisterResult*)data);
        break;

    case ERROR_:
        HandleError((packet_head*)data);
        break;
    default:
        break;
    }
}

void CLoginWidget::HandleRegister(RegisterResult *data)
{

}

void CLoginWidget::HandleLogin(LoginResult *data)
{
    if(m_bIsLogin)
    {
        if(data->resultCode == S_OK_)
        {
            std::string sigserver = data->GetIp();
            qDebug() << "login success, sigserver ip:" << sigserver.c_str()
                     << "port:" << data->port;
        }
    }
    else
    {
        HandleLoadLogin((LoginReply*)data);
    }

}

void CLoginWidget::HandleError(const packet_head *data)
{
    qDebug() << "error";
}

void CLoginWidget::HandleLoadLogin(LoginReply *data)
{
    m_strIP = QString(data->ip.data());
    m_nPort = data->port;
    qDebug() << "login ip :" << m_strIP << "port: " << m_nPort;

    m_socket->disconnectFromHost();
    m_bIsConnect = false;

    m_socket->connectToHost(m_strIP, m_nPort);
    if(m_socket->waitForConnected(1000))
    {
        m_bIsConnect = true;
        m_bIsLogin = true;

        qDebug() << "login server success";

        //开始登录
        UserLogin login;
        QString strAccount = m_accountEdit->text();
        QString strPasswd = m_passwordEdit->text();

        login.SetCode("123");
        login.SetCount(strAccount.toStdString());
        login.SetPasswd(strPasswd.toStdString());
        login.timestamp = GetTimeStamp();
        m_socket->write((const char*)&login, login.len);
        m_socket->flush();
    }
}

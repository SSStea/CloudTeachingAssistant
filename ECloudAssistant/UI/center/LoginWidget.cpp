#include "LoginWidget.h"
#include <QVBoxLayout>
#include "StyleLoader.h"

CLoginWidget::CLoginWidget(QWidget *parent)
    : QWidget{parent}
{
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground);
    setFixedSize(700, 610);

    m_accountEdit = new QLineEdit(this);
    m_passwordEdit = new QLineEdit(this);
    m_loginBtn = new QPushButton(QString("登录"), this);

    this->setObjectName("Loginer");
    m_accountEdit->setObjectName("AcountEdit");
    m_passwordEdit->setObjectName("PasswdEdit");
    m_loginBtn->setObjectName("login_Btn");

    m_accountEdit->setPlaceholderText(QString("请输入账号"));
    m_passwordEdit->setPlaceholderText(QString("请输入密码"));

    m_passwordEdit->setEchoMode(QLineEdit::Password);

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

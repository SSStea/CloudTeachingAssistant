#include "RemoteWidget.h"
#include <QVBoxLayout>
#include "StyleLoader.h"

CRemoteWidget::CRemoteWidget(QWidget *parent)
    : QWidget{parent}
{
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground);
    setFixedSize(700, 610);

    m_selfCodeEdit = new QLineEdit(this);
    m_remoteCodeEdit = new QLineEdit(this);
    m_startRemoteBtn = new QPushButton(QString("开始远程"), this);

    this->setObjectName("RemoteWgt");
    m_selfCodeEdit->setObjectName("selfCodeEdit");
    m_remoteCodeEdit->setObjectName("remoteCodeEdit");
    m_startRemoteBtn->setObjectName("remoteBtn");

    m_selfCodeEdit->setPlaceholderText(QString("本机识别码"));
    m_remoteCodeEdit->setPlaceholderText(QString("远程识别码"));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addStretch(3);
    layout->addWidget(m_selfCodeEdit, 0, Qt::AlignCenter);
    layout->addSpacing(40);
    layout->addWidget(m_remoteCodeEdit, 1, Qt::AlignCenter);
    layout->addWidget(m_startRemoteBtn, 2, Qt::AlignCenter);
    layout->addStretch(1);

    setLayout(layout);

    CStyleLoader::GetInstance()->LoadStyle(":/UI/brown/main.css", this);
}

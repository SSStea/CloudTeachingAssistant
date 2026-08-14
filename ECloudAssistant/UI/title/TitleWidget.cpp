#include "TitleWidget.h"
#include <QHBoxLayout>

CTitleWidget::CTitleWidget(QWidget *parent)
    : QWidget{parent}
{
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground);
    setFixedSize(700, 30);
    setStyleSheet("background-color: #714201");

    m_minBtn = new QPushButton(this);
    m_closeBtn = new QPushButton(this);

    m_minBtn->setFixedSize(30, 30);
    m_closeBtn->setFixedSize(30, 30);

    connect(m_minBtn, &QPushButton::clicked, this , [this](){
        this->parentWidget()->showMinimized();
    });
    connect(m_closeBtn, &QPushButton::clicked, this , [this](){
        this->parentWidget()->close();
    });

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addStretch(1);
    layout->addWidget(m_minBtn);
    layout->addWidget(m_closeBtn);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);
}

#include "ECloudAssistant.h"
#include "TitleWidget.h"
#include "MainWidget.h"
#include "ListInfoWidget.h"
#include <QGridLayout>
#include <QMouseEvent>

ECloudAssistant::ECloudAssistant(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(1000, 640);
    setStyleSheet("background-color: #121212");

    m_mainWgt = new CMainWidget(this);
    m_titleWgt = new CTitleWidget(this);
    m_listWgt = new CListInfoWidget(this);

    connect(m_listWgt, &CListInfoWidget::SigSelect, m_mainWgt, &CMainWidget::SlotItemClicked);

    QGridLayout* layout = new QGridLayout(this);
    layout->setSpacing(0);
    layout->addWidget(m_listWgt, 0, 0, 2, 1);
    layout->addWidget(m_titleWgt, 0, 1, 1, 2);
    layout->addWidget(m_mainWgt, 1, 1, 1, 2);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

ECloudAssistant::~ECloudAssistant() {}

void ECloudAssistant::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton && m_bIsPress)
    {
        if(!qobject_cast<QPushButton*>(childAt(event->pos())))
        {
            move(event->globalPos() - m_ponit);
        }
    }
    QWidget::mouseMoveEvent(event);
}

void ECloudAssistant::mousePressEvent(QMouseEvent *event)
{
    if(!qobject_cast<QPushButton*>(childAt(event->pos())))
    {
        m_bIsPress = true;
        m_ponit = event->globalPos() - this->frameGeometry().topLeft();
    }
    QWidget::mouseMoveEvent(event);
}

void ECloudAssistant::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_bIsPress = false;
    }
    QWidget::mouseMoveEvent(event);
}

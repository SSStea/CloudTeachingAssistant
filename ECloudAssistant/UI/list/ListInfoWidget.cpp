#include "ListInfoWidget.h"
#include <QVBoxLayout>
#include "ListWidget.h"
#include "StyleLoader.h"

CListInfoWidget::CListInfoWidget(QWidget *parent)
    : QWidget{parent}
{
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground);
    setFixedSize(300, 640);
    //setStyleSheet("background-color: #563149");

    m_userBtn = new QPushButton(this);
    m_listWgt = new CListWidget(this);

    m_userBtn->setObjectName("user_Btn");
    m_listWgt->setObjectName("listWidget");

    connect(m_listWgt, &CListWidget::itemClicked, this, &CListInfoWidget::HandleItemSelect);

    connect(m_userBtn, &QPushButton::clicked, this, [this](){
        emit SigSelect(0);
        for(auto item : m_vecCustomWgts)
        {
            //取消高亮
            item->setHightLight(false);
        }
        m_listWgt->clearSelection();
    });

    m_userBtn->setFixedSize(100, 100);
    m_listWgt->setFixedSize(300, 440);

    //添加item
    CCustomWidget* rmtWgt = new CCustomWidget(this);
    CCustomWidget* dvcWgt = new CCustomWidget(this);
    CCustomWidget* setWgt = new CCustomWidget(this);

    //更新图片
    rmtWgt->setImageAndText("远程控制", ":/UI/brown/list/remote.png",
                            ":/UI/brown/list/remote_press.png", true);
    dvcWgt->setImageAndText("设备列表", ":/UI/brown/list/device.png",
                            ":/UI/brown/list/device_press.png", false);
    setWgt->setImageAndText("高级设置", ":/UI/brown/list/setting.png",
                            ":/UI/brown/list/setting_press.png", false);

    m_vecCustomWgts.push_back(rmtWgt);
    m_vecCustomWgts.push_back(dvcWgt);
    m_vecCustomWgts.push_back(setWgt);

    rmtWgt->setFixedHeight(80);
    dvcWgt->setFixedHeight(80);
    setWgt->setFixedHeight(80);

    m_listWgt->AddWidget(rmtWgt);
    m_listWgt->AddWidget(dvcWgt);
    m_listWgt->AddWidget(setWgt);


    //布局
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addSpacing(50);
    layout->addWidget(m_userBtn, 0, Qt::AlignHCenter);
    layout->addSpacing(50);
    layout->addWidget(m_listWgt, 1, Qt::AlignHCenter);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);

    CStyleLoader::GetInstance()->LoadStyle(":/UI/brown/main.css", this);
}

void CListInfoWidget::HandleItemSelect(int nIndex)
{
    if(nIndex < 0 || nIndex > m_vecCustomWgts.size() - 1)
    {
        return ;
    }

    emit SigSelect(nIndex + 1);

    for(int i = 0; i < m_vecCustomWgts.size(); i++)
    {
        if(i == nIndex)
        {
            m_vecCustomWgts[i]->setHightLight(true);
        }
        else
        {
            m_vecCustomWgts[i]->setHightLight(false);
        }
    }
}

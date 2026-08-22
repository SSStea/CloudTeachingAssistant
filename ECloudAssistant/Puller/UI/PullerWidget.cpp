#include "PullerWidget.h"
#include <QResizeEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include "define.h"
#include <QVBoxLayout>

CPullerWidget::CPullerWidget(QWidget *parent)
    : QMainWindow{parent}
{
    this->setMinimumSize(400, 250);
    this->resize(800, 500);
    //窗口标题
    setWindowTitle(QString("ECloudAssiant"));
    //窗口图标
    setWindowIcon(QIcon(":/UI/brown/center/favicon-32.ico"));
    //设置窗口背景颜色
    setStyleSheet("background-color:#121212");

    m_pPlayer.reset(new CAVPlayer(this));
    //布局
    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(m_pPlayer.get());
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);
}

void CPullerWidget::Open(const QString &strStreamPath)
{
    m_pPlayer->Open(strStreamPath);
}

void CPullerWidget::resizeEvent(QResizeEvent *event)
{
    m_pPlayer->resize(event->size());
    QMainWindow::resizeEvent(event);
}

void CPullerWidget::wheelEvent(QWheelEvent *event)
{
    //准备一个事件
    std::shared_ptr<Wheel_Body> pBody(new Wheel_Body(), [](Wheel_Body* p){
        delete p;
    });
    //获取滚轮值
    pBody->wheel = event->Wheel;
    QPoint pixelDelta = event->pixelDelta();//滚轮像素位移：保存横向和纵向滚动的像素距离，y() 表示纵向滚动像素
    QPoint angleDelta = event->angleDelta();//鼠标滚轮的角度变化：正值通常向上滚，负值通常向下滚
    pBody->wheel = (!pixelDelta.isNull() ? (pixelDelta.y() > 0 ? 1 : -1) : (angleDelta.y() > 0 ? 1 : -1));
    //发送数据包
    QMainWindow::wheelEvent(event);
}

void CPullerWidget::mouseMoveEvent(QMouseEvent *event)
{
    //准备一个事件
    std::shared_ptr<MouseMove_Body> pBody(new MouseMove_Body(), [](MouseMove_Body* p){
        delete p;
    });
    //获取鼠标移动的x,y比值
    QMainWindow::mouseMoveEvent(event);
}

void CPullerWidget::mousePressEvent(QMouseEvent *event)
{
    std::shared_ptr<Mouse_Body> pBody(new Mouse_Body(), [](Mouse_Body* p){
        delete p;
    });
    pBody->type = MouseKeyType::PRESS;
    pBody->mouseButtons = (MouseType)event->button();
    //发送
    QMainWindow::mousePressEvent(event);
}

void CPullerWidget::mouseReleaseEvent(QMouseEvent *event)
{
    std::shared_ptr<Mouse_Body> pBody(new Mouse_Body(), [](Mouse_Body* p){
        delete p;
    });
    pBody->type = MouseKeyType::RELEASE;
    pBody->mouseButtons = (MouseType)event->button();
    //发送
    QMainWindow::mouseReleaseEvent(event);
}

void CPullerWidget::keyPressEvent(QKeyEvent *event)
{
    std::shared_ptr<Key_Body> pBody(new Key_Body(), [](Key_Body* p){
        delete p;
    });
    pBody->type = MouseKeyType::PRESS;
    pBody->key = event->key();
    //发送

    QMainWindow::keyPressEvent(event);
}

void CPullerWidget::keyReleaseEvent(QKeyEvent *event)
{
    std::shared_ptr<Key_Body> pBody(new Key_Body(), [](Key_Body* p){
        delete p;
    });
    pBody->type = MouseKeyType::RELEASE;
    pBody->key = event->key();
    QMainWindow::keyReleaseEvent(event);
}

#include "ListWidget.h"
#include <QMouseEvent>

CListWidget::CListWidget(QWidget *parent)
{
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground);
    setCurrentRow(0);
}

CListWidget::~CListWidget()
{

}

void CListWidget::AddWidget(CCustomWidget *widget)
{
    QListWidgetItem* listWidgetItem = new QListWidgetItem(this);
    listWidgetItem->setSizeHint(widget->sizeHint());
    this->setItemWidget(listWidgetItem, widget);
}

void CListWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        QListWidgetItem* item = itemAt(event->pos());
        if(item)
        {
            int nIndex = row(item);
            //发送信号
            emit itemClicked(nIndex);
        }
        QListWidget::mousePressEvent(event);
    }
}

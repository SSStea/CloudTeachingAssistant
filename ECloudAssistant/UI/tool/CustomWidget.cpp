#include "CustomWidget.h"
#include <QHBoxLayout>
#include <QFont>

CCustomWidget::CCustomWidget(QWidget *parent)
    : QWidget{parent}, m_vecImagePath(2, "")
{
    setWindowFlag(Qt::FramelessWindowHint);
    setFixedSize(200, 50);
    Init();
}

CCustomWidget::~CCustomWidget()
{
    m_vecImagePath.clear();
}

void CCustomWidget::setHightLight(bool bFlag)
{
    const QString strImagePath = bFlag ? m_vecImagePath[1] : m_vecImagePath[0];
    setPicture(strImagePath);
}

void CCustomWidget::setImageAndText(const QString &strText, const QString &strNormal, const QString &strHightlight, bool bIsHigtlight)
{
    if(strNormal.isEmpty() || strHightlight.isEmpty())
    {
        return;
    }
    m_vecImagePath[0] = strNormal;
    m_vecImagePath[1] = strHightlight;
    const QString strImagePath = bIsHigtlight ? strHightlight : strNormal;
    setPicture(strImagePath);
    setLableTxt(strText);
}

void CCustomWidget::Init()
{
    m_nameLbl = new QLabel(this);
    m_imageLbl = new QLabel(this);

    m_nameLbl->setFixedSize(150, 50);
    m_imageLbl->setFixedSize(40, 40);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(m_imageLbl);
    layout->addWidget(m_nameLbl);
    setLayout(layout);
}

void CCustomWidget::setLableTxt(const QString &text)
{
    m_nameLbl->setText(text);
    QFont font("Microsoft Yahei", 11);
    m_nameLbl->setFont(font);
    m_nameLbl->setStyleSheet("color: #ffffff");
}

void CCustomWidget::setPicture(const QString &imagepath)
{
    QPixmap pixmap;
    if(pixmap.load(imagepath))
    {
        m_imageLbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_imageLbl->setPixmap(pixmap.scaled(m_imageLbl->size(),
            Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

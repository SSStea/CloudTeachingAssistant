#ifndef CUSTOMWIDGET_H
#define CUSTOMWIDGET_H

#include <QWidget>
#include <QLabel>
#include <vector>

class CCustomWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CCustomWidget(QWidget *parent = nullptr);
    ~CCustomWidget();
    void setHightLight(bool bFlag = true);
    void setImageAndText(const QString& strText,const QString& strNormal,const QString& strHightlight,bool bIsHigtlight = false);
protected:
    void Init();
    void setLableTxt(const QString& text);
    void setPicture(const QString& imagepath);
private:
    QLabel* m_nameLbl;
    QLabel* m_imageLbl;
    std::vector<QString> m_vecImagePath; //[0] 存放非高亮图片路径 [1]高亮
};

#endif // CUSTOMWIDGET_H

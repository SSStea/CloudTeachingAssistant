#ifndef LISTINFOWIDGET_H
#define LISTINFOWIDGET_H

#include <QWidget>
#include <QPushButton>

class CListWidget;
class CCustomWidget;
class CListInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CListInfoWidget(QWidget *parent = nullptr);

signals:
    void SigSelect(int nIndex);

protected slots:
    void HandleItemSelect(int nIndex);

private:
    QPushButton* m_userBtn;
    CListWidget* m_listWgt;

    std::vector<CCustomWidget*> m_vecCustomWgts;
};

#endif // LISTINFOWIDGET_H

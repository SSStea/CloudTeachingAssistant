#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QStackedWidget>

class CLoginWidget;
class CRemoteWidget;

class CMainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CMainWidget(QWidget *parent = nullptr);

public slots:
    void SlotItemClicked(int nIndex);

private:
    QStackedWidget* m_stackWgt;
    CLoginWidget* m_login;
    CRemoteWidget* m_remoteWgt;
    QWidget* m_deviceWgt;
    QWidget* m_settingWgt;
};

#endif // MAINWIDGET_H

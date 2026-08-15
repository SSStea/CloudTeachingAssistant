#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTcpSocket>
#include "define.h"

class CLoginWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CLoginWidget(QWidget *parent = nullptr);

protected slots:
    void ReadData();
    void HandleMessage(const packet_head* data);
protected:
    void HandleRegister(RegisterResult* data);
    void HandleLogin(LoginResult* data);
    void HandleError(const packet_head* data);
    void HandleLoadLogin(LoginReply* data);

private:
    QLineEdit* m_accountEdit;
    QLineEdit* m_passwordEdit;
    QPushButton* m_loginBtn;

    QString m_strIP;
    uint16_t m_nPort;
    bool m_bIsLogin = false;
    bool m_bIsConnect = false;
    QTcpSocket* m_socket = nullptr;
};

#endif // LOGINWIDGET_H

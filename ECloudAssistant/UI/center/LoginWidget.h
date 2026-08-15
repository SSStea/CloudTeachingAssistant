#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

class CLoginWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CLoginWidget(QWidget *parent = nullptr);

private:
    QLineEdit* m_accountEdit;
    QLineEdit* m_passwordEdit;
    QPushButton* m_loginBtn;
};

#endif // LOGINWIDGET_H

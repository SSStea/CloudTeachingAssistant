#ifndef REMOTEWIDGET_H
#define REMOTEWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

class CRemoteWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CRemoteWidget(QWidget *parent = nullptr);

private:
    QLineEdit* m_selfCodeEdit;
    QLineEdit* m_remoteCodeEdit;
    QPushButton* m_startRemoteBtn;
};

#endif // REMOTEWIDGET_H

#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include <QWidget>
#include <QPushButton>

class CTitleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CTitleWidget(QWidget *parent = nullptr);

private:
    QPushButton* m_minBtn;
    QPushButton* m_closeBtn;
};

#endif // TITLEWIDGET_H

#ifndef ECLOUDASSISTANT_H
#define ECLOUDASSISTANT_H

#include <QWidget>

class CTitleWidget;
class CMainWidget;
class CListInfoWidget;

class ECloudAssistant : public QWidget
{
    Q_OBJECT

public:
    ECloudAssistant(QWidget *parent = nullptr);
    ~ECloudAssistant();

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    CMainWidget* m_mainWgt;
    CTitleWidget* m_titleWgt;
    CListInfoWidget* m_listWgt;

    QPoint m_ponit;
    bool m_bIsPress;
};
#endif // ECLOUDASSISTANT_H

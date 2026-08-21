#ifndef PULLERWIDGET_H
#define PULLERWIDGET_H

#include <QMainWindow>

class CPullerWidget : public QMainWindow
{
    Q_OBJECT
public:
    explicit CPullerWidget(QWidget *parent = nullptr);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
};

#endif // PULLERWIDGET_H

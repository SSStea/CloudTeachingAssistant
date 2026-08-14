#include <QListWidget>
#include "CustomWidget.h"

class CCustomWidget;
class CListWidget : public QListWidget
{
    Q_OBJECT
public:
    CListWidget(QWidget* parent = nullptr);
    ~CListWidget();
    void AddWidget(CCustomWidget *widget);
signals:
    void itemClicked(int nIndex);
protected:
    void mousePressEvent(QMouseEvent* event)override;
private:

};

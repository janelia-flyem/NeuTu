#ifndef ZBODYLISTWIDGET_H
#define ZBODYLISTWIDGET_H

#include <QWidget>

namespace Ui {
class ZBodyListWidget;
}

class ZBodyListWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ZBodyListWidget(QWidget *parent = 0);
  ~ZBodyListWidget();

private:
  Ui::ZBodyListWidget *ui;
};

#endif // ZBODYLISTWIDGET_H

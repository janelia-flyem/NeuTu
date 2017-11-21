#ifndef FLYEMBODYINFOWIDGET_H
#define FLYEMBODYINFOWIDGET_H

#include <QWidget>

class FlyEmBodyInfoDialog;

namespace Ui {
class FlyEmBodyInfoWidget;
}

class FlyEmBodyInfoWidget : public QWidget
{
  Q_OBJECT

public:
  explicit FlyEmBodyInfoWidget(QWidget *parent = 0);
  ~FlyEmBodyInfoWidget();


private:
  Ui::FlyEmBodyInfoWidget *ui;
  FlyEmBodyInfoDialog *m_mainWidget;
};

#endif // FLYEMBODYINFOWIDGET_H

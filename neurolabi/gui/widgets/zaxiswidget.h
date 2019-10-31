#ifndef ZAXISWIDGET_H
#define ZAXISWIDGET_H

#include <QWidget>

#include "common/neutudefs.h"

namespace Ui {
class ZAxisWidget;
}

class ZAxisWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ZAxisWidget(QWidget *parent = 0);
  ~ZAxisWidget();

  neutu::EAxis getAxis() const;

private:
  Ui::ZAxisWidget *ui;
};

#endif // ZAXISWIDGET_H

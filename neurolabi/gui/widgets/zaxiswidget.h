#ifndef ZAXISWIDGET_H
#define ZAXISWIDGET_H

#include <QWidget>

#include "core/neutube_def.h"

namespace Ui {
class ZAxisWidget;
}

class ZAxisWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ZAxisWidget(QWidget *parent = 0);
  ~ZAxisWidget();

  neutube::EAxis getAxis() const;

private:
  Ui::ZAxisWidget *ui;
};

#endif // ZAXISWIDGET_H

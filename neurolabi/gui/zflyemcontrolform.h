#ifndef ZFLYEMCONTROLFORM_H
#define ZFLYEMCONTROLFORM_H

#include <QWidget>

namespace Ui {
class ZFlyEmControlForm;
}

class ZFlyEmControlForm : public QWidget
{
  Q_OBJECT

public:
  explicit ZFlyEmControlForm(QWidget *parent = 0);
  ~ZFlyEmControlForm();

private:
  Ui::ZFlyEmControlForm *ui;
};

#endif // ZFLYEMCONTROLFORM_H

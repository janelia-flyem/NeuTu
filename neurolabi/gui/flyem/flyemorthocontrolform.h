#ifndef FLYEMORTHOCONTROLFORM_H
#define FLYEMORTHOCONTROLFORM_H

#include <QWidget>

namespace Ui {
class FlyEmOrthoControlForm;
}

class FlyEmOrthoControlForm : public QWidget
{
  Q_OBJECT

public:
  explicit FlyEmOrthoControlForm(QWidget *parent = 0);
  ~FlyEmOrthoControlForm();

signals:
  void movingUp();
  void movingDown();
  void movingLeft();
  void movingRight();

private:
  void connectSignalSlot();

private:
  Ui::FlyEmOrthoControlForm *ui;
};

#endif // FLYEMORTHOCONTROLFORM_H

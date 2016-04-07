#ifndef FLYEMORTHOCONTROLFORM_H
#define FLYEMORTHOCONTROLFORM_H

#include <QWidget>

namespace Ui {
class FlyEmOrthoControlForm;
}

class ZFlyEmMessageWidget;
class ZWidgetMessage;

class FlyEmOrthoControlForm : public QWidget
{
  Q_OBJECT

public:
  explicit FlyEmOrthoControlForm(QWidget *parent = 0);
  ~FlyEmOrthoControlForm();

  ZFlyEmMessageWidget* getMessageWidget() const;

signals:
  void movingUp();
  void movingDown();
  void movingLeft();
  void movingRight();
  void locatingMain();

public slots:
  void dump(const ZWidgetMessage &message);

private:
  void connectSignalSlot();

private:
  Ui::FlyEmOrthoControlForm *ui;
};

#endif // FLYEMORTHOCONTROLFORM_H

#ifndef ZDOWNSAMPLINGMODULE_H
#define ZDOWNSAMPLINGMODULE_H
#include <QWidget>
#include "zsandboxmodule.h"



class QPushButton;
class QSpinBox;
class ZStack;
class ZDownSamplingWindow:public QWidget
{
  Q_OBJECT
public:
  ZDownSamplingWindow(QWidget *parent = 0);
private slots:
  void onDownSampling();
  void onRecover();
private:
  QPushButton*  down;
  QPushButton*  recover;
  QSpinBox*     spin_step;
  int step;
  ZStack*  original;
};

class ZStack;
class ZDownSamplingModule : public ZSandboxModule
{
  Q_OBJECT
public:
  explicit ZDownSamplingModule(QObject *parent = 0);
  ~ZDownSamplingModule();
signals:

public slots:

private slots:
  void execute();

private:
  void init();
private:
  ZDownSamplingWindow* window;
};


#endif // ZDOWNSAMPLINGMODULE_H

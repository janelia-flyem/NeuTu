#ifndef ZWATERSHEDMODULE
#define ZWATERSHEDMODULE
#include <QWidget>
#include"zsandboxmodule.h"

class ZStack;


class QPushButton;
class QSpinBox;
class ZStack;
class QComboBox;

class ZWaterShedWindow:public QWidget
{
  Q_OBJECT
public:
  ZWaterShedWindow(QWidget *parent = 0);
private slots:
  void onOk();
  void onCancel();
private:
  QPushButton*  ok;
  QPushButton*  cancel;
  QSpinBox*     spin_step;
  QComboBox*   algorithms;
  QComboBox*   ds_method;

};

class ZMultiscaleWaterShedModule:public ZSandboxModule
{
  Q_OBJECT
  public:
  explicit ZMultiscaleWaterShedModule(QObject *parent = 0);
    ~ZMultiscaleWaterShedModule();
  signals:
  private slots:
    void execute();
  private:
    void init();
  private:
    ZWaterShedWindow* window;
};

#endif // ZWATERSHEDMODULE


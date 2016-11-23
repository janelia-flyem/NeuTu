#ifndef ZWATERSHEDMODULE
#define ZWATERSHEDMODULE

#include"zsandboxmodule.h"

class ZStack;

class ZWaterShedModule:public ZSandboxModule
{
  Q_OBJECT
  public:
  explicit ZWaterShedModule(QObject *parent = 0);
    ~ZWaterShedModule();
  signals:
  private slots:
    void execute();
  private:
    void init();
};

#endif // ZWATERSHEDMODULE


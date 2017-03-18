#ifndef ZSURFRECONMODULE_H
#define ZSURFRECONMODULE_H
#include "zsandboxmodule.h"

class ZSurfReconModule : public ZSandboxModule
{
  Q_OBJECT
public:
  explicit ZSurfReconModule(QObject *parent = 0);

signals:

public slots:

private slots:
  void execute();

private:
  void init();

};
#endif // ZSURFRECONMODULE_H

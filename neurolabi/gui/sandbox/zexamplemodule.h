#ifndef ZEXAMPLEMODULE_H
#define ZEXAMPLEMODULE_H

#include "zsandboxmodule.h"

class ZExampleModule : public ZSandboxModule
{
  Q_OBJECT
public:
  explicit ZExampleModule(QObject *parent = nullptr);

signals:

public slots:

private slots:
  void execute();

private:
  void init();

};

#endif // ZEXAMPLEMODULE_H

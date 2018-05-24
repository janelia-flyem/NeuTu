#ifndef ZTRACEMODULE_H
#define ZTRACEMODULE_H

#include "zsandboxmodule.h"

class ZTraceModule : public ZSandboxModule
{
  Q_OBJECT
public:
  explicit ZTraceModule(QObject *parent = 0);

signals:

public slots:

private slots:
  void computeSeed();

private:
  void init();

};

#endif // ZTRACEMODULE_H

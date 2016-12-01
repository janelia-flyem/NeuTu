#ifndef ZABOUTMODULE_H
#define ZABOUTMODULE_H

#include "zsandboxmodule.h"

class ZAboutModule : public ZSandboxModule
{
  Q_OBJECT
public:
  explicit ZAboutModule(QObject *parent = 0);

signals:

public slots:

private slots:
  void execute();

private:
  void init();

};

#endif // ZABOUTMODULE_H

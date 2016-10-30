#ifndef ZPIXELSMODULE_H
#define ZPIXELSMODULE_H

#include "zsandboxmodule.h"
class ZPixelsModule : public ZSandboxModule
{
    Q_OBJECT
  public:
    explicit ZPixelsModule(QObject *parent = 0);

  signals:

  public slots:

  private slots:
    void execute();

  private:
    void init();
};

#endif // ZPIXELSMODULE_H

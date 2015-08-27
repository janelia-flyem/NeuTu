#ifndef ZFLYEMBODYWINDOWFACTORY_H
#define ZFLYEMBODYWINDOWFACTORY_H

#include "zwindowfactory.h"

class ZFlyEmBodyWindowFactory : public ZWindowFactory
{
public:
  ZFlyEmBodyWindowFactory();

protected:
  void configure(Z3DWindow *window);
};

#endif // ZFLYEMBODYWINDOWFACTORY_H

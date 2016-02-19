#ifndef ZARRAYFACTORY_H
#define ZARRAYFACTORY_H

#include "zarray.h"

class ZStack;

class ZArrayFactory
{
public:
  ZArrayFactory();

  static ZArray* MakeArray(const ZStack *stack);
};

#endif // ZARRAYFACTORY_H

#ifndef ZARRAYFACTORY_H
#define ZARRAYFACTORY_H

#include "zarray.h"

class ZStack;
class ZIntCuboid;

class ZArrayFactory
{
public:
  ZArrayFactory();

  static ZArray* MakeArray(const ZStack *stack);
  static ZArray* MakeArray(const ZIntCuboid box, mylib::Value_Type type);
};

#endif // ZARRAYFACTORY_H

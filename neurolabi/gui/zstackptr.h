#ifndef ZSTACKPTR_H
#define ZSTACKPTR_H

#include "zsharedpointer.h"

class ZStack;

class ZStackPtr : public ZSharedPointer<ZStack>
{
public:
  ZStackPtr();
  ZStackPtr(ZStack *stack);

  static ZStackPtr Make();
};

#endif // ZSTACKPTR_H

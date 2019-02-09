#ifndef ZSTACKPTR_H
#define ZSTACKPTR_H

#include <memory>

class ZStack;

class ZStackPtr : public std::shared_ptr<ZStack>
{
public:
  ZStackPtr();
  ZStackPtr(ZStack *stack);

  static ZStackPtr Make();
};

#endif // ZSTACKPTR_H

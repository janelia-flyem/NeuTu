#ifndef ZSTACKPRINTER_H
#define ZSTACKPRINTER_H

#include "c_stack.h"
class ZStack;

class ZStackPrinter
{
public:
  ZStackPrinter();

  void setDetailLevel(int level);

  void print(const Stack *stack);
  void print(const ZStack *stack);

private:
  int m_detailLevel = 0;
};

#endif // ZSTACKPRINTER_H

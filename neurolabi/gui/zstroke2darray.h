#ifndef ZSTROKE2DARRAY_H
#define ZSTROKE2DARRAY_H

#include <QVector>
#include "zstroke2d.h"
#include "c_stack.h"

class ZStack;

class ZStroke2dArray : public QVector<ZStroke2d*>
{
public:
  ZStroke2dArray();

  void labelStack(Stack *stack);
};

#endif // ZSTROKE2DARRAY_H

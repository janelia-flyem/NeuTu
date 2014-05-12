#include "zstroke2darray.h"

ZStroke2dArray::ZStroke2dArray()
{
}

void ZStroke2dArray::labelStack(Stack *stack)
{
  foreach (ZStroke2d *stroke, *this) {
    stroke->labelGrey(stack);
  }
}

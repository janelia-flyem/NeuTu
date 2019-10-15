/* tz_stack_relation.c
 *
 * 12-Dec-2007 Initial write: Ting Zhao
 */

#include "tz_stack_attribute.h"
#include "tz_stack_relation.h"

_BOOL_ Stack_Same_Size(const Stack *stack1, const Stack *stack2)
{
  if ((stack1->width == stack2->width) && (stack1->height == stack2->height) &&
      (stack1->depth == stack2->depth)) {
    return _TRUE_;
  } else {
    return _FALSE_;
  }
}

_BOOL_ Stack_Same_Kind(const Stack *stack1, const Stack *stack2)
{
  if (Stack_Kind(stack1) == Stack_Kind(stack2)) {
    return _TRUE_;
  } else {
    return _FALSE_;
  }
}

_BOOL_ Stack_Same_Attribute(const Stack *stack1, const Stack *stack2)
{
  if ((stack1 == NULL) && (stack2 == NULL)) {
    return _TRUE_;
  }

  if ((Stack_Same_Kind(stack1, stack2) == _TRUE_) && 
      (Stack_Same_Size(stack1, stack2)==_TRUE_)) {
    return _TRUE_;
  } else {
    return _FALSE_;
  }
}

_BOOL_ Stack_Identical(const Stack *stack1, const Stack *stack2)
{
  if (Stack_Same_Size(stack1, stack2) && Stack_Same_Kind(stack1, stack2)) {
    int n = Stack_Array_Bsize(stack1);
    int i;
    for (i = 0; i < n; i++) {
      if (stack1->array[i] != stack2->array[i]) {
	return _FALSE_;
      }
    }
  } else {
    return _FALSE_;
  }

  return _TRUE_;
}

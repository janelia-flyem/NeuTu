#ifndef ZSTACKFACTORY_H
#define ZSTACKFACTORY_H

#include <vector>
#include "zstack.hxx"
#include "tz_math.h"
#include "tz_stack_lib.h"

/*!
 * \brief The class of creating a stack
 */
class ZStackFactory
{
public:
  ZStackFactory();

public:
  template<class InputIterator>
  static ZStack* composite(InputIterator begin, InputIterator end);
};

template<class InputIterator>
ZStack* ZStackFactory::composite(InputIterator begin, InputIterator end)
{
  int stackNumber = 0;
  for (InputIterator iter = begin; iter != end; ++iter) {
    ++stackNumber;
  }

  if (stackNumber == 0) {
    return NULL;
  }

  std::vector<Stack*> stackArray(stackNumber);
  int **offset;
  int i;
  MALLOC_2D_ARRAY(offset, stackNumber, 3, int, i);

  int corner[3];

  i = 0;
  for (InputIterator iter = begin; iter != end; ++iter) {
    ZStack *stack = *iter;
    stackArray[i] = stack->c_stack();
    offset[i][0] = iround(stack->getOffset().x());
    offset[i][1] = iround(stack->getOffset().y());
    offset[i][2] = iround(stack->getOffset().z());
    ++i;
  }

  for (i = 0; i < 3; ++i) {
    corner[i] = offset[0][i];
  }

  for (i = 1; i < stackNumber; ++i) {
    for (int j = 0; j < 3; ++j) {
      if (corner[j] > offset[i][j]) {
        corner[j] = offset[i][j];
      }
    }
  }

  Stack *merged = Stack_Merge_M(&(stackArray[0]), stackNumber, offset, 1, NULL);

  FREE_2D_ARRAY(offset, stackNumber);

  ZStack *stack = new ZStack;
  stack->consumeData(merged);
  stack->setOffset(corner[0], corner[1], corner[2]);

  return stack;
}


#endif // ZSTACKFACTORY_H

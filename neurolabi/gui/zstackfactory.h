#ifndef ZSTACKFACTORY_H
#define ZSTACKFACTORY_H

#include <vector>
#include "zstack.hxx"
#include "tz_math.h"
#include "tz_stack_lib.h"
#include "zintcuboid.h"

class ZClosedCurve;
class ZStroke2d;


/*!
 * \brief The class of creating a stack
 */
class ZStackFactory
{
public:
  ZStackFactory();

  virtual ZStack* makeStack(ZStack *stack = NULL) const;

  /*!
   * \brief Make a virtual stack
   *
   * \return It returns NULL if any dimension is not positive.
   */
  static ZStack* makeVirtualStack(int width, int height, int depth);


  static ZStack* makeVirtualStack(const ZIntCuboid &box);

public:
  template<class InputIterator>
  static ZStack* composite(InputIterator begin, InputIterator end);

  static ZStack* makeOneStack(int width, int height, int depth);
  static ZStack* makeZeroStack(int width, int height, int depth);
  static ZStack* makeIndexStack(int width, int height, int depth);
  static ZStack* makeUniformStack(int width, int height, int depth, int v);

  static ZStack* makePolygonPicture(const ZStroke2d &stroke);

private:
  static Stack* pileMatched(const std::vector<Stack*> stackArray);
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

  bool isPilable = true;
  i = 0;
  for (InputIterator iter = begin; iter != end; ++iter) {
    ZStack *stack = *iter;
    stackArray[i] = stack->c_stack();
    offset[i][0] = iround(stack->getOffset().getX());
    offset[i][1] = iround(stack->getOffset().getY());
    offset[i][2] = iround(stack->getOffset().getZ());
    if (i > 0) {
      if (offset[i][0] != offset[i-1][0]) {
        isPilable = false;
      }
      if (offset[i][1] != offset[i-1][1]) {
        isPilable = false;
      }
      if (offset[i][2] != offset[i-1][2] + C_Stack::depth(stackArray[i-1])) {
        isPilable = false;
      }

      if (C_Stack::width(stackArray[i]) != C_Stack::width(stackArray[i-1])) {
        isPilable = false;
      }
      if (C_Stack::height(stackArray[i]) != C_Stack::height(stackArray[i-1])) {
        isPilable = false;
      }
    }

    ++i;
  }

  for (int i = 0; i < 3; ++i) {
    corner[i] = offset[0][i];
  }

  for (int i = 1; i < stackNumber; ++i) {
    for (int j = 0; j < 3; ++j) {
      if (corner[j] > offset[i][j]) {
        corner[j] = offset[i][j];
      }
    }
  }

  Stack *merged = NULL;

  if (isPilable) {
    merged = pileMatched(stackArray);
  } else {
    merged = Stack_Merge_M(&(stackArray[0]), stackNumber, offset, 1, NULL);
  }

  FREE_2D_ARRAY(offset, stackNumber);

  ZStack *stack = new ZStack;
  stack->consume(merged);
  stack->setOffset(corner[0], corner[1], corner[2]);

  return stack;
}


#endif // ZSTACKFACTORY_H

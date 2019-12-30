#ifndef ZSTACKFACTORY_H
#define ZSTACKFACTORY_H

#include <vector>

#include "common/math.h"
#include "zstack.hxx"
#include "tz_stack_lib.h"

class ZClosedCurve;
class ZStroke2d;
class ZPointArray;
class ZWeightedPointArray;
class ZObject3dScanArray;
class ZStackObject;
class ZSwcTree;
class ZObject3dScan;
class ZStackArray;
class ZArray;

/*!
 * \brief The class of creating a stack
 */
class ZStackFactory
{
public:
  ZStackFactory();
  virtual ~ZStackFactory();

  virtual ZStack* makeStack(ZStack *stack = NULL) const;

  /*!
   * \brief Make a virtual stack
   *
   * \return It returns NULL if any dimension is not positive.
   */
  static ZStack* MakeVirtualStack(int width, int height, int depth);


  static ZStack* MakeVirtualStack(const ZIntCuboid &box);

public:
  template<class InputIterator>
  static ZStack* composite(InputIterator begin, InputIterator end);

  static ZStack* MakeOneStack(int width, int height, int depth,
                              int nchannel = 1);
  static ZStack* MakeZeroStack(int width, int height, int depth,
                               int nchannel = 1);

  static ZStack* MakeZeroStack(int kind, int width, int height, int depth,
                               int nchannel);
  static ZStack* makeSlice(const ZStack& stack, int z);


  static ZStack* MakeZeroStack(const ZIntCuboid box, int nchannel = 1);
  static ZStack* MakeZeroStack(int kind, const ZIntCuboid box, int nchannel = 1);

  static ZStack* makeIndexStack(int width, int height, int depth);
  static ZStack* MakeUniformStack(int width, int height, int depth, int v);

  static ZStack* makePolygonPicture(const ZStroke2d &stroke);

  static ZStack* MakeDensityMap(const ZPointArray &ptArray, double sigma);
  static ZStack* MakeDensityMap(
      const ZWeightedPointArray &ptArray, double sigma);

  static ZStack* MakeSeedStack(const ZWeightedPointArray &ptArray);
  static ZStack* MakeSeedStack(const ZObject3dScanArray &objArray);
  static ZStack* MakeBinaryStack(const ZObject3dScanArray &objArray, int v = 1);
  static ZStack* MakeColorStack(const ZObject3dScanArray &objArray);

//  static ZStack* MakeLabelStack(ZSwcTree *obj, int v);

  /*!
   * \brief Only support GREY data
   */
  static ZStack* makeAlphaBlend(const ZStack &stack1, const ZStack &stack2,
                                double alpha);

  static ZStack* MakeColorStack(const ZStack &stack, double h, double s);
  static ZStack* MakeColorStack(
      const ZStack &stack, const ZStack &mask, double h, double s);

  static ZStack* MakeColorStack(const ZStack &stack, const ZStack &labelField);
  static ZStack* MakeRgbStack(
      const ZStack &redStack, const ZStack &greenStack, const ZStack &blueStack);

  static ZStack* CompositeForeground(const ZStack &stack1, const ZStack &stack2);
  static ZStack* CompositeForeground(const ZStackArray &stackArray);

  static ZStack* MakeStrokeMask(const std::vector<ZStroke2d*> strokeList);
  static ZStack* MakeStrokeProjMask(const std::vector<ZStroke2d*> strokeList);
  static ZStack* MakeStrokeProjMask(
      const std::vector<ZStroke2d*> strokeList, int value);
  static ZStack* MakeStrokeMask(const std::vector<ZStroke2d*> strokeList, int z);

  static ZStack* Compose(const std::vector<ZStack*> &stackArray);

  static ZStack* MakeLabelBinaryStack(
      const std::vector<ZArray*> &labelArray, uint64_t v);
  static ZStack* MakeLabelColorStack(const std::vector<ZArray*> &labelArray);

  static ZStack* LoadFromFile(const std::string &path);

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
    offset[i][0] = stack->getOffset().getX();
    offset[i][1] = stack->getOffset().getY();
    offset[i][2] = stack->getOffset().getZ();
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

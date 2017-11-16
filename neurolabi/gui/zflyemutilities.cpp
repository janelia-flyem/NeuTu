#include "zflyemutilities.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include "zstring.h"
#include "zintpoint.h"
#include "zstack.hxx"

double flyem::GetFlyEmRoiMarkerRadius(double s)
{
  return 10.0 + s / 2000;
}

double flyem::GetFlyEmRoiMarkerRadius(double width, double height)
{
  return GetFlyEmRoiMarkerRadius(std::min(width, height));
}

std::set<uint64_t> flyem::LoadBodySet(const std::string &input)
{
//  ZString

  std::set<uint64_t> bodySet;

  FILE *fp = fopen(input.c_str(), "r");
  if (fp != NULL) {
    ZString str;
    while (str.readLine(fp)) {
      std::vector<uint64_t> bodyArray = str.toUint64Array();
      bodySet.insert(bodyArray.begin(), bodyArray.end());
    }
    fclose(fp);
  } else {
    std::cout << "Failed to open " << input << std::endl;
  }

  return bodySet;
}

ZIntPoint flyem::FindClosestBg(const ZStack *stack, int x, int y, int z)
{
  if (stack->getIntValue(x, y, z) == 0) {
    return ZIntPoint(x, y, z);
  }

  size_t voxelCount = stack->getVoxelNumber();
#ifdef _DEBUG_2
  C_Stack::printValue(stack->c_stack());
#endif

  ZIntPoint rawCoord = ZIntPoint(x, y, z) - stack->getOffset();
  ssize_t index = C_Stack::indexFromCoord(
        rawCoord.getX(), rawCoord.getY(), rawCoord.getZ(),
        stack->width(), stack->height(), stack->depth());
  ZIntPoint pt;
  pt.invalidate();

  if (index >= 0) {
    long int *label = new long int[voxelCount];
    Stack *out = C_Stack::Bwdist(stack->c_stack(), NULL, label);
    C_Stack::kill(out);
#ifdef _DEBUG_2
    for (size_t i = 0; i < voxelCount; ++i) {
      std::cout << i << " " << label[i] << std::endl;
    }
#endif

    int labelIndex = label[index];
    if (stack->getIntValue(labelIndex) == 0) {
      int labelX = 0;
      int labelY = 0;
      int labelZ = 0;
      C_Stack::indexToCoord(labelIndex, stack->width(), stack->height(),
                            &labelX, &labelY, &labelZ);

      pt.set(labelX, labelY, labelZ);
      pt += stack->getOffset();
    }
    delete []label;
  }

  return pt;
}

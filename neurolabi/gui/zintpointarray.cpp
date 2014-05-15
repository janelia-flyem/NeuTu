#include "zintpointarray.h"

ZIntPointArray::ZIntPointArray()
{
}

void ZIntPointArray::append(const ZIntPoint &pt)
{
  push_back(pt);
}

void ZIntPointArray::append(int x, int y, int z)
{
  append(ZIntPoint(x, y, z));
}

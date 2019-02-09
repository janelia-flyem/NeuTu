#include "zlinesegmentarray.h"

ZLineSegmentArray::ZLineSegmentArray()
{
}

void ZLineSegmentArray::append(const ZPoint &v0, const ZPoint &v1)
{
  push_back(ZLineSegment(v0, v1));
}

void ZLineSegmentArray::append(const ZLineSegmentArray &lineArray)
{
  insert(end(), lineArray.begin(), lineArray.end());
}

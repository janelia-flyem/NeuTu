#include "zjsonfactory.h"
#include "zjsonarray.h"
#include "zjsonobject.h"
#include "zobject3dscan.h"

ZJsonFactory::ZJsonFactory()
{
}

ZJsonArray ZJsonFactory::makeJsonArray(const ZObject3dScan &obj)
{
  ZJsonArray array;

  ZObject3dScan::ConstSegmentIterator iterator(&obj);

  while (iterator.hasNext()) {
    const ZObject3dScan::Segment &seg = iterator.next();
    ZJsonArray segmentObj;
    segmentObj.append(seg.getZ());
    segmentObj.append(seg.getY());
    segmentObj.append(seg.getStart());
    segmentObj.append(seg.getEnd());
    array.append(segmentObj);
  }

  return array;
}

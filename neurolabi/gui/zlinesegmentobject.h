#ifndef ZLINESEGMENTOBJECT_H
#define ZLINESEGMENTOBJECT_H

#include "zlinesegment.h"
#include "zstackobject.h"

class ZLineSegmentObject : public ZStackObject
{
public:
  ZLineSegmentObject();
  void display(ZPainter &painter, int slice, EDisplayStyle option) const;


  bool isSliceVisible(int z) const;

private:
  ZLineSegment m_segment;
  double m_width;
  int m_label;
};

#endif // ZLINESEGMENTOBJECT_H

#ifndef ZLINESEGMENTOBJECT_H
#define ZLINESEGMENTOBJECT_H

#include "zlinesegment.h"
#include "zdocumentable.h"
#include "zstackdrawable.h"

class ZLineSegmentObject : public ZLineSegment, ZDocumentable, ZStackDrawable
{
public:
  ZLineSegmentObject();
  void display(ZPainter &painter, int z, Display_Style option) const;

private:
  int m_label;
};

#endif // ZLINESEGMENTOBJECT_H

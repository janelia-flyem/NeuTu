#include "zlinesegmentobject.h"
#include "zpainter.h"

ZLineSegmentObject::ZLineSegmentObject() : m_width(1.0), m_label(0)
{
}

void ZLineSegmentObject::display(
    ZPainter &painter, int slice, EDisplayStyle /*option*/) const
{
  int z = painter.getZ(slice);
  if (isSliceVisible(z)) {

  }
}

bool ZLineSegmentObject::isSliceVisible(int z) const
{
  if (isVisible()) {

  }

  return false;
}

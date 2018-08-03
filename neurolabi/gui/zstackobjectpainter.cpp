#include "zstackobjectpainter.h"
#include "zpainter.h"

ZStackObjectPainter::ZStackObjectPainter()
{
}

void ZStackObjectPainter::setDisplayStyle(ZStackObject::EDisplayStyle style)
{
  m_style = style;
}

void ZStackObjectPainter::setSliceAxis(neutube::EAxis sliceAxis)
{
  m_axis = sliceAxis;
}

void ZStackObjectPainter::paint(
    const ZStackObject *obj, ZPainter &painter, int slice,
    ZStackObject::EDisplayStyle option, neutube::EAxis sliceAxis) const
{
  if (obj != NULL) {
    if (m_painterConst) {
      painter.save();
    }

    obj->display(painter, slice, option, sliceAxis);

    if (m_painterConst) {
      painter.restore();
    }
  }
}

void ZStackObjectPainter::paint(
    const ZStackObject *obj, ZPainter &painter, int slice)
{
  paint(obj, painter, slice, m_style, m_axis);
}

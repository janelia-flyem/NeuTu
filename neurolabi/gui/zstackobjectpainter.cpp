#include "zstackobjectpainter.h"
#include "zpainter.h"

ZStackObjectPainter::ZStackObjectPainter()
{
  init();
}

void ZStackObjectPainter::init()
{
  m_painterConst = true;
}

void ZStackObjectPainter::paint(
    const ZStackObject *obj, ZPainter &painter, int slice,
    ZStackObject::EDisplayStyle option, NeuTube::EAxis sliceAxis) const
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

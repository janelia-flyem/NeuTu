#include "zcrosshair.h"
#include "zpainter.h"

ZCrossHair::ZCrossHair()
{
  init();
}

void ZCrossHair::init()
{
  setTarget(ZStackObject::TARGET_WIDGET);
  m_type = GetType();
  setZOrder(5);
  useCosmeticPen(true);
}


void ZCrossHair::display(ZPainter &painter, int slice,
                         ZStackObject::EDisplayStyle style,
                         NeuTube::EAxis sliceAxis) const
{
  if (!isVisible()) {
    return;
  }

  ZPoint shiftedCenter = getCenter();
  shiftedCenter.shiftSliceAxis(sliceAxis);

  QPen pen(m_color, 1);
  pen.setCosmetic(m_usingCosmeticPen);

//  painter.get

}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZCrossHair)

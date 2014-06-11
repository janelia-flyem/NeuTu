#include "zsparseobject.h"
#include <QPen>
#include "zpainter.h"
#include "tz_math.h"

const ZLabelColorTable ZSparseObject::m_colorTable;

ZINTERFACE_DEFINE_CLASS_NAME(ZSparseObject)

ZSparseObject::ZSparseObject()
{
  setLabel(-1);
}

#if 0
void ZSparseObject::display(ZPainter &painter, int z, Display_Style option) const
{
  UNUSED_PARAMETER(option);
#if _QT_GUI_USED_
  z -= iround(painter.getOffset().z());

  QPen pen(m_color);
  painter.setPen(pen);

  size_t stripeNumber = m_obj.getStripeNumber();
  for (size_t i = 0; i < stripeNumber; ++i) {
    const ZObject3dStripe &stripe = m_obj.getStripe(i);
    if (stripe.getZ() == z || z < 0) {
      int nseg = stripe.getSegmentNumber();
      for (int j = 0; j < nseg; ++j) {
        int x0 = stripe.getSegmentStart(j);
        int x1 = stripe.getSegmentEnd(j);
        int y = stripe.getY();
        painter.drawLine(x0, y, x1, y);
      }
    }
  }
#else
  UNUSED_PARAMETER(&painter);
  UNUSED_PARAMETER(z);
  UNUSED_PARAMETER(option);
#endif
}
#endif
void ZSparseObject::setLabel(int label)
{
  m_label = label;
  m_color = m_colorTable.getColor(label);
}

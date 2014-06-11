#include "zstackdrawable.h"

double ZStackDrawable::m_defaultPenWidth = 0.5;

void ZStackDrawable::display(QPainter *painter, int z, Display_Style option) const
{
  UNUSED_PARAMETER(painter);
  UNUSED_PARAMETER(z);
  UNUSED_PARAMETER(option);
}

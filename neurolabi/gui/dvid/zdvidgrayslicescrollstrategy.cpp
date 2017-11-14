#include "zdvidgrayslicescrollstrategy.h"

#include "zdvidgrayslice.h"

ZDvidGraySliceScrollStrategy::ZDvidGraySliceScrollStrategy()
{
  m_graySlice = NULL;
}

int ZDvidGraySliceScrollStrategy::scroll(int slice, int step) const
{
  if (m_graySlice != NULL) {
    if (m_graySlice->hasLowresRegion()) {
      int scale = m_graySlice->getScale() * 2;
      int newSlice = (slice / scale + step) * scale;
      return getValidSlice(newSlice);
    }
  }

  return ZScrollSliceStrategy::scroll(slice, step);
}

void ZDvidGraySliceScrollStrategy::setGraySlice(ZDvidGraySlice *slice)
{
  m_graySlice = slice;
}

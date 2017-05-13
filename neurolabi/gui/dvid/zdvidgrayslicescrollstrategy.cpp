#include "zdvidgrayslicescrollstrategy.h"

#include "zdvidgrayslice.h"

ZDvidGraySliceScrollStrategy::ZDvidGraySliceScrollStrategy()
{
  m_graySlice = NULL;
}

int ZDvidGraySliceScrollStrategy::scroll(int slice, int step) const
{
  if (m_graySlice == NULL) {
    return ZScrollSliceStrategy::scroll(slice, step);
  }

  int scale = m_graySlice->getScale();
  int newSlice = (slice / scale + step) * scale;

  return getValidSlice(newSlice);
}

void ZDvidGraySliceScrollStrategy::setGraySlice(ZDvidGraySlice *slice)
{
  m_graySlice = slice;
}

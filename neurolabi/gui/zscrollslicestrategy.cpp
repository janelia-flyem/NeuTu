#include "zscrollslicestrategy.h"

ZScrollSliceStrategy::ZScrollSliceStrategy()
{
  setRange(0, 0);
}

void ZScrollSliceStrategy::setRange(int minSlice, int maxSlice)
{
  m_minSlice = minSlice;
  m_maxSlice = maxSlice;
}

int ZScrollSliceStrategy::getValidSlice(int slice) const
{
  if (slice < m_minSlice) {
    return m_minSlice;
  }

  if (slice > m_maxSlice) {
    return m_maxSlice;
  }

  return slice;
}

int ZScrollSliceStrategy::scroll(int slice, int step) const
{
  int newSlice = slice + step;

  return getValidSlice(newSlice);
}

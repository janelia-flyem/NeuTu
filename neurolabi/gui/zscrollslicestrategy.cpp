#include "zscrollslicestrategy.h"
#include "zstackviewparam.h"
#include "mvc/zstackview.h"

ZScrollSliceStrategy::ZScrollSliceStrategy(ZStackView *view) : m_view(view)
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

ZStackViewParam ZScrollSliceStrategy::scroll(
    const ZStackViewParam &param, int step) const
{
  ZStackViewParam newParam = param;
//  newParam.setSliceIndex(scroll(param.getSliceIndex(), step));

  return newParam;
}

void ZScrollSliceStrategy::scroll(int step)
{
  ZStackViewParam param = scroll(m_view->getViewParameter(), step);
//  m_view->updateViewParam(param);
}

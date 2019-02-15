#include "zarbslicescrollstrategy.h"
#include "zstackviewparam.h"
#include "mvc/zstackview.h"

ZArbSliceScrollStrategy::ZArbSliceScrollStrategy(ZStackView *view) :
  ZScrollSliceStrategy(view)
{

}

/*
void ZArbSliceScrollStrategy::scroll(int step)
{
  ZArbSliceViewParam sliceParam = m_view->getSliceViewParam();
  sliceParam.move(0, 0, step);

  m_view->updateViewParam(sliceParam);
}
*/


ZStackViewParam ZArbSliceScrollStrategy::scroll(
    const ZStackViewParam &param, int step) const
{
  return ZScrollSliceStrategy::scroll(param, step);
  /*
  ZStackViewParam viewParam = param;
  viewParam.moveSlice(step);
  */

/*
  ZArbSliceViewParam sliceParam = m_view->getSliceViewParam();
  sliceParam.move(0, 0, step);

  m_view->setSliceViewParam(sliceParam);
  ZStackViewParam viewParam = param;
  viewParam.setViewPort(sliceParam.getViewPort());
  viewParam.setZ(sliceParam.getZ());
  */

//  return viewParam;
}

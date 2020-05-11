#include "zflyemarbpresenter.h"

ZFlyEmArbPresenter::ZFlyEmArbPresenter(QWidget *parent) :
  ZFlyEmProofPresenter(parent)
{

}

/*
void ZFlyEmArbPresenter::setViewParam(const ZArbSliceViewParam &param)
{
  m_viewParam = param;
  m_mouseEventProcessor.setArbSlice(m_viewParam.getAffinePlane());
}
*/
ZFlyEmArbPresenter* ZFlyEmArbPresenter::Make(QWidget *parent)
{
  ZFlyEmArbPresenter *presenter = new ZFlyEmArbPresenter(parent);
  presenter->configKeyMap();

  return presenter;
}

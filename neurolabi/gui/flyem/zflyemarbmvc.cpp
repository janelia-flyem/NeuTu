#include "zflyemarbmvc.h"
#include "zflyemarbdoc.h"
#include "zflyemarbpresenter.h"
#include "zstackview.h"
#include "zarbslicescrollstrategy.h"

ZFlyEmArbMvc::ZFlyEmArbMvc(QWidget *parent) : ZFlyEmProofMvc(parent)
{
  init();
}

void ZFlyEmArbMvc::init()
{

}

ZFlyEmArbMvc* ZFlyEmArbMvc::Make(QWidget *parent, ZSharedPointer<ZFlyEmArbDoc> doc)
{
  ZFlyEmArbMvc *frame = new ZFlyEmArbMvc(parent);

  BaseConstruct(frame, doc, neutube::A_AXIS);
  frame->getView()->setHoverFocus(true);
  ZArbSliceScrollStrategy *strategy = new ZArbSliceScrollStrategy(frame->getView());
  frame->getView()->setScrollStrategy(strategy);
  frame->getView()->configure(ZStackView::MODE_PLAIN_IMAGE);
  frame->getView()->setMaxViewPort(1600 * 1600);

  return frame;
}

ZFlyEmArbMvc* ZFlyEmArbMvc::Make(const ZDvidTarget &target)
{
  ZFlyEmArbDoc *doc = new ZFlyEmArbDoc;
  ZFlyEmArbMvc *mvc =
      ZFlyEmArbMvc::Make(NULL, ZSharedPointer<ZFlyEmArbDoc>(doc));

  mvc->setDvidTarget(target);

  return mvc;
}

void ZFlyEmArbMvc::setDvidTarget(const ZDvidTarget &target)
{
  ZDvidReader reader;
  if (reader.open(target)) {
    clear();
    getCompleteDocument()->setDvidTarget(reader.getDvidTarget());
  }
}

ZFlyEmArbDoc* ZFlyEmArbMvc::getCompleteDocument() const
{
  return qobject_cast<ZFlyEmArbDoc*>(getDocument().get());
}

ZFlyEmArbPresenter* ZFlyEmArbMvc::getCompletePresenter() const
{
  return qobject_cast<ZFlyEmArbPresenter*>(getPresenter());
}

void ZFlyEmArbMvc::updateViewParam(const ZArbSliceViewParam &param)
{
#ifdef _DEBUG_
  std::cout << "Updating arb slice view: " << "Z=" << param.getZ() << std::endl;
#endif
  getCompletePresenter()->setViewParam(param);
  getView()->updateViewParam(param);
}

void ZFlyEmArbMvc::resetViewParam(const ZArbSliceViewParam &param)
{
  getCompletePresenter()->setViewParam(param);
  getView()->resetViewParam(param);
}

void ZFlyEmArbMvc::processViewChangeCustom(const ZStackViewParam &viewParam)
{
  emit sliceViewChanged(viewParam.getSliceViewParam());
}

void ZFlyEmArbMvc::createPresenter()
{
  if (m_doc.get() != NULL) {
    m_presenter = ZFlyEmArbPresenter::Make(this);
  }
}

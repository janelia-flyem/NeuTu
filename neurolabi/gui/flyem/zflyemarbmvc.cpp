#include "zflyemarbmvc.h"

#include <QPushButton>

#include "zflyemarbdoc.h"
#include "zflyemarbpresenter.h"
#include "mvc/zstackview.h"
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

  BaseConstruct(frame, doc, neutu::EAxis::ARB);
  frame->getView()->setHoverFocus(true);
  ZArbSliceScrollStrategy *strategy = new ZArbSliceScrollStrategy(frame->getView());
  frame->getView()->setScrollStrategy(strategy);
  frame->getView()->configure(ZStackView::EMode::PLAIN_IMAGE);
  frame->getView()->setMaxViewPort(1600 * 1600);
  frame->initViewButton();

  return frame;
}

ZFlyEmArbMvc* ZFlyEmArbMvc::Make(const ZDvidEnv &env)
{
  ZFlyEmArbDoc *doc = new ZFlyEmArbDoc;
  ZFlyEmArbMvc *mvc =
      ZFlyEmArbMvc::Make(NULL, ZSharedPointer<ZFlyEmArbDoc>(doc));

  mvc->setDvid(env);

  return mvc;
}

void ZFlyEmArbMvc::makeViewButtons()
{
  makeViewButton(EViewButton::GOTO_POSITION);
}

/*
void ZFlyEmArbMvc::setDvidTarget(const ZDvidTarget &target)
{
  ZDvidReader reader;
  if (reader.open(target)) {
    clear();
    getCompleteDocument()->setDvidTarget(reader.getDvidTarget());
//    getCompletePresenter()->allowBlinkingSegmentation(true);
    getView()->enableCustomCheckBox(0, "Blinking", getCompletePresenter(),
                                    SLOT(allowBlinkingSegmentation(bool)));
  }
}
*/

void ZFlyEmArbMvc::setDvid(const ZDvidEnv &env)
{
  clear();
  if (getCompleteDocument()->setDvid(env)) {
    getView()->enableCustomCheckBox(0, "Blinking", getCompletePresenter(),
                                    SLOT(allowBlinkingSegmentation(bool)));
    if (getRole() == ERole::ROLE_WIDGET) {
      getViewButton(EViewButton::GOTO_POSITION)->show();
    }
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

/*
void ZFlyEmArbMvc::updateViewParam(const ZArbSliceViewParam &param)
{
#ifdef _DEBUG_2
  std::cout << "Updating arb slice view: " << "Z=" << param.getZ() << std::endl;
#endif
  getCompletePresenter()->setViewParam(param);
  getView()->updateViewParam(param);
}
*/

//void ZFlyEmArbMvc::updateViewParam(const ZStackViewParam &param)
//{
//  getCompletePresenter()->setViewParam(param.getSliceViewParam());
//}

void ZFlyEmArbMvc::resetViewParam(const ZArbSliceViewParam &param)
{
//  getCompletePresenter()->setViewParam(param);
  getView()->resetViewParam(param);
}

void ZFlyEmArbMvc::processViewChangeCustom(const ZStackViewParam &viewParam)
{
//  getCompletePresenter()->setViewParam(viewParam.getSliceViewParam());
  emit sliceViewChanged(viewParam.getSliceViewParam());
}

void ZFlyEmArbMvc::createPresenter()
{
  if (m_doc.get() != NULL) {
    m_presenter = ZFlyEmArbPresenter::Make(this);
  }
}

void ZFlyEmArbMvc::zoomTo(const ZIntPoint &pt)
{
  emit locating(pt.getX(), pt.getY(), pt.getZ());
}

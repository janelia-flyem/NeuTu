#include "zflyemproofmvccontroller.h"
#include<QMainWindow>

#include "zflyemproofmvc.h"
#include "zintpoint.h"
#include "zflyemproofpresenter.h"

ZFlyEmProofMvcController::ZFlyEmProofMvcController()
{

}

void ZFlyEmProofMvcController::GoToBody(ZFlyEmProofMvc *mvc, uint64_t bodyId)
{
  mvc->locateBody(bodyId);
}

void ZFlyEmProofMvcController::GoToPosition(
    ZFlyEmProofMvc *mvc, const ZIntPoint &pos)
{
  mvc->zoomTo(pos);
}

void ZFlyEmProofMvcController::Close(ZFlyEmProofMvc *mvc)
{
  mvc->getMainWindow()->close();
//  mvc->parentWidget()->close();
}

void ZFlyEmProofMvcController::EnableHighlightMode(ZFlyEmProofMvc *mvc)
{
  mvc->getCompletePresenter()->setHighlightMode(true);
  mvc->highlightSelectedObject(true);
}

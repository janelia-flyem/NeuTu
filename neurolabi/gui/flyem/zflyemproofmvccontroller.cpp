#include "zflyemproofmvccontroller.h"
#include<QMainWindow>

#include "zflyemproofmvc.h"
#include "geometry/zintpoint.h"
#include "zflyemproofpresenter.h"
#include "zstackdocaccessor.h"
#include "zstackdocnullmenufactory.h"
#include "flyem/zflyemtododelegate.h"
#include "zintcuboidobj.h"
#include "zstackobjectsourcefactory.h"

ZFlyEmProofMvcController::ZFlyEmProofMvcController()
{

}

void ZFlyEmProofMvcController::GoToBody(ZFlyEmProofMvc *mvc, uint64_t bodyId)
{
  mvc->locateBody(bodyId);
}

void ZFlyEmProofMvcController::SelectBody(ZFlyEmProofMvc *mvc, uint64_t bodyId)
{
  mvc->selectBody(bodyId);
}

void ZFlyEmProofMvcController::SelectBody(
    ZFlyEmProofMvc *mvc, const QSet<uint64_t> &bodySet)
{
  for (uint64_t bodyId : bodySet) {
    mvc->selectBody(bodyId);
  }
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

void ZFlyEmProofMvcController::Disable3DVisualization(ZFlyEmProofMvc *mvc)
{
  mvc->disable3D();
  mvc->notifyStateUpdate();
}

void ZFlyEmProofMvcController::DisableSequencer(ZFlyEmProofMvc *mvc)
{
  mvc->disableSequencer();
}


void ZFlyEmProofMvcController::DisableContextMenu(ZFlyEmProofMvc *mvc)
{
  mvc->getPresenter()->setContextMenuFactory(
        std::unique_ptr<ZStackDocMenuFactory>(new ZStackDocNullMenuFactory));
}

void ZFlyEmProofMvcController::SetTodoDelegate(
    ZFlyEmProofMvc *mvc, ZStackDoc *todoDoc)
{
  if (todoDoc != NULL) {
    mvc->getCompletePresenter()->setTodoDelegate(
          std::unique_ptr<ZFlyEmToDoDelegate>(new ZFlyEmToDoDelegate(todoDoc)));
  }
}

void ZFlyEmProofMvcController::UpdateProtocolRangeGlyph(
    ZFlyEmProofMvc *mvc, const ZIntCuboid &range)
{
  std::string source = ZStackObjectSourceFactory::MakeProtocolRangeSource();

  if (range.isEmpty()) {
    ZStackDocAccessor::RemoveObject(
          mvc->getDocument().get(), ZStackObject::EType::INT_CUBOID, source, true);
  } else {
    ZIntCuboidObj *obj = new ZIntCuboidObj;
    obj->setColor(QColor(255, 255, 255, 164));
    obj->setGridInterval(64);
    obj->addVisualEffect(neutu::display::Box::VE_GRID);
    obj->setSelectable(false);
    obj->setCuboid(range);
    obj->setSource(source);
    ZStackDocAccessor::AddObjectUnique(mvc->getDocument().get(), obj);
  }
}

void ZFlyEmProofMvcController::SetProtocolRangeGlyphVisible(
    ZFlyEmProofMvc *mvc, bool visible)
{
  std::string source = ZStackObjectSourceFactory::MakeProtocolRangeSource();

  ZStackDocAccessor::SetObjectVisible(
        mvc->getDocument().get(), ZStackObject::EType::INT_CUBOID, source, visible);
}



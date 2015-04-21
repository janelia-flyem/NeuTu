#include "zflyemproofmvc.h"

#include "flyem/zflyemproofdoc.h"
#include "zstackview.h"
#include "dvid/zdvidtileensemble.h"
#include "zstackpresenter.h"
#include "zdviddialog.h"

ZFlyEmProofMvc::ZFlyEmProofMvc(QWidget *parent) :
  ZStackMvc(parent)
{
}

ZFlyEmProofMvc* ZFlyEmProofMvc::Make(
    QWidget *parent, ZSharedPointer<ZFlyEmProofDoc> doc)
{
  ZFlyEmProofMvc *frame = new ZFlyEmProofMvc(parent);

  BaseConstruct(frame, doc);

  return frame;
}

ZFlyEmProofMvc* ZFlyEmProofMvc::Make(const ZDvidTarget &target)
{
  ZFlyEmProofDoc *doc = new ZFlyEmProofDoc(NULL, NULL);
  doc->setTag(NeuTube::Document::FLYEM_DVID);
  ZFlyEmProofMvc *mvc =
      ZFlyEmProofMvc::Make(NULL, ZSharedPointer<ZFlyEmProofDoc>(doc));
  mvc->getPresenter()->setObjectStyle(ZStackObject::SOLID);
  mvc->setDvidTarget(target);

  return mvc;
}

ZFlyEmProofDoc* ZFlyEmProofMvc::getCompleteDocument() const
{
  return dynamic_cast<ZFlyEmProofDoc*>(getDocument().get());
}

void ZFlyEmProofMvc::mergeSelected()
{
  if (getCompleteDocument() != NULL) {
    getCompleteDocument()->mergeSelected();
  }
}

void ZFlyEmProofMvc::undo()
{
  if (getCompleteDocument() != NULL) {
    getCompleteDocument()->undoStack()->undo();
  }
}

void ZFlyEmProofMvc::redo()
{
  if (getCompleteDocument() != NULL) {
    getCompleteDocument()->undoStack()->redo();
  }
}

void ZFlyEmProofMvc::setSegmentationVisible(bool visible)
{
  m_showSegmentation = visible;
  if (getCompleteDocument() != NULL) {
    QList<ZDvidLabelSlice*> sliceList =
        getCompleteDocument()->getDvidLabelSliceList();
    foreach (ZDvidLabelSlice *slice, sliceList) {
      slice->setVisible(visible);
      slice->update();
      /*
      if (visible) {
        slice->update();
      }
      */
    }
  }
  getView()->redrawObject();
}

void ZFlyEmProofMvc::clear()
{
  if (getCompleteDocument() != NULL) {
    getCompleteDocument()->clearData();
    getPresenter()->clearData();

  }
}

void ZFlyEmProofMvc::setDvidTarget(const ZDvidTarget &target)
{
  if (getCompleteDocument() != NULL) {
    clear();
//    getCompleteDocument()->clearData();
    getCompleteDocument()->setDvidTarget(target);
    getCompleteDocument()->updateTileData();
    QList<ZDvidTileEnsemble*> teList =
        getCompleteDocument()->getDvidTileEnsembleList();
    foreach (ZDvidTileEnsemble *te, teList) {
      te->attachView(getView());
    }
    getView()->reset(false);
  }
}

void ZFlyEmProofMvc::setDvidTarget()
{
  m_dvidDlg = new ZDvidDialog(this);
  if (m_dvidDlg->exec()) {
    const ZDvidTarget &target = m_dvidDlg->getDvidTarget();
    setDvidTarget(target);
  }
}

//void ZFlyEmProofMvc::toggleEdgeMode(bool edgeOn)



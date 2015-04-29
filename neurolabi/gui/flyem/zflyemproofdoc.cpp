#include "zflyemproofdoc.h"
#include "dvid/zdvidlabelslice.h"
#include "dvid/zdvidreader.h"
#include "zstackobjectsourcefactory.h"
#include "dvid/zdvidtileensemble.h"
#include "dvid/zdvidlabelslice.h"
#include "zstackfactory.h"

ZFlyEmProofDoc::ZFlyEmProofDoc(ZStack *stack, QObject *parent) :
  ZStackDoc(stack, parent)
{
}

void ZFlyEmProofDoc::mergeSelected()
{
  QList<ZDvidLabelSlice*> sliceList = getDvidLabelSliceList();

  ZFlyEmBodyMerger::TLabelSet labelSet;
  for (QList<ZDvidLabelSlice*>::const_iterator iter = sliceList.begin();
       iter != sliceList.end(); ++iter) {
    const ZDvidLabelSlice *labelSlice = *iter;
    const std::set<int64_t> &selected = labelSlice->getSelected();
    for (std::set<int64_t>::const_iterator iter = selected.begin();
         iter != selected.end(); ++iter) {
      labelSet.insert(*iter);
    }
  }

  m_bodyMerger.pushMap(labelSet);
  m_bodyMerger.undo();

  ZFlyEmProofDocCommand::MergeBody *command =
      new ZFlyEmProofDocCommand::MergeBody(this);
  pushUndoCommand(command);
}

void ZFlyEmProofDoc::updateTileData()
{
  ZDvidReader reader;
  if (reader.open(m_dvidTarget)) {
    ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
    ZStack *stack = ZStackFactory::makeVirtualStack(
          ZIntCuboid(dvidInfo.getStartCoordinates(),
                     dvidInfo.getEndCoordinates()));
    loadStack(stack);

    ZDvidTileEnsemble *ensemble = new ZDvidTileEnsemble;
    ensemble->setDvidTarget(getDvidTarget());
//    ensemble->attachView(stackWidget->getView());
    ensemble->setSource(ZStackObjectSourceFactory::MakeDvidTileSource());
    addObject(ensemble, true);

  //  target.setBodyLabelName("labels");

    ZDvidLabelSlice *labelSlice = new ZDvidLabelSlice;
    labelSlice->setRole(ZStackObjectRole::ROLE_ACTIVE_VIEW);
    labelSlice->setDvidTarget(getDvidTarget());
    labelSlice->setSource(ZStackObjectSourceFactory::MakeDvidLabelSliceSource());
    labelSlice->setBodyMerger(&m_bodyMerger);
    addObject(labelSlice, true);
  }
}

ZDvidTileEnsemble* ZFlyEmProofDoc::getDvidTileEnsemble() const
{
  QList<ZDvidTileEnsemble*> teList = getDvidTileEnsembleList();
  if (!teList.empty()) {
    return teList[0];
  }

  return NULL;
}

ZDvidLabelSlice* ZFlyEmProofDoc::getDvidLabelSlice() const
{
  QList<ZDvidLabelSlice*> teList = getDvidLabelSliceList();
  if (!teList.empty()) {
    return teList[0];
  }

  return NULL;
}

void ZFlyEmProofDoc::updateBodyObject()
{
  QList<ZDvidLabelSlice*> sliceList = getDvidLabelSliceList();
  foreach (ZDvidLabelSlice *slice, sliceList) {
    slice->clearSelection();
    slice->updateLabelColor();
  }
}

void ZFlyEmProofDoc::clearData()
{
  ZStackDoc::clearData();
  m_bodyMerger.clear();
  m_dvidTarget.clear();
}


//////////////////////////////////////////
ZFlyEmProofDocCommand::MergeBody::MergeBody(
    ZStackDoc *doc, QUndoCommand *parent)
  : ZUndoCommand(parent), m_doc(doc)
{

}

ZFlyEmProofDoc* ZFlyEmProofDocCommand::MergeBody::getCompleteDocument()
{
  return dynamic_cast<ZFlyEmProofDoc*>(m_doc);
}

void ZFlyEmProofDocCommand::MergeBody::redo()
{
  getCompleteDocument()->getBodyMerger()->redo();
  getCompleteDocument()->updateBodyObject();

  m_doc->notifyObject3dScanModified();
}

void ZFlyEmProofDocCommand::MergeBody::undo()
{
  getCompleteDocument()->getBodyMerger()->undo();
  getCompleteDocument()->updateBodyObject();

  m_doc->notifyObject3dScanModified();
}

#include "zflyemproofdoc.h"

#include <QSet>
#include <QList>

#include "dvid/zdvidlabelslice.h"
#include "dvid/zdvidreader.h"
#include "zstackobjectsourcefactory.h"
#include "dvid/zdvidtileensemble.h"
#include "dvid/zdvidlabelslice.h"
#include "zstackfactory.h"
#include "dvid/zdvidsparsestack.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidsparsevolslice.h"
#include "zwidgetmessage.h"
#include "flyem/zflyemsupervisor.h"
//#include "zflyemproofmvc.h"

ZFlyEmProofDoc::ZFlyEmProofDoc(ZStack *stack, QObject *parent) :
  ZStackDoc(stack, parent)
{
  setTag(NeuTube::Document::FLYEM_PROOFREAD);
}

void ZFlyEmProofDoc::mergeSelected(ZFlyEmSupervisor *supervisor)
{
  QList<ZDvidLabelSlice*> sliceList = getDvidLabelSliceList();

  ZFlyEmBodyMerger::TLabelSet labelSet;
  for (QList<ZDvidLabelSlice*>::const_iterator iter = sliceList.begin();
       iter != sliceList.end(); ++iter) {
    const ZDvidLabelSlice *labelSlice = *iter;
    const std::set<uint64_t> &selected = labelSlice->getSelectedOriginal();
    for (std::set<uint64_t>::const_iterator iter = selected.begin();
         iter != selected.end(); ++iter) {
      if (supervisor != NULL) {
        if (supervisor->checkOut(*iter)) {
          labelSet.insert(*iter);
        } else {
          labelSet.clear();
          emit messageGenerated(
                ZWidgetMessage(
                  QString("Failed to merge. %1 has been locked by someone else").
                  arg(*iter), NeuTube::MSG_ERROR));
          break;
        }
      } else {
        labelSet.insert(*iter);
      }
    }
  }

  if (!labelSet.empty()) {
    m_bodyMerger.pushMap(labelSet);
    m_bodyMerger.undo();

    ZFlyEmProofDocCommand::MergeBody *command =
        new ZFlyEmProofDocCommand::MergeBody(this);
    pushUndoCommand(command);
  }
}

void ZFlyEmProofDoc::updateTileData()
{
  ZDvidReader reader;
  if (reader.open(m_dvidTarget)) {
    ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
    if (dvidInfo.isValid()) {
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

const ZDvidSparseStack *ZFlyEmProofDoc::getDvidSparseStack() const
{
  return dynamic_cast<ZDvidSparseStack*>(
        getObjectGroup().findFirstSameSource(
          ZStackObject::TYPE_DVID_SPARSE_STACK,
          ZStackObjectSourceFactory::MakeSplitObjectSource()));
}

ZDvidSparseStack* ZFlyEmProofDoc::getDvidSparseStack()
{
  return const_cast<ZDvidSparseStack*>(
        static_cast<const ZFlyEmProofDoc&>(*this).getDvidSparseStack());
}

void ZFlyEmProofDoc::updateBodyObject()
{
  QList<ZDvidLabelSlice*> sliceList = getDvidLabelSliceList();
  foreach (ZDvidLabelSlice *slice, sliceList) {
//    slice->clearSelection();
    slice->updateLabelColor();
  }

  QList<ZDvidSparsevolSlice*> sparsevolSliceList = getDvidSparsevolSliceList();
  foreach (ZDvidSparsevolSlice *slice, sparsevolSliceList) {
//    slice->setLabel(m_bodyMerger.getFinalLabel(slice->getLabel()));
//    uint64_t finalLabel = m_bodyMerger.getFinalLabel(slice->getLabel());
    slice->setColor(getDvidLabelSlice()->getColor(
                      slice->getLabel(), NeuTube::BODY_LABEL_ORIGINAL));
    //slice->updateSelection();
  }
}

void ZFlyEmProofDoc::clearData()
{
  ZStackDoc::clearData();
  m_bodyMerger.clear();
  m_dvidTarget.clear();
}

bool ZFlyEmProofDoc::isSplittable(uint64_t bodyId) const
{
  return !m_bodyMerger.isMapped(bodyId);
}


const ZSparseStack* ZFlyEmProofDoc::getSparseStack() const
{
  if (getDvidSparseStack() != NULL) {
    return getDvidSparseStack()->getSparseStack();
  }

  return NULL;
}


ZSparseStack* ZFlyEmProofDoc::getSparseStack()
{
  if (getDvidSparseStack() != NULL) {
    return getDvidSparseStack()->getSparseStack();
  }

  return NULL;
}


bool ZFlyEmProofDoc::hasVisibleSparseStack() const
{
  /*
  if (hasSparseStack()) {
    return getDvidSparseStack()->isVisible();
  }
  */

  return false;
}

void ZFlyEmProofDoc::saveMergeOperation()
{
  ZDvidWriter writer;
  if (writer.open(getDvidTarget())) {
    writer.writeMergeOperation(m_bodyMerger.getFinalMap());

    if (writer.getStatusCode() == 200) {
      if (m_bodyMerger.isEmpty()) {
        emit messageGenerated(ZWidgetMessage("Merge operation cleared."));
      } else {
        emit messageGenerated(ZWidgetMessage("Merge operation saved."));
      }
    } else {
      emit messageGenerated(
            ZWidgetMessage("Cannot save the merge operation",
                           NeuTube::MSG_ERROR));
    }
  }
}

void ZFlyEmProofDoc::downloadBodyMask()
{
  if (getDvidSparseStack() != NULL) {
    getDvidSparseStack()->downloadBodyMask();
    notifyObjectModified();
  }
}

QList<uint64_t> ZFlyEmProofDoc::getMergedSource(uint64_t bodyId) const
{
  return m_bodyMerger.getOriginalLabelList(bodyId);
}

QSet<uint64_t> ZFlyEmProofDoc::getMergedSource(const QSet<uint64_t> &bodySet)
const
{
  QSet<uint64_t> source;

  for (QSet<uint64_t>::const_iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    QList<uint64_t> merged = getMergedSource(*iter);
#ifdef _DEBUG_
    std::cout << "Merge list: " << merged.size() << std::endl;
#endif
    for (QList<uint64_t>::const_iterator bodyIter = merged.begin();
         bodyIter != merged.end(); ++bodyIter) {
      uint64_t label = *bodyIter;
      source.insert(label);
    }
  }

  return source;
}

void ZFlyEmProofDoc::notifyBodyMerged()
{
  emit bodyMerged();
}

void ZFlyEmProofDoc::notifyBodyUnmerged()
{
  emit bodyUnmerged();
}

void ZFlyEmProofDoc::clearBodyMerger()
{
  getBodyMerger()->clear();
  undoStack()->clear();
}

void ZFlyEmProofDoc::updateDvidLabelObject()
{
  beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  TStackObjectList &objList = getObjectList(ZStackObject::TYPE_DVID_LABEL_SLICE);
  for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
       ++iter) {
    ZDvidLabelSlice *obj = dynamic_cast<ZDvidLabelSlice*>(*iter);
    obj->update();
    processObjectModified(obj);
  }

  TStackObjectList &objList2 = getObjectList(ZStackObject::TYPE_DVID_SPARSEVOL_SLICE);
  for (TStackObjectList::iterator iter = objList2.begin(); iter != objList2.end();
       ++iter) {
    ZDvidSparsevolSlice *obj = dynamic_cast<ZDvidSparsevolSlice*>(*iter);
    obj->update();
    processObjectModified(obj);
  }
  endObjectModifiedMode();

  notifyObjectModified();
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

  getCompleteDocument()->notifyBodyMerged();
//  m_doc->notifyObject3dScanModified();
}

void ZFlyEmProofDocCommand::MergeBody::undo()
{
  getCompleteDocument()->getBodyMerger()->undo();
  getCompleteDocument()->updateBodyObject();

  getCompleteDocument()->notifyBodyUnmerged();
//  m_doc->notifyObject3dScanModified();
}

#include "zflyemproofdoccommand.h"


#include "flyem/zflyemproofdoc.h"

//////////////////////////////////////////
ZFlyEmProofDocCommand::MergeBody::MergeBody(
    ZStackDoc *doc, QUndoCommand *parent)
  : ZUndoCommand(parent), m_doc(doc)
{

}

ZFlyEmProofDoc* ZFlyEmProofDocCommand::MergeBody::getCompleteDocument()
{
  return qobject_cast<ZFlyEmProofDoc*>(m_doc);
}

void ZFlyEmProofDocCommand::MergeBody::redo()
{
  getCompleteDocument()->getBodyMerger()->redo();
  getCompleteDocument()->updateBodyObject();

  getCompleteDocument()->notifyBodyMerged();
  getCompleteDocument()->notifyBodyMergeEdited();
//  m_doc->notifyObject3dScanModified();
}

void ZFlyEmProofDocCommand::MergeBody::undo()
{
  ZFlyEmBodyMerger::TLabelMap mapped =
      getCompleteDocument()->getBodyMerger()->undo();

  std::set<uint64_t> bodySet;
  for (ZFlyEmBodyMerger::TLabelMap::const_iterator iter = mapped.begin();
       iter != mapped.end(); ++iter) {
    bodySet.insert(iter.key());
    bodySet.insert(iter.value());
  }

  for (std::set<uint64_t>::const_iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    uint64_t bodyId = *iter;
    if (!getCompleteDocument()->getBodyMerger()->isMerged(bodyId)) {
      getCompleteDocument()->notifyBodyIsolated(bodyId);
    }
  }

  getCompleteDocument()->updateBodyObject();

  getCompleteDocument()->notifyBodyUnmerged();
  getCompleteDocument()->notifyBodyMergeEdited();
//  m_doc->notifyObject3dScanModified();
}

////////////////////////////
ZFlyEmProofDocCommand::UnmergeBody::UnmergeBody(
    ZStackDoc *doc, QUndoCommand *parent)
  : ZUndoCommand(parent), m_doc(doc)
{

}

ZFlyEmProofDoc* ZFlyEmProofDocCommand::UnmergeBody::getCompleteDocument()
{
  return qobject_cast<ZFlyEmProofDoc*>(m_doc);
}

void ZFlyEmProofDocCommand::UnmergeBody::redo()
{
  ZFlyEmProofDoc *doc = getCompleteDocument();
  if (doc != NULL) {
    m_oldMapList = doc->getBodyMerger()->getMapList();
    std::set<uint64_t> bodySet =
        doc->getSelectedBodySet(NeuTube::BODY_LABEL_ORIGINAL);
    for (std::set<uint64_t>::const_iterator iter = bodySet.begin();
         iter != bodySet.end(); ++iter) {
      uint64_t bodyId = *iter;
      if (doc->getBodyMerger()->isMerged(bodyId)) {
        m_bodySet.insert(bodyId);
      }
    }

    for (std::set<uint64_t>::const_iterator iter = m_bodySet.begin();
         iter != m_bodySet.end(); ++iter) {
      uint64_t bodyId = *iter;
      doc->getBodyMerger()->unmerge(bodyId);
      doc->notifyBodyIsolated(bodyId);
    }

    if (!m_bodySet.empty()) {
      doc->updateBodyObject();
      doc->notifyBodyUnmerged();
      doc->notifyBodyMergeEdited();
    }
  }
}

void ZFlyEmProofDocCommand::UnmergeBody::undo()
{
  ZFlyEmProofDoc *doc = getCompleteDocument();
  if (doc != NULL) {

    for (std::set<uint64_t>::const_iterator iter = m_bodySet.begin();
         iter != m_bodySet.end(); ++iter) {
      uint64_t bodyId = *iter;
      doc->notifyBodyLock(bodyId, true);
    }

    if (!m_bodySet.empty()) {
      doc->getBodyMerger()->setMapList(m_oldMapList);
      doc->updateBodyObject();
      doc->notifyBodyMerged();
      doc->notifyBodyMergeEdited();
    }
  }
}



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
#include "zpuncta.h"
#include "dvid/zdvidurl.h"
#include "dvid/zdvidbufferreader.h"
//#include "zflyemproofmvc.h"
#include "flyem/zflyembookmark.h"
#include "zstring.h"

ZFlyEmProofDoc::ZFlyEmProofDoc(QObject *parent) :
  ZStackDoc(parent)
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
          std::string owner = supervisor->getOwner(*iter);
          if (owner.empty()) {
            owner = "unknown user";
          }
          emit messageGenerated(
                ZWidgetMessage(
                  QString("Failed to merge. %1 has been locked by %2").
                  arg(*iter).arg(owner.c_str()), NeuTube::MSG_ERROR));
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
    obj->forceUpdate();
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

void ZFlyEmProofDoc::downloadBookmark()
{
  if (getDvidTarget().isValid()) {
    ZDvidUrl url(getDvidTarget());
    ZDvidBufferReader reader;
    reader.read(url.getCustomBookmarkUrl(NeuTube::GetUserName()).c_str());
    ZJsonArray jsonObj;
    jsonObj.decodeString(reader.getBuffer());
    if (!jsonObj.isEmpty()) {
      beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
      for (size_t i = 0; i < jsonObj.size(); ++i) {
        ZJsonObject bookmarkObj =
            ZJsonObject(jsonObj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
        ZFlyEmBookmark *bookmark = new ZFlyEmBookmark;
        bookmark->loadJsonObject(bookmarkObj);
        addObject(bookmark, true);
      }
      endObjectModifiedMode();
      notifyObjectModified();
    }
  }
}

void ZFlyEmProofDoc::downloadSynapse()
{
  if (getDvidTarget().isValid()) {
    ZDvidUrl url(getDvidTarget());
    ZDvidBufferReader reader;
    reader.read(url.getSynapseAnnotationUrl().c_str());
    ZJsonObject jsonObj;
    jsonObj.decodeString(reader.getBuffer());
    if (!jsonObj.isEmpty()) {
      ZPuncta *puncta = new ZPuncta;
      puncta->setSource(ZStackObjectSourceFactory::MakeFlyEmSynapseSource());
      puncta->load(jsonObj, 5.0);
      puncta->pushCosmeticPen(true);
      addObject(puncta);
    }
  }
}

void ZFlyEmProofDoc::loadSynapse(const std::string &filePath)
{
  if (!filePath.empty()) {
    ZPuncta *puncta = new ZPuncta;
    puncta->setSource(ZStackObjectSourceFactory::MakeFlyEmSynapseSource());
    puncta->load(filePath, 5.0);
    puncta->pushCosmeticPen(true);
    addObject(puncta);
  }
}

ZFlyEmBookmark* ZFlyEmProofDoc::findFirstBookmark(const QString &key) const
{
  const TStackObjectList &objList =
      getObjectList(ZStackObject::TYPE_FLYEM_BOOKMARK);
  for (TStackObjectList::const_iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    const ZFlyEmBookmark *bookmark = dynamic_cast<const ZFlyEmBookmark*>(*iter);
    if (bookmark->getDvidKey() == key) {
      return const_cast<ZFlyEmBookmark*>(bookmark);
    }
  }

  return NULL;
}

void ZFlyEmProofDoc::importFlyEmBookmark(const std::string &filePath)
{
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  if (!filePath.empty()) {
//    removeObject(ZStackObject::TYPE_FLYEM_BOOKMARK, true);
    TStackObjectList objList = getObjectList(ZStackObject::TYPE_FLYEM_BOOKMARK);
#ifdef _DEBUG_
    std::cout << objList.size() << " bookmarks" << std::endl;
#endif
    for (TStackObjectList::iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      ZStackObject *obj = *iter;
      ZFlyEmBookmark *bookmark = dynamic_cast<ZFlyEmBookmark*>(obj);
      if (bookmark != NULL) {
        if (!bookmark->isCustom()) {
#ifdef _DEBUG_
          std::cout << "Removing bookmark: " << bookmark << std::endl;
#endif
          removeObject(*iter, true);
        }
      }
    }

    ZJsonObject obj;

    obj.load(filePath);

    ZJsonArray bookmarkArrayObj(obj["data"], false);
    for (size_t i = 0; i < bookmarkArrayObj.size(); ++i) {
      ZJsonObject bookmarkObj(bookmarkArrayObj.at(i), false);
      ZString text = ZJsonParser::stringValue(bookmarkObj["text"]);
      text.toLower();
      if (bookmarkObj["location"] != NULL) {
        ZJsonValue idJson = bookmarkObj.value("body ID");
        int64_t bodyId = 0;
        if (idJson.isInteger()) {
          bodyId = ZJsonParser::integerValue(idJson.getData());
        } else if (idJson.isString()) {
          bodyId = ZString::firstInteger(ZJsonParser::stringValue(idJson.getData()));
        }

        if (bodyId > 0) {
          std::vector<int> coordinates =
              ZJsonParser::integerArray(bookmarkObj["location"]);

          if (coordinates.size() == 3) {
            ZFlyEmBookmark *bookmark = new ZFlyEmBookmark;
            double x = coordinates[0];
            double y = coordinates[1];
            double z = coordinates[2];
            bookmark->setLocation(iround(x), iround(y), iround(z));
            bookmark->setBodyId(bodyId);
            bookmark->setRadius(5.0);
            bookmark->setColor(255, 0, 0);
            bookmark->setHittable(false);
            if (text.startsWith("split") || text.startsWith("small split")) {
              bookmark->setBookmarkType(ZFlyEmBookmark::TYPE_FALSE_MERGE);
            } else if (text.startsWith("merge")) {
              bookmark->setBookmarkType(ZFlyEmBookmark::TYPE_FALSE_SPLIT);
            } else {
              bookmark->setBookmarkType(ZFlyEmBookmark::TYPE_LOCATION);
            }
            addObject(bookmark);
          }
        }
      }
    }
  }
  endObjectModifiedMode();

  notifyObjectModified();
}

uint64_t ZFlyEmProofDoc::getBodyId(int x, int y, int z)
{
  uint64_t bodyId = 0;
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    bodyId = m_bodyMerger.getFinalLabel(reader.readBodyIdAt(x, y, z));
  }

  return bodyId;
}

uint64_t ZFlyEmProofDoc::getBodyId(const ZIntPoint &pt)
{
  return getBodyId(pt.getX(), pt.getY(), pt.getZ());
}

void ZFlyEmProofDoc::autoSave()
{
  saveCustomBookmark();
}

void ZFlyEmProofDoc::saveCustomBookmark()
{
  ZDvidWriter writer;
  if (writer.open(getDvidTarget())) {
    const TStackObjectList &objList =
        getObjectList(ZStackObject::TYPE_FLYEM_BOOKMARK);
    ZJsonArray jsonArray;
    for (TStackObjectList::const_iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      const ZFlyEmBookmark *bookmark = dynamic_cast<ZFlyEmBookmark*>(*iter);
      if (bookmark != NULL) {
        if (bookmark->isCustom()) {
          ZJsonObject obj = bookmark->toJsonObject();
          jsonArray.append(obj);
        }
      }
    }
    writer.writeCustomBookmark(jsonArray);
    if (writer.getStatusCode() != 200 && !jsonArray.isEmpty()) {
      emit messageGenerated(
            ZWidgetMessage("Failed to save bookmarks.", NeuTube::MSG_ERROR));
    }
  }
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

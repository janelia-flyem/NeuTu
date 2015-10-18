#include "zflyemproofdoc.h"

#include <QSet>
#include <QList>
#include <QTimer>
#include <QDir>
#include <QtConcurrentRun>
#include <QMessageBox>

#include "neutubeconfig.h"
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
#include "flyem/zsynapseannotationarray.h"
#include "zintcuboidobj.h"
#include "zslicedpuncta.h"
#include "zdialogfactory.h"

ZFlyEmProofDoc::ZFlyEmProofDoc(QObject *parent) :
  ZStackDoc(parent)
{
  init();
}

void ZFlyEmProofDoc::init()
{
  setTag(NeuTube::Document::FLYEM_PROOFREAD);

  initTimer();
  initAutoSave();

  connectSignalSlot();
}

void ZFlyEmProofDoc::initTimer()
{
  m_bookmarkTimer = new QTimer(this);
  m_bookmarkTimer->setInterval(60000);
  m_bookmarkTimer->start();
}

void ZFlyEmProofDoc::initAutoSave()
{
  m_isCustomBookmarkSaved = true;

  QDir autoSaveDir(NeutubeConfig::getInstance().getPath(
        NeutubeConfig::AUTO_SAVE).c_str());
  QString mergeFolder = "neutu_proofread_backup";

  if (!autoSaveDir.exists(mergeFolder)) {
    if (!autoSaveDir.mkdir(mergeFolder)) {
      emit messageGenerated(
            ZWidgetMessage("Failed to create autosave folder for merging. "
                           "Backup disabled for merge operations.",
                           NeuTube::MSG_ERROR));
    }
  }

  if (autoSaveDir.exists(mergeFolder)) {
    QDir mergeDir(autoSaveDir.absoluteFilePath(mergeFolder));
    m_mergeAutoSavePath = mergeDir.absoluteFilePath("neutu_merge_opr.json");
  }
}

void ZFlyEmProofDoc::connectSignalSlot()
{
  connect(m_bookmarkTimer, SIGNAL(timeout()),
          this, SLOT(saveCustomBookmarkSlot()));
}

void ZFlyEmProofDoc::setSelectedBody(
    std::set<uint64_t> &selected, NeuTube::EBodyLabelType labelType)
{
  if (getDvidLabelSlice() != NULL) {
    getDvidLabelSlice()->setSelection(selected, labelType);

    emit bodySelectionChanged();
  }
}

void ZFlyEmProofDoc::addSelectedBody(
    std::set<uint64_t> &selected, NeuTube::EBodyLabelType labelType)
{
  if (getDvidLabelSlice() != NULL) {
    if (!selected.empty()) {
      getDvidLabelSlice()->addSelection(
            selected.begin(), selected.end(), labelType);
      emit bodySelectionChanged();
    }
  }
}

void ZFlyEmProofDoc::setSelectedBody(
    uint64_t bodyId, NeuTube::EBodyLabelType labelType)
{
  std::set<uint64_t> selected;
  selected.insert(bodyId);
  setSelectedBody(selected, labelType);
}

std::set<uint64_t> ZFlyEmProofDoc::getSelectedBodySet(
    NeuTube::EBodyLabelType labelType) const
{
  QList<ZDvidLabelSlice*> sliceList = getDvidLabelSliceList();

  std::set<uint64_t> finalSet;
  for (QList<ZDvidLabelSlice*>::const_iterator iter = sliceList.begin();
       iter != sliceList.end(); ++iter) {
    const ZDvidLabelSlice *labelSlice = *iter;
    const std::set<uint64_t> &selected = labelSlice->getSelectedOriginal();
    finalSet.insert(selected.begin(), selected.end());
  }

  switch (labelType) {
  case NeuTube::BODY_LABEL_ORIGINAL:
    break;
  case NeuTube::BODY_LABEL_MAPPED:
    finalSet = getBodyMerger()->getFinalLabel(finalSet);
    break;
  }

  return finalSet;
}

void ZFlyEmProofDoc::removeSelectedAnnotation(uint64_t bodyId)
{
  m_annotationMap.remove(bodyId);
}

void ZFlyEmProofDoc::recordAnnotation(
    uint64_t bodyId, const ZFlyEmBodyAnnotation &anno)
{
  if (m_annotationMap.count(bodyId) == 0) {
    m_annotationMap[bodyId] = anno;
  } else {
    m_annotationMap[bodyId].mergeAnnotation(anno);
  }
}


void ZFlyEmProofDoc::mergeSelected(ZFlyEmSupervisor *supervisor)
{
  bool okToContinue = true;

  QMap<uint64_t, QString> nameMap;
  for (QMap<uint64_t, ZFlyEmBodyAnnotation>::const_iterator
       iter = m_annotationMap.begin(); iter != m_annotationMap.end(); ++iter) {
    const ZFlyEmBodyAnnotation& anno = iter.value();
    if (!anno.getName().empty()) {
      nameMap[iter.key()] = anno.getName().c_str();
    }
  }
  if (nameMap.size() > 1) {
    QString detail = "<p>Details: </p>";
    detail += "<ul>";
    for (QMap<uint64_t, QString>::const_iterator iter = nameMap.begin();
         iter != nameMap.end(); ++iter) {
      detail += QString("<li>%1: %2</li>").arg(iter.key()).arg(iter.value());
    }
    detail += "</ul>";
    okToContinue = ZDialogFactory::Ask(
          "Conflict to Resolve",
          "You are about to merge multiple names. Do you want to continue?" +
          detail,
          NULL);
  }

  if (okToContinue) {
    QList<ZDvidLabelSlice*> sliceList = getDvidLabelSliceList();

    ZFlyEmBodyMerger::TLabelSet labelSet;
    for (QList<ZDvidLabelSlice*>::const_iterator iter = sliceList.begin();
         iter != sliceList.end(); ++iter) {
      const ZDvidLabelSlice *labelSlice = *iter;
      const std::set<uint64_t> &selected = labelSlice->getSelectedOriginal();

      if (selected.size() > 1){
        for (std::set<uint64_t>::const_iterator iter = selected.begin();
             iter != selected.end(); ++iter) {
          if (supervisor != NULL) {
            if (supervisor->checkOut(*iter)) {
              labelSet.insert(*iter);
            } else {
              labelSet.clear();
              std::string owner = supervisor->getOwner(*iter);
              if (owner.empty()) {
                //            owner = "unknown user";
                emit messageGenerated(
                      ZWidgetMessage(
                        QString("Failed to merge. Is the librarian sever (%2) ready?").
                        arg(*iter).arg(getDvidTarget().getSupervisor().c_str()),
                        NeuTube::MSG_ERROR));
              } else {
                emit messageGenerated(
                      ZWidgetMessage(
                        QString("Failed to merge. %1 has been locked by %2").
                        arg(*iter).arg(owner.c_str()), NeuTube::MSG_ERROR));
              }
              break;
            }
          } else {
            labelSet.insert(*iter);
          }
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
}

void ZFlyEmProofDoc::annotateBody(
    uint64_t bodyId, const ZFlyEmBodyAnnotation &annotation)
{
  ZDvidWriter writer;
  if (writer.open(getDvidTarget())) {
    writer.writeAnnotation(bodyId, annotation.toJsonObject());

    if (getDvidLabelSlice()->hasCustomColorMap()) {
      m_bodyColorMap->updateNameMap(annotation);
      getDvidLabelSlice()->assignColorMap();
      processObjectModified(getDvidLabelSlice());
      notifyObjectModified();
    }
  }
  if (writer.getStatusCode() == 200) {
    emit messageGenerated(
          ZWidgetMessage(QString("Body %1 is annotated.").arg(bodyId)));
  } else {
    qDebug() << writer.getStandardOutput();
    emit messageGenerated(
          ZWidgetMessage("Cannot save annotation.", NeuTube::MSG_ERROR));
  }
}

void ZFlyEmProofDoc::setDvidTarget(const ZDvidTarget &target)
{
  if (m_dvidReader.open(target)) {
    m_dvidTarget = target;
    m_bodyColorMap.reset();
  } else {
    emit messageGenerated(
          ZWidgetMessage("Failed to open the node.", NeuTube::MSG_ERROR));
  }
}

void ZFlyEmProofDoc::updateTileData()
{
  if (m_dvidReader.isReady()) {
    ZDvidInfo dvidInfo = m_dvidReader.readGrayScaleInfo();
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

const ZDvidSparseStack *ZFlyEmProofDoc::getBodyForSplit() const
{
  return dynamic_cast<ZDvidSparseStack*>(
        getObjectGroup().findFirstSameSource(
          ZStackObject::TYPE_DVID_SPARSE_STACK,
          ZStackObjectSourceFactory::MakeSplitObjectSource()));
}

ZDvidSparseStack* ZFlyEmProofDoc::getBodyForSplit()
{
  return const_cast<ZDvidSparseStack*>(
        static_cast<const ZFlyEmProofDoc&>(*this).getBodyForSplit());
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
  if (m_dvidReader.isReady()) {
    ZFlyEmBodyAnnotation annotation = m_dvidReader.readBodyAnnotation(bodyId);
    if (annotation.isFinalized()) {
      return false;
    }
  }

  return !m_bodyMerger.isMerged(bodyId);
}


const ZSparseStack* ZFlyEmProofDoc::getSparseStack() const
{
  if (getBodyForSplit() != NULL) {
    return getBodyForSplit()->getSparseStack();
  }

  return NULL;
}


ZSparseStack* ZFlyEmProofDoc::getSparseStack()
{
  if (getBodyForSplit() != NULL) {
    return getBodyForSplit()->getSparseStack();
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

void ZFlyEmProofDoc::backupMergeOperation()
{
  if (!m_mergeAutoSavePath.isEmpty()) {
    if (!m_bodyMerger.isEmpty()) {
      m_bodyMerger.toJsonArray().dump(m_mergeAutoSavePath.toStdString());
    }
  }
}

void ZFlyEmProofDoc::downloadBodyMask()
{
  if (getBodyForSplit() != NULL) {
    getBodyForSplit()->downloadBodyMask();
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
    reader.read(url.getCustomBookmarkUrl(NeuTube::GetCurrentUserName()).c_str());
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

void ZFlyEmProofDoc::downloadSynapseFunc()
{
  if (getDvidTarget().isValid()) {
    emit messageGenerated(ZWidgetMessage("Downloading synapses ..."));

    ZDvidUrl url(getDvidTarget());
    ZDvidBufferReader reader;
    reader.read(url.getSynapseAnnotationUrl().c_str());

    ZJsonObject jsonObj;
    jsonObj.decodeString(reader.getBuffer());
    if (!jsonObj.isEmpty()) {
      FlyEm::ZSynapseAnnotationArray synapseArray;
      synapseArray.loadJson(jsonObj);
      const double radius = 5.0;
      std::vector<ZStackBall*> puncta = synapseArray.toTBarBall(radius);

      ZSlicedPuncta *tbar = new ZSlicedPuncta;
      tbar->addPunctum(puncta.begin(), puncta.end());
      decorateTBar(tbar);

      emit addingObject(tbar, true);
      emit messageGenerated(ZWidgetMessage("TBars ready."));
//      addObject(tbar);

      ZSlicedPuncta *psd = new ZSlicedPuncta;
      puncta = synapseArray.toPsdBall(radius / 2.0);
      psd->addPunctum(puncta.begin(), puncta.end());
      decoratePsd(psd);

      emit addingObject(psd, true);

      emit messageGenerated(ZWidgetMessage("All synapses ready."));

//      addObject(psd);
    } else {
      emit messageGenerated(ZWidgetMessage("No synapse found."));
    }
  }
}

void ZFlyEmProofDoc::downloadSynapse()
{
  const QString threadId = "downloadSynapse";
  if (!m_futureMap.isAlive(threadId)) {
    m_futureMap.removeDeadThread();
    QFuture<void> future =
        QtConcurrent::run(this, &ZFlyEmProofDoc::downloadSynapseFunc);
    m_futureMap[threadId] = future;
  }
}

void ZFlyEmProofDoc::processBookmarkAnnotationEvent(ZFlyEmBookmark */*bookmark*/)
{
  m_isCustomBookmarkSaved = false;
}

void ZFlyEmProofDoc::decorateTBar(ZPuncta *puncta)
{
  puncta->setSource(ZStackObjectSourceFactory::MakeFlyEmTBarSource());
  puncta->pushCosmeticPen(true);
  puncta->pushColor(QColor(0, 255, 0));
  puncta->pushVisualEffect(NeuTube::Display::Sphere::VE_CROSS_CENTER |
                           NeuTube::Display::Sphere::VE_OUT_FOCUS_DIM);
}

void ZFlyEmProofDoc::decoratePsd(ZPuncta *puncta)
{
  puncta->setSource(ZStackObjectSourceFactory::MakeFlyEmPsdSource());
  puncta->pushCosmeticPen(true);
  puncta->pushColor(QColor(0, 0, 255));
  puncta->pushVisualEffect(NeuTube::Display::Sphere::VE_CROSS_CENTER |
                           NeuTube::Display::Sphere::VE_OUT_FOCUS_DIM);
}

void ZFlyEmProofDoc::decorateTBar(ZSlicedPuncta *puncta)
{
  puncta->setSource(ZStackObjectSourceFactory::MakeFlyEmTBarSource());
  puncta->pushCosmeticPen(true);
  puncta->pushColor(QColor(0, 255, 0));
  puncta->pushVisualEffect(NeuTube::Display::Sphere::VE_CROSS_CENTER |
                           NeuTube::Display::Sphere::VE_OUT_FOCUS_DIM);
}

void ZFlyEmProofDoc::decoratePsd(ZSlicedPuncta *puncta)
{
  puncta->setSource(ZStackObjectSourceFactory::MakeFlyEmPsdSource());
  puncta->pushCosmeticPen(true);
  puncta->pushColor(QColor(0, 0, 255));
  puncta->pushVisualEffect(NeuTube::Display::Sphere::VE_CROSS_CENTER |
                           NeuTube::Display::Sphere::VE_OUT_FOCUS_DIM);
}

std::vector<ZPunctum*> ZFlyEmProofDoc::getTbar(ZObject3dScan &body)
{
  std::vector<ZPunctum*> puncta;
  ZSlicedPuncta  *tbar = dynamic_cast<ZSlicedPuncta*>(
        getObjectGroup().findFirstSameSource(
          ZStackObject::TYPE_SLICED_PUNCTA,
          ZStackObjectSourceFactory::MakeFlyEmTBarSource()));

  if (tbar != NULL) {
    ZDvidReader reader;
    if (reader.open(getDvidTarget())) {
//      ZIntCuboid box = reader.readBodyBoundBox(bodyId);
      ZIntCuboid box = body.getBoundBox();
      int minZ = box.getFirstCorner().getZ();
      int maxZ = box.getLastCorner().getZ();

//      ZObject3dScan coarseBody = reader.readCoarseBody(bodyId);
//      ZDvidInfo dvidInfo = reader.readGrayScaleInfo();

      for (int z = minZ; z <= maxZ; ++z) {
        QList<ZStackBall*> ballList = tbar->getPunctaOnSlice(z);
        for (QList<ZStackBall*>::const_iterator iter = ballList.begin();
             iter != ballList.end(); ++iter) {
          ZStackBall *ball = *iter;
          ZIntPoint pt = ball->getCenter().toIntPoint();
          if (box.contains(pt)) {
//            ZIntPoint blockIndex = dvidInfo.getBlockIndex(pt);

//            if (coarseBody.contains(blockIndex)) {
              if (body.contains(pt)) {
                puncta.push_back(
                      new ZPunctum(ball->x(), ball->y(), ball->z(), ball->radius()));
              }
//            }
          }
        }
      }
    }
  }

  return puncta;
}

std::vector<ZPunctum*> ZFlyEmProofDoc::getTbar(uint64_t bodyId)
{
  std::vector<ZPunctum*> puncta;
  ZSlicedPuncta  *tbar = dynamic_cast<ZSlicedPuncta*>(
        getObjectGroup().findFirstSameSource(
          ZStackObject::TYPE_SLICED_PUNCTA,
          ZStackObjectSourceFactory::MakeFlyEmTBarSource()));

  if (tbar != NULL) {
    ZDvidReader reader;
    reader.setVerbose(false);
    if (reader.open(getDvidTarget())) {
      ZIntCuboid box = reader.readBodyBoundBox(bodyId);
      int minZ = box.getFirstCorner().getZ();
      int maxZ = box.getLastCorner().getZ();

      ZObject3dScan coarseBody = reader.readCoarseBody(bodyId);
      ZDvidInfo dvidInfo = reader.readGrayScaleInfo();

      for (int z = minZ; z <= maxZ; ++z) {
        QList<ZStackBall*> ballList = tbar->getPunctaOnSlice(z);
        for (QList<ZStackBall*>::const_iterator iter = ballList.begin();
             iter != ballList.end(); ++iter) {
          ZStackBall *ball = *iter;
          ZIntPoint pt = ball->getCenter().toIntPoint();
          if (box.contains(pt)) {
            ZIntPoint blockIndex = dvidInfo.getBlockIndex(pt);

            if (coarseBody.contains(blockIndex)) {
              if (reader.readBodyIdAt(pt) == bodyId) {
                puncta.push_back(
                      new ZPunctum(ball->x(), ball->y(), ball->z(), ball->radius()));
              }
            }
          }
        }
      }
    }
  }

  return puncta;
}

std::pair<std::vector<ZPunctum*>, std::vector<ZPunctum*> >
ZFlyEmProofDoc::getSynapse(uint64_t bodyId)
{
  std::pair<std::vector<ZPunctum*>, std::vector<ZPunctum*> > synapse;
  ZSlicedPuncta  *tbar = dynamic_cast<ZSlicedPuncta*>(
        getObjectGroup().findFirstSameSource(
          ZStackObject::TYPE_SLICED_PUNCTA,
          ZStackObjectSourceFactory::MakeFlyEmTBarSource()));
  ZSlicedPuncta  *psd = dynamic_cast<ZSlicedPuncta*>(
        getObjectGroup().findFirstSameSource(
          ZStackObject::TYPE_SLICED_PUNCTA,
          ZStackObjectSourceFactory::MakeFlyEmPsdSource()));

  ZDvidReader reader;
  reader.setVerbose(false);
  if (reader.open(getDvidTarget())) {
    ZIntCuboid box = reader.readBodyBoundBox(bodyId);
    int minZ = box.getFirstCorner().getZ();
    int maxZ = box.getLastCorner().getZ();

    ZObject3dScan coarseBody = reader.readCoarseBody(bodyId);
    ZDvidInfo dvidInfo = reader.readGrayScaleInfo();

    for (int z = minZ; z <= maxZ; ++z) {
      if (tbar != NULL) {
        std::vector<ZPunctum*> &puncta = synapse.first;
        QList<ZStackBall*> ballList = tbar->getPunctaOnSlice(z);
        for (QList<ZStackBall*>::const_iterator iter = ballList.begin();
             iter != ballList.end(); ++iter) {
          ZStackBall *ball = *iter;
          ZIntPoint pt = ball->getCenter().toIntPoint();
          if (box.contains(pt)) {
            ZIntPoint blockIndex = dvidInfo.getBlockIndex(pt);

            if (coarseBody.contains(blockIndex)) {
              if (reader.readBodyIdAt(pt) == bodyId) {
                puncta.push_back(
                      new ZPunctum(ball->x(), ball->y(), ball->z(), ball->radius()));
              }
            }
          }
        }
      }
      if (psd != NULL) {
        std::vector<ZPunctum*> &puncta = synapse.second;
        QList<ZStackBall*> ballList = psd->getPunctaOnSlice(z);
        for (QList<ZStackBall*>::const_iterator iter = ballList.begin();
             iter != ballList.end(); ++iter) {
          ZStackBall *ball = *iter;
          ZIntPoint pt = ball->getCenter().toIntPoint();
          if (box.contains(pt)) {
            ZIntPoint blockIndex = dvidInfo.getBlockIndex(pt);

            if (coarseBody.contains(blockIndex)) {
              if (reader.readBodyIdAt(pt) == bodyId) {
                puncta.push_back(
                      new ZPunctum(ball->x(), ball->y(), ball->z(), ball->radius()));
              }
            }
          }
        }
      }
    }
  }

  return synapse;
}

void ZFlyEmProofDoc::loadSynapse(const std::string &filePath)
{
  if (!filePath.empty()) {
    ZPuncta *puncta = new ZPuncta;
    puncta->load(filePath, 5.0);
    decorateTBar(puncta);
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
    std::vector<ZStackObject*> removed;

    for (TStackObjectList::iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      ZStackObject *obj = *iter;
      ZFlyEmBookmark *bookmark = dynamic_cast<ZFlyEmBookmark*>(obj);
      if (bookmark != NULL) {
        if (!bookmark->isCustom()) {
#ifdef _DEBUG_2
          std::cout << "Removing bookmark: " << bookmark << std::endl;
#endif
          removeObject(*iter, false);
          removed.push_back(*iter);
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
    for (std::vector<ZStackObject*>::iterator iter = removed.begin();
         iter != removed.end(); ++iter) {
      delete *iter;
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
  backupMergeOperation();
}

void ZFlyEmProofDoc::saveCustomBookmarkSlot()
{
  if (!m_isCustomBookmarkSaved) {
    std::cout << "Saving user bookmarks ..." << std::endl;
    saveCustomBookmark();
  }
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
    if (writer.getStatusCode() != 200) {
      emit messageGenerated(
            ZWidgetMessage("Failed to save bookmarks.", NeuTube::MSG_ERROR));
    } else {
      m_isCustomBookmarkSaved = true;
    }
  }
}

void ZFlyEmProofDoc::customNotifyObjectModified(ZStackObject::EType type)
{
  switch (type) {
  case ZStackObject::TYPE_FLYEM_BOOKMARK:
    m_isCustomBookmarkSaved = false;
    emit userBookmarkModified();
    break;
  default:
    break;
  }
}

void ZFlyEmProofDoc::enhanceTileContrast(bool highContrast)
{
  ZDvidTileEnsemble *tile = getDvidTileEnsemble();
  if (tile != NULL) {
    tile->enhanceContrast(highContrast);
    if (!tile->isEmpty()) {
      bufferObjectModified(tile->getTarget());
      notifyObjectModified();
    }
  }
}

void ZFlyEmProofDoc::notifyBodyIsolated(uint64_t bodyId)
{
  emit bodyIsolated(bodyId);
}

ZIntCuboidObj* ZFlyEmProofDoc::getSplitRoi() const
{
  return dynamic_cast<ZIntCuboidObj*>(
      getObjectGroup().findFirstSameSource(
        ZStackObject::TYPE_INT_CUBOID,
        ZStackObjectSourceFactory::MakeFlyEmSplitRoiSource()));
}

void ZFlyEmProofDoc::updateSplitRoi(ZRect2d *rect)
{
//  ZRect2d rect = getRect2dRoi();

  ZUndoCommand *command = new ZUndoCommand("Update ROI");

  beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  /*
  ZIntCuboidObj* roi = ZFlyEmProofDoc::getSplitRoi();
  if (roi == NULL) {
  */
  ZIntCuboidObj* roi = new ZIntCuboidObj;
  roi->setColor(QColor(255, 255, 255));
  roi->setSource(ZStackObjectSourceFactory::MakeFlyEmSplitRoiSource());
  roi->clear();

  executeRemoveObjectCommand(getSplitRoi());

  roi->setRole(ZStackObjectRole::ROLE_ROI);
  new ZStackDocCommand::ObjectEdit::AddObject(this, roi, false, command);
//    addObject(roi);
//  }

  if (rect != NULL) {
    if (rect->isValid()) {
      int sz = iround(sqrt(rect->getWidth() * rect->getWidth() +
                           rect->getHeight() * rect->getHeight()) / 2.0);
      roi->setFirstCorner(rect->getFirstX(), rect->getFirstY(), rect->getZ() - sz);
      roi->setLastCorner(rect->getLastX(), rect->getLastY(), rect->getZ() + sz);
    }
  }
  m_splitSource.reset();

  removeObject(rect, true);

  pushUndoCommand(command);

//  new ZStackDocCommand::ObjectEdit::RemoveObje
//  removeRect2dRoi();

//  processObjectModified(roi);

  endObjectModifiedMode();
  notifyObjectModified();
}

ZDvidSparseStack* ZFlyEmProofDoc::getDvidSparseStack() const
{
  ZDvidSparseStack *stack = NULL;

  ZDvidSparseStack *originalStack = ZStackDoc::getDvidSparseStack();
  if (originalStack != NULL) {
    ZIntCuboidObj *roi = getSplitRoi();
    if (roi != NULL) {
      if (roi->isValid()) {
        if (m_splitSource.get() == NULL) {
          m_splitSource = ZSharedPointer<ZDvidSparseStack>(
                originalStack->getCrop(roi->getCuboid()));
        }

        stack = m_splitSource.get();
      }
    }
  }

  if (stack == NULL) {
    stack = originalStack;
  }

  return stack;
}

void ZFlyEmProofDoc::deprecateSplitSource()
{
  m_splitSource.reset();
}

void ZFlyEmProofDoc::prepareBodyMap(const ZJsonValue &bodyInfoObj)
{
  if (m_bodyColorMap.get() == NULL) {
    m_bodyColorMap =
        ZSharedPointer<ZFlyEmBodyColorScheme>(new ZFlyEmBodyColorScheme);
    m_bodyColorMap->setDvidTarget(getDvidTarget());
  }
  m_bodyColorMap->prepareNameMap(bodyInfoObj);

  emit bodyMapReady();
}

void ZFlyEmProofDoc::useBodyNameMap(bool on)
{
  if (getDvidLabelSlice() != NULL) {
    if (on) {
      if (m_bodyColorMap.get() == NULL) {
        m_bodyColorMap =
            ZSharedPointer<ZFlyEmBodyColorScheme>(new ZFlyEmBodyColorScheme);
        m_bodyColorMap->setDvidTarget(getDvidTarget());
        m_bodyColorMap->prepareNameMap();
      }
      getDvidLabelSlice()->setCustomColorMap(m_bodyColorMap);
    } else {
      getDvidLabelSlice()->removeCustomColorMap();
    }

    processObjectModified(getDvidLabelSlice());
    notifyObjectModified();
  }
}

void ZFlyEmProofDoc::selectBody(uint64_t bodyId)
{
  ZDvidLabelSlice *slice = getDvidLabelSlice();
  if (slice != NULL) {
    slice->addSelection(bodyId, NeuTube::BODY_LABEL_MAPPED);
  }
}

void ZFlyEmProofDoc::selectBodyInRoi(int z, bool appending)
{
  ZRect2d rect = getRect2dRoi();

  if (rect.isValid()) {
    ZDvidReader reader;
    if (reader.open(getDvidTarget())) {
      std::set<uint64_t> bodySet = reader.readBodyId(
            rect.getFirstX(), rect.getFirstY(), z,
            rect.getWidth(), rect.getHeight(), 1);
      if (appending) {
        addSelectedBody(bodySet, NeuTube::BODY_LABEL_ORIGINAL);
      } else {
        setSelectedBody(bodySet, NeuTube::BODY_LABEL_ORIGINAL);
      }
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
  return qobject_cast<ZFlyEmProofDoc*>(m_doc);
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
//  m_doc->notifyObject3dScanModified();
}

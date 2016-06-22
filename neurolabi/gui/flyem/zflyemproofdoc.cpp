#include "zflyemproofdoc.h"

#include <QSet>
#include <QList>
#include <QTimer>
#include <QDir>
#include <QtConcurrentRun>
#include <QMessageBox>
#include <QElapsedTimer>

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
#include "zflyemnamebodycolorscheme.h"
#include "zflyemsequencercolorscheme.h"
#include "dvid/zdvidsynapseensenmble.h"
#include "dvid/zdvidsynpasecommand.h"
#include "dvid/zflyembookmarkcommand.h"
#include "dvid/zdvidannotation.h"
#include "flyem/zflyemtodolist.h"

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
//  m_isCustomBookmarkSaved = true;

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
    connect(this, SIGNAL(bodyMerged()),
            this, SLOT(saveMergeOperation()));
    connect(this, SIGNAL(bodyUnmerged()),
            this, SLOT(saveMergeOperation()));

  /*
  connect(m_bookmarkTimer, SIGNAL(timeout()),
          this, SLOT(saveCustomBookmarkSlot()));
          */
}

void ZFlyEmProofDoc::setSelectedBody(
    std::set<uint64_t> &selected, NeuTube::EBodyLabelType labelType)
{
  QList<ZDvidLabelSlice*> sliceList = getDvidLabelSliceList();
  if (!sliceList.isEmpty()) {
    for (QList<ZDvidLabelSlice*>::iterator iter = sliceList.begin();
         iter != sliceList.end(); ++iter) {
      ZDvidLabelSlice *slice = *iter;
      slice->setSelection(selected, labelType);
    }

    emit bodySelectionChanged();
  }
}

void ZFlyEmProofDoc::addSelectedBody(
    std::set<uint64_t> &selected, NeuTube::EBodyLabelType labelType)
{
  QList<ZDvidLabelSlice*> sliceList = getDvidLabelSliceList();
  if (!sliceList.isEmpty()) {
    if (!selected.empty()) {
      for (QList<ZDvidLabelSlice*>::iterator iter = sliceList.begin();
           iter != sliceList.end(); ++iter) {
        ZDvidLabelSlice *slice = *iter;
        slice->addSelection(selected.begin(), selected.end(), labelType);
      }
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
  m_annotationMap[bodyId] = anno;
}

void ZFlyEmProofDoc::cleanBodyAnnotationMap()
{
  std::set<uint64_t> selected = getSelectedBodySet(NeuTube::BODY_LABEL_ORIGINAL);
  std::vector<uint64_t> keysToRemove;
  for (QMap<uint64_t, ZFlyEmBodyAnnotation>::const_iterator
       iter = m_annotationMap.begin(); iter != m_annotationMap.end(); ++iter) {
    uint64_t bodyId = iter.key();
    if (selected.count(bodyId) == 0) {
      LWARN() << "In consistent body selection: " << bodyId;
      keysToRemove.push_back(bodyId);
    }
  }

  for (std::vector<uint64_t>::const_iterator iter = keysToRemove.begin();
       iter != keysToRemove.end(); ++iter) {
    m_annotationMap.remove(*iter);
  }
}

void ZFlyEmProofDoc::verifyBodyAnnotationMap()
{
  std::set<uint64_t> selected = getSelectedBodySet(NeuTube::BODY_LABEL_ORIGINAL);
  for (QMap<uint64_t, ZFlyEmBodyAnnotation>::const_iterator
       iter = m_annotationMap.begin(); iter != m_annotationMap.end(); ++iter) {
    uint64_t bodyId = iter.key();
    if (selected.count(bodyId) == 0) {
      emit messageGenerated(
            ZWidgetMessage(
              QString("Inconsistent body selection: %1").arg(bodyId),
              NeuTube::MSG_WARNING));
    }
  }
}

void ZFlyEmProofDoc::clearBodyMergeStage()
{
  clearBodyMerger();
  saveMergeOperation();
  notifyBodyUnmerged();
}


void ZFlyEmProofDoc::mergeSelected(ZFlyEmSupervisor *supervisor)
{
  bool okToContinue = true;

  cleanBodyAnnotationMap();

  QMap<uint64_t, QVector<QString> > nameMap;
  for (QMap<uint64_t, ZFlyEmBodyAnnotation>::const_iterator
       iter = m_annotationMap.begin(); iter != m_annotationMap.end(); ++iter) {
    const ZFlyEmBodyAnnotation& anno = iter.value();
    if (!anno.getName().empty()) {
      uint64_t mappedBodyId = getBodyMerger()->getFinalLabel(iter.key());

      if (!nameMap.contains(mappedBodyId)) {
        nameMap[mappedBodyId] = QVector<QString>();
      }
      nameMap[mappedBodyId].append(anno.getName().c_str());
//      nameMap[iter.key()] = anno.getName().c_str();
    }
  }
  if (nameMap.size() > 1) {
    QString detail = "<p>Details: </p>";
    detail += "<ul>";
    for (QMap<uint64_t, QVector<QString> >::const_iterator iter = nameMap.begin();
         iter != nameMap.end(); ++iter) {
      const QVector<QString> &nameArray = iter.value();
      detail += QString("<li>%1:").arg(iter.key());
      foreach (const QString &name, nameArray) {
        detail += " \"" + name + "\"";
      }

//      detail += QString("<li>%1: %2</li>").arg(iter.key()).arg(iter.value());

      detail += "</li>";
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

    QList<ZDvidLabelSlice*> sliceList = getDvidLabelSliceList();

    for (QList<ZDvidLabelSlice*>::iterator iter = sliceList.begin();
         iter != sliceList.end(); ++iter) {
      ZDvidLabelSlice *slice = *iter;
      if (slice->hasCustomColorMap()) {
        ZFlyEmNameBodyColorScheme *colorMap =
            dynamic_cast<ZFlyEmNameBodyColorScheme*>(m_activeBodyColorMap.get());
        if (colorMap != NULL) {
          colorMap->updateNameMap(annotation);
          slice->assignColorMap();
          processObjectModified(slice);
        }
      }
    }

    notifyObjectModified();
  }
  if (writer.getStatusCode() == 200) {
    if (getSelectedBodySet(NeuTube::BODY_LABEL_ORIGINAL).count(bodyId) > 0) {
      m_annotationMap[bodyId] = annotation;
    }
    emit messageGenerated(
          ZWidgetMessage(QString("Body %1 is annotated.").arg(bodyId)));
  } else {
    qDebug() << writer.getStandardOutput();
    emit messageGenerated(
          ZWidgetMessage("Cannot save annotation.", NeuTube::MSG_ERROR));
  }
}

void ZFlyEmProofDoc::initData(
    const std::string &type, const std::string &dataName)
{
  if (!m_dvidReader.hasData(dataName)) {
    m_dvidWriter.createData(type, dataName);
    if (!m_dvidWriter.isStatusOk()) {
      emit messageGenerated(
            ZWidgetMessage(QString("Failed to create data: ") + dataName.c_str(),
                           NeuTube::MSG_ERROR));
    }
  }
}

void ZFlyEmProofDoc::initData(const ZDvidTarget &target)
{
  if (m_dvidReader.isReady()) {
    initData("annotation", target.getBookmarkName());
    initData("annotation", target.getTodoListName());
    initData("keyvalue", target.getSkeletonName());
    initData("keyvalue", target.getThumbnailName());
    initData("keyvalue", target.getBookmarkKeyName());
  }
}

void ZFlyEmProofDoc::setDvidTarget(const ZDvidTarget &target)
{
  if (m_dvidReader.open(target)) {
    m_dvidWriter.open(target);
    m_dvidTarget = target;
    m_activeBodyColorMap.reset();
    initData(target);
  } else {
    m_dvidTarget.clear();
    emit messageGenerated(
          ZWidgetMessage("Failed to open the node.", NeuTube::MSG_ERROR));
  }

  prepareDvidData();

  updateDvidTargetForObject();
}

void ZFlyEmProofDoc::prepareDvidData()
{
  if (m_dvidReader.isReady()) {
    ZDvidInfo dvidInfo = m_dvidReader.readGrayScaleInfo();
    ZIntCuboid boundBox;
    if (dvidInfo.isValid()) {
      boundBox = ZIntCuboid(dvidInfo.getStartCoordinates(),
                       dvidInfo.getEndCoordinates());
    } else {
      boundBox = ZIntCuboid(ZIntPoint(0, 0, 0), ZIntPoint(13500, 11000, 10000));
    }

    ZStack *stack = ZStackFactory::makeVirtualStack(boundBox);
    loadStack(stack);
  }


  ZDvidTileEnsemble *ensemble = new ZDvidTileEnsemble;
  ensemble->addRole(ZStackObjectRole::ROLE_ACTIVE_VIEW);
  ensemble->setSource(ZStackObjectSourceFactory::MakeDvidTileSource());
  addObject(ensemble, true);

  ensemble->setDvidTarget(getDvidTarget());
  addDvidLabelSlice(NeuTube::Z_AXIS);

//    addDvidLabelSlice(NeuTube::Y_AXIS);
//    addDvidLabelSlice(NeuTube::Z_AXIS);
}

void ZFlyEmProofDoc::updateTileData()
{
  if (m_dvidReader.isReady()) {
    ZDvidInfo dvidInfo = m_dvidReader.readGrayScaleInfo();
    ZIntCuboid boundBox;
    if (dvidInfo.isValid()) {
      boundBox = ZIntCuboid(dvidInfo.getStartCoordinates(),
                       dvidInfo.getEndCoordinates());
    } else {
      boundBox = ZIntCuboid(ZIntPoint(0, 0, 0), ZIntPoint(13500, 11000, 10000));
    }

    ZStack *stack = ZStackFactory::makeVirtualStack(boundBox);
    loadStack(stack);

    ZDvidTileEnsemble *ensemble = getDvidTileEnsemble();

    if (ensemble == NULL) {
      ensemble = new ZDvidTileEnsemble;
      ensemble->addRole(ZStackObjectRole::ROLE_ACTIVE_VIEW);
      ensemble->setSource(ZStackObjectSourceFactory::MakeDvidTileSource());
      addObject(ensemble, true);
    }

    ensemble->setDvidTarget(getDvidTarget());

    ZDvidLabelSlice *slice = getDvidLabelSlice(NeuTube::Z_AXIS);

    if (slice == NULL) {
      addDvidLabelSlice(NeuTube::Z_AXIS);
    } else{
      slice->setDvidTarget(getDvidTarget());
    }
//    addDvidLabelSlice(NeuTube::Y_AXIS);
//    addDvidLabelSlice(NeuTube::Z_AXIS);
  }
}


void ZFlyEmProofDoc::addDvidLabelSlice(NeuTube::EAxis axis)
{
  ZDvidLabelSlice *labelSlice = new ZDvidLabelSlice;
  labelSlice->setSliceAxis(axis);
  labelSlice->setRole(ZStackObjectRole::ROLE_ACTIVE_VIEW);
  labelSlice->setDvidTarget(getDvidTarget());
  labelSlice->setSource(
        ZStackObjectSourceFactory::MakeDvidLabelSliceSource(axis));
  labelSlice->setBodyMerger(&m_bodyMerger);
  addObject(labelSlice, 0, true);
}


ZDvidTileEnsemble* ZFlyEmProofDoc::getDvidTileEnsemble() const
{
  QList<ZDvidTileEnsemble*> teList = getDvidTileEnsembleList();
  if (!teList.empty()) {
    return teList[0];
  }

  return NULL;
}

template<typename T>
static void UpdateDvidTargetForObject(ZFlyEmProofDoc *doc)
{
  QList<T*> objList = doc->getObjectList<T>();
  for (typename QList<T*>::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    T *obj = *iter;
    obj->setDvidTarget(doc->getDvidTarget());
//    doc->processObjectModified(obj);
  }
}

void ZFlyEmProofDoc::updateDvidTargetForObject()
{
  /*
  QList<ZDvidLabelSlice*> objList = getObjectList<ZDvidLabelSlice>();
  for (QList<ZDvidLabelSlice*>::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZDvidLabelSlice *obj = *iter;
    obj->setDvidTarget(getDvidTarget());
  }
  */
//  beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  UpdateDvidTargetForObject<ZDvidLabelSlice>(this);
  UpdateDvidTargetForObject<ZDvidSparseStack>(this);
  UpdateDvidTargetForObject<ZDvidSparsevolSlice>(this);
  UpdateDvidTargetForObject<ZDvidSynapseEnsemble>(this);
  UpdateDvidTargetForObject<ZDvidTileEnsemble>(this);
//  endObjectModifiedMode();
//  notifyObjectModified();
}


ZDvidLabelSlice* ZFlyEmProofDoc::getDvidLabelSlice(NeuTube::EAxis axis) const
{
  QList<ZDvidLabelSlice*> teList = getDvidLabelSliceList();
  std::string source = ZStackObjectSourceFactory::MakeDvidLabelSliceSource(axis);
  for (QList<ZDvidLabelSlice*>::iterator iter = teList.begin();
       iter != teList.end(); ++iter) {
    ZDvidLabelSlice *te = *iter;
    if (te->getSource() == source) {
      return te;
    }
  }

  return NULL;
}

ZDvidSynapseEnsemble* ZFlyEmProofDoc::getDvidSynapseEnsemble(
    NeuTube::EAxis axis) const
{
  QList<ZDvidSynapseEnsemble*> teList = getObjectList<ZDvidSynapseEnsemble>();
//  QList<ZStackObject*> teList = getObjectList(ZStackObject::TYPE_DVID_SYNAPE_ENSEMBLE);
  for (QList<ZDvidSynapseEnsemble*>::iterator iter = teList.begin();
       iter != teList.end(); ++iter) {
    ZDvidSynapseEnsemble *te = *iter;
    if (te->getSliceAxis() == axis) {
      return te;
    }
  }

  return NULL;
}

QList<ZDvidSynapseEnsemble*> ZFlyEmProofDoc::getDvidSynapseEnsembleList() const
{
  QList<ZStackObject*> objList =
      getObjectList(ZStackObject::TYPE_DVID_SYNAPE_ENSEMBLE);

  QList<ZDvidSynapseEnsemble*> teList;
  for (QList<ZStackObject*>::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    teList.append(dynamic_cast<ZDvidSynapseEnsemble*>(*iter));
  }

  return teList;
}

bool ZFlyEmProofDoc::hasDvidSynapse() const
{
  return !getDvidSynapseEnsembleList().isEmpty();
}

bool ZFlyEmProofDoc::hasDvidSynapseSelected() const
{
  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(NeuTube::Z_AXIS);
  if (se != NULL) {
    return getDvidSynapseEnsemble(NeuTube::Z_AXIS)->hasSelected();
  }

  return false;
}

std::set<ZIntPoint> ZFlyEmProofDoc::getSelectedSynapse() const
{
  std::set<ZIntPoint> selected;

  QList<ZDvidSynapseEnsemble*> seList = getDvidSynapseEnsembleList();
  for (QList<ZDvidSynapseEnsemble*>::const_iterator iter = seList.begin();
       iter != seList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    const std::set<ZIntPoint> &selectedSet = se->getSelector().getSelectedSet();
    selected.insert(selectedSet.begin(), selectedSet.end());
  }

  return selected;
}

void ZFlyEmProofDoc::tryMoveSelectedSynapse(
    const ZIntPoint &dest, NeuTube::EAxis axis)
{
  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(axis);
  if (se != NULL) {
    const std::set<ZIntPoint> &selectedSet = se->getSelector().getSelectedSet();
    const ZIntPoint &source = *selectedSet.begin();
    if (selectedSet.size() == 1) {
      se->moveSynapse(source, dest, ZDvidSynapseEnsemble::DATA_GLOBAL);
      processObjectModified(se);
    }

    QList<ZDvidSynapseEnsemble*> seList = getDvidSynapseEnsembleList();
    for (QList<ZDvidSynapseEnsemble*>::const_iterator iter = seList.begin();
         iter != seList.end(); ++iter) {
      ZDvidSynapseEnsemble *buddySe = *iter;
      if (buddySe != se) {
        buddySe->moveSynapse(source, dest, ZDvidSynapseEnsemble::DATA_LOCAL);
        processObjectModified(se);
      }
    }

    notifyObjectModified();
  }
}

void ZFlyEmProofDoc::addSynapse(
    const ZIntPoint &pt, ZDvidSynapse::EKind kind)
{
  ZDvidSynapse synapse;
  synapse.setPosition(pt);
  synapse.setKind(kind);
  synapse.setDefaultRadius();
  synapse.setDefaultColor();
  synapse.setUserName(NeuTube::GetCurrentUserName());

  ZDvidSynapseEnsemble::EDataScope scope = ZDvidSynapseEnsemble::DATA_GLOBAL;
  QList<ZDvidSynapseEnsemble*> seList = getDvidSynapseEnsembleList();
  for (QList<ZDvidSynapseEnsemble*>::const_iterator iter = seList.begin();
       iter != seList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    se->addSynapse(synapse, scope);
    scope = ZDvidSynapseEnsemble::DATA_LOCAL;
    processObjectModified(se);
  }

  notifyObjectModified();
}

void ZFlyEmProofDoc::removeSynapse(
    const ZIntPoint &pos, ZDvidSynapseEnsemble::EDataScope scope)
{
  QList<ZDvidSynapseEnsemble*> synapseList = getDvidSynapseEnsembleList();
  for (QList<ZDvidSynapseEnsemble*>::const_iterator iter = synapseList.begin();
       iter != synapseList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    se->removeSynapse(pos, scope);
    scope = ZDvidSynapseEnsemble::DATA_LOCAL;
    processObjectModified(se);
  }

  notifyObjectModified();
}

void ZFlyEmProofDoc::addSynapse(
    const ZDvidSynapse &synapse, ZDvidSynapseEnsemble::EDataScope scope)
{
  QList<ZDvidSynapseEnsemble*> synapseList = getDvidSynapseEnsembleList();
  for (QList<ZDvidSynapseEnsemble*>::const_iterator iter = synapseList.begin();
       iter != synapseList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    se->addSynapse(synapse, scope);
    scope = ZDvidSynapseEnsemble::DATA_LOCAL;
    processObjectModified(se);
  }

  notifyObjectModified();
}

void ZFlyEmProofDoc::moveSynapse(const ZIntPoint &from, const ZIntPoint &to)
{
  ZDvidSynapseEnsemble::EDataScope scope = ZDvidSynapseEnsemble::DATA_GLOBAL;
  QList<ZDvidSynapseEnsemble*> synapseList = getDvidSynapseEnsembleList();
  for (QList<ZDvidSynapseEnsemble*>::const_iterator iter = synapseList.begin();
       iter != synapseList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    se->moveSynapse(from, to, scope);
    scope = ZDvidSynapseEnsemble::DATA_LOCAL;
    processObjectModified(se);
  }

  notifyObjectModified();
}

void ZFlyEmProofDoc::updateSynapsePartner(const ZIntPoint &pos)
{
  QList<ZDvidSynapseEnsemble*> synapseList = getDvidSynapseEnsembleList();
  for (QList<ZDvidSynapseEnsemble*>::const_iterator iter = synapseList.begin();
       iter != synapseList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    se->updatePartner(se->getSynapse(pos, ZDvidSynapseEnsemble::DATA_LOCAL));
    processObjectModified(se);
  }

  notifyObjectModified();
}

void ZFlyEmProofDoc::updateSynapsePartner(const std::set<ZIntPoint> &posArray)
{
  QList<ZDvidSynapseEnsemble*> synapseList = getDvidSynapseEnsembleList();
  for (QList<ZDvidSynapseEnsemble*>::const_iterator iter = synapseList.begin();
       iter != synapseList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    for (std::set<ZIntPoint>::const_iterator iter = posArray.begin();
         iter != posArray.end(); ++iter) {
      se->updatePartner(
            se->getSynapse(*iter, ZDvidSynapseEnsemble::DATA_LOCAL));
    }
    processObjectModified(se);
  }

  notifyObjectModified();
}

void ZFlyEmProofDoc::deleteSelectedSynapse()
{
  QList<ZDvidSynapseEnsemble*> synapseList = getDvidSynapseEnsembleList();
  ZDvidSynapseEnsemble::EDataScope scope = ZDvidSynapseEnsemble::DATA_GLOBAL;
  const std::set<ZIntPoint> &selected = getSelectedSynapse();
  for (QList<ZDvidSynapseEnsemble*>::const_iterator iter = synapseList.begin();
       iter != synapseList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    for (std::set<ZIntPoint>::const_iterator iter = selected.begin();
         iter != selected.end(); ++iter) {
      const ZIntPoint &pt = *iter;
      se->removeSynapse(pt, scope);
      scope = ZDvidSynapseEnsemble::DATA_LOCAL;
    }
    processObjectModified(se);
  }
  notifyObjectModified();

  /*
  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble();
  if (se != NULL) {
    const std::set<ZIntPoint> &selected =
        se->getSelector().getSelectedSet();
    bool changed = false;
    for (std::set<ZIntPoint>::const_iterator iter = selected.begin();
         iter != selected.end(); ++iter) {
      const ZIntPoint &pt = *iter;
      if (se->removeSynapse(pt, ZDvidSynapseEnsemble::DATA_GLOBAL)) {
        changed = true;
      }
    }
    se->getSelector().deselectAll();

    if (changed) {
      processObjectModified(se);
      notifyObjectModified();
    }
  }
  */
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
    slice->setColor(getDvidLabelSlice(NeuTube::Z_AXIS)->getColor(
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

void ZFlyEmProofDoc::notifyBodyMergeEdited()
{
  emit bodyMergeEdited();
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

  cleanBodyAnnotationMap();
}

void ZFlyEmProofDoc::downloadBookmark(int x, int y, int z)
{
  if (m_dvidReader.isReady()) {
    ZJsonObject bookmarkJson = m_dvidReader.readBookmarkJson(x, y, z);
    ZFlyEmBookmark *bookmark = getBookmark(x, y, z);
    if (!bookmarkJson.isEmpty()) {
      bool newBookmark = false;
      if (bookmark == NULL) {
        bookmark = new ZFlyEmBookmark;
        newBookmark = true;
      }
      bookmark->loadDvidAnnotation(bookmarkJson);
      if (newBookmark) {
        addLocalBookmark(bookmark);
      } else {
        updateLocalBookmark(bookmark);
      }
    } else {
      if (bookmark != NULL) {
        removeObject(bookmark, true);
      }
    }
  }
}

void ZFlyEmProofDoc::downloadBookmark()
{
  if (m_dvidReader.isReady()) {
    ZJsonArray bookmarkJson =
        m_dvidReader.readTaggedBookmark("user:" + NeuTube::GetCurrentUserName());
    beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
    for (size_t i = 0; i < bookmarkJson.size(); ++i) {
      ZFlyEmBookmark *bookmark = new ZFlyEmBookmark;
      ZJsonObject bookmarkObj = ZJsonObject(bookmarkJson.value(i));
      bookmark->loadDvidAnnotation(bookmarkObj);
      if (m_dvidReader.isBookmarkChecked(bookmark->getCenter().toIntPoint())) {
        bookmark->setChecked(true);
        ZDvidAnnotation::AddProperty(bookmarkObj, "checked", true);
//        bookmarkObj.setProperty("checked", "1");
        m_dvidWriter.writeBookmark(bookmarkObj);
        m_dvidWriter.deleteBookmarkKey(*bookmark);
      }
      addObject(bookmark, true);
    }
    endObjectModifiedMode();
    notifyObjectModified();

    if (bookmarkJson.isEmpty()) {
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
          bookmark->addUserTag();
          if (m_dvidReader.isBookmarkChecked(bookmark->getCenter().toIntPoint())) {
            bookmark->setChecked(true);
          }
          m_dvidWriter.writeBookmark(*bookmark);
        }
        endObjectModifiedMode();
        notifyObjectModified();
      }
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

void ZFlyEmProofDoc::downloadSynapse(int x, int y, int z)
{
  QList<ZDvidSynapseEnsemble*> seList = getObjectList<ZDvidSynapseEnsemble>();
//  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  for (QList<ZDvidSynapseEnsemble*>::iterator iter = seList.begin();
       iter != seList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    se->update(x, y, z);
    processObjectModified(se);
  }
//  endObjectModifiedMode();
  notifyObjectModified();
}

void ZFlyEmProofDoc::downloadSynapse()
{
  if (!getDvidTarget().getSynapseName().empty()) {
    ZDvidSynapseEnsemble *synapseEnsemble = new ZDvidSynapseEnsemble;
    synapseEnsemble->setDvidTarget(getDvidTarget());
    synapseEnsemble->setSource(
          ZStackObjectSourceFactory::MakeDvidSynapseEnsembleSource());

    addObject(synapseEnsemble);
  }

#if 0
  const QString threadId = "downloadSynapse";
  if (!m_futureMap.isAlive(threadId)) {
    m_futureMap.removeDeadThread();
    QFuture<void> future =
        QtConcurrent::run(this, &ZFlyEmProofDoc::downloadSynapseFunc);
    m_futureMap[threadId] = future;
  }
#endif
}

void ZFlyEmProofDoc::downloadTodoList()
{
  ZFlyEmToDoList *todoList = new ZFlyEmToDoList;
  todoList->setDvidTarget(getDvidTarget());
  todoList->setSource(ZStackObjectSourceFactory::MakeTodoListEnsembleSource());
  addObject(todoList);
}

void ZFlyEmProofDoc::processBookmarkAnnotationEvent(ZFlyEmBookmark */*bookmark*/)
{
//  m_isCustomBookmarkSaved = false;
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
        std::vector<ZIntPoint> ptArray;
        for (QList<ZStackBall*>::const_iterator iter = ballList.begin();
             iter != ballList.end(); ++iter) {
          ZStackBall *ball = *iter;
          ZIntPoint pt = ball->getCenter().toIntPoint();
          if (box.contains(pt)) {
            ZIntPoint blockIndex = dvidInfo.getBlockIndex(pt);

            if (coarseBody.contains(blockIndex)) {
              ptArray.push_back(pt);
#if 0
              if (reader.readBodyIdAt(pt) == bodyId) {
                puncta.push_back(
                      new ZPunctum(ball->x(), ball->y(), ball->z(), ball->radius()));
              }
#endif
            }
          }
        }
        if (!ptArray.empty()) {
          std::vector<uint64_t> idArray = reader.readBodyIdAt(ptArray);
          for (size_t i = 0; i < idArray.size(); ++i) {
            if (idArray[i] == bodyId) {
              ZStackBall *ball = ballList[i];
              puncta.push_back(
                    new ZPunctum(ball->x(), ball->y(), ball->z(), ball->radius()));
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
  QElapsedTimer timer;
  timer.start();

  std::pair<std::vector<ZPunctum*>, std::vector<ZPunctum*> > synapse;
  ZDvidReader reader;
//  reader.setVerbose(false);
  const double radius = 50.0;
  if (reader.open(getDvidTarget())) {
    std::vector<ZDvidSynapse> synapseArray = reader.readSynapse(bodyId);

    std::vector<ZPunctum*> &tbar = synapse.first;
    std::vector<ZPunctum*> &psd = synapse.second;

    for (std::vector<ZDvidSynapse>::const_iterator iter = synapseArray.begin();
         iter != synapseArray.end(); ++iter) {
      const ZDvidSynapse &synapse = *iter;
      if (synapse.getKind() == ZDvidSynapse::KIND_PRE_SYN) {
        tbar.push_back(new ZPunctum(synapse.getPosition(), radius));
      } else if (synapse.getKind() == ZDvidSynapse::KIND_POST_SYN) {
        psd.push_back(new ZPunctum(synapse.getPosition(), radius));
      }
    }
    qDebug() << "Synapse loading time: " << timer.restart();
  }

  return synapse;
}

#if 0
std::pair<std::vector<ZPunctum*>, std::vector<ZPunctum*> >
ZFlyEmProofDoc::getSynapse(uint64_t bodyId)
{
  QElapsedTimer timer;
  timer.start();

  std::pair<std::vector<ZPunctum*>, std::vector<ZPunctum*> > synapse;
  ZDvidReader reader;
  reader.setVerbose(false);
  if (reader.open(getDvidTarget())) {
//    ZIntCuboid box = reader.readBodyBoundBox(bodyId);
//    qDebug() << "Bounding box reading time" << timer.restart();
//    int minZ = box.getFirstCorner().getZ();
//    int maxZ = box.getLastCorner().getZ();

    ZObject3dScan coarseBody = reader.readCoarseBody(bodyId);

    qDebug() << "Coarse body reading time" << timer.restart();

    ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
    std::vector<ZIntPoint> tbarPtArray;
    std::vector<ZIntPoint> psdPtArray;
    ZObject3dScan::ConstSegmentIterator segIter(&coarseBody);
    while (segIter.hasNext()) {
      const ZObject3dScan::Segment seg = segIter.next();
      ZIntCuboid blockBox = dvidInfo.getBlockBox(
            seg.getStart(), seg.getEnd(), seg.getY(), seg.getZ());
      std::vector<ZDvidSynapse> synapseArray = reader.readSynapse(blockBox);
      for (std::vector<ZDvidSynapse>::const_iterator iter = synapseArray.begin();
           iter != synapseArray.end(); ++iter) {
        const ZDvidSynapse &synapse = *iter;
        if (synapse.getKind() == ZDvidSynapse::KIND_PRE_SYN) {
          tbarPtArray.push_back(synapse.getPosition());
        } else if (synapse.getKind() == ZDvidSynapse::KIND_POST_SYN) {
          psdPtArray.push_back(synapse.getPosition());
        }
      }
    }
    qDebug() << "Coarse body screening time" << timer.restart();

    if (!tbarPtArray.empty()) {
      std::vector<ZPunctum*> &puncta = synapse.first;
      std::vector<uint64_t> idArray = reader.readBodyIdAt(tbarPtArray);
      for (size_t i = 0; i < idArray.size(); ++i) {
        if (idArray[i] == bodyId) {
          const ZIntPoint &pt = tbarPtArray[i];
          puncta.push_back(
                new ZPunctum(pt.getX(), pt.getY(), pt.getZ(), 50.0));
        }
      }
    }

    if (!psdPtArray.empty()) {
      std::vector<ZPunctum*> &puncta = synapse.second;
      std::vector<uint64_t> idArray = reader.readBodyIdAt(psdPtArray);
      for (size_t i = 0; i < idArray.size(); ++i) {
        if (idArray[i] == bodyId) {
          const ZIntPoint &pt = psdPtArray[i];
          puncta.push_back(
                new ZPunctum(pt.getX(), pt.getY(), pt.getZ(), 50.0));
        }
      }
    }

    qDebug() << "Final verifying time" << timer.elapsed();
  }

#if 0
  ZDvidSynapseEnsemble se;
  se.setDvidTarget(getDvidTarget());

  se.downloadForLabel(bodyId);

  std::pair<std::vector<ZPunctum*>, std::vector<ZPunctum*> > synapse;
  std::vector<ZPunctum*> &tbar = synapse.first;
  std::vector<ZPunctum*> &psd = synapse.second;

  ZDvidSynapseEnsemble::SynapseIterator sIter(&se);
  while (sIter.hasNext()) {
    ZDvidSynapse &s = sIter.next();
    if (s.getKind() == ZDvidSynapse::KIND_PRE_SYN) {
      tbar.push_back(new ZPunctum(
                       s.getPosition().getX(), s.getPosition().getY(),
                       s.getPosition().getZ(), 50.0));
    } else if (s.getKind() == ZDvidSynapse::KIND_POST_SYN) {
      psd.push_back(new ZPunctum(s.getPosition().getX(), s.getPosition().getY(),
                                 s.getPosition().getZ(), 50.0));
    }
  }
#endif

#if 0
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
    std::vector<ZIntPoint> tbarPtArray;
    std::vector<ZIntPoint> psdPtArray;
    for (int z = minZ; z <= maxZ; ++z) {
      if (tbar != NULL) {
        QList<ZStackBall*> ballList = tbar->getPunctaOnSlice(z);
        for (QList<ZStackBall*>::const_iterator iter = ballList.begin();
             iter != ballList.end(); ++iter) {
          ZStackBall *ball = *iter;
          ZIntPoint pt = ball->getCenter().toIntPoint();
          if (box.contains(pt)) {
            ZIntPoint blockIndex = dvidInfo.getBlockIndex(pt);

            if (coarseBody.contains(blockIndex)) {
              tbarPtArray.push_back(pt);
            }
          }
        }
      }

      if (psd != NULL) {
        QList<ZStackBall*> ballList = psd->getPunctaOnSlice(z);
        for (QList<ZStackBall*>::const_iterator iter = ballList.begin();
             iter != ballList.end(); ++iter) {
          ZStackBall *ball = *iter;
          ZIntPoint pt = ball->getCenter().toIntPoint();
          if (box.contains(pt)) {
            ZIntPoint blockIndex = dvidInfo.getBlockIndex(pt);

            if (coarseBody.contains(blockIndex)) {
              psdPtArray.push_back(pt);
            }
          }
        }
      }
    }
    if (!tbarPtArray.empty()) {
      std::vector<ZPunctum*> &puncta = synapse.first;
      std::vector<uint64_t> idArray = reader.readBodyIdAt(tbarPtArray);
      for (size_t i = 0; i < idArray.size(); ++i) {
        if (idArray[i] == bodyId) {
          const ZIntPoint &pt = tbarPtArray[i];
          puncta.push_back(
                new ZPunctum(pt.getX(), pt.getY(), pt.getZ(), 50.0));
        }
      }
    }

    if (!psdPtArray.empty()) {
      std::vector<ZPunctum*> &puncta = synapse.second;
      std::vector<uint64_t> idArray = reader.readBodyIdAt(psdPtArray);
      for (size_t i = 0; i < idArray.size(); ++i) {
        if (idArray[i] == bodyId) {
          const ZIntPoint &pt = psdPtArray[i];
          puncta.push_back(
                new ZPunctum(pt.getX(), pt.getY(), pt.getZ(), 50.0));
        }
      }
    }

  }
#endif
  return synapse;

}
#endif

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

#if 0
void ZFlyEmProofDoc::saveCustomBookmarkSlot()
{
  if (!m_isCustomBookmarkSaved) {
    std::cout << "Saving user bookmarks ..." << std::endl;
    saveCustomBookmark();
  }
}
#endif

#if 0
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
#endif

void ZFlyEmProofDoc::customNotifyObjectModified(ZStackObject::EType type)
{
  switch (type) {
  case ZStackObject::TYPE_FLYEM_BOOKMARK:
//    m_isCustomBookmarkSaved = false;
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

void ZFlyEmProofDoc::updateSplitRoi(ZRect2d *rect, bool appending)
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
    } else if (appending) {
      roi->setFirstCorner(rect->getFirstX(), rect->getFirstY(), rect->getZ());
      roi->setLastCorner(roi->getFirstCorner());
    }
  }
  m_splitSource.reset();

  if (appending) {
    ZIntCuboidObj *oldRoi = getSplitRoi();
    if (oldRoi != NULL) {
      roi->join(oldRoi->getCuboid());
    }
  }

  executeRemoveObjectCommand(getSplitRoi());

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

void ZFlyEmProofDoc::prepareNameBodyMap(const ZJsonValue &bodyInfoObj)
{
  ZSharedPointer<ZFlyEmNameBodyColorScheme> colorMap =
      getColorScheme<ZFlyEmNameBodyColorScheme>(BODY_COLOR_NAME);
  if (colorMap.get() != NULL) {
    colorMap->prepareNameMap(bodyInfoObj);

    emit bodyMapReady();
  }
}

void ZFlyEmProofDoc::updateSequencerBodyMap(
    const ZFlyEmSequencerColorScheme &colorScheme)
{
  ZSharedPointer<ZFlyEmSequencerColorScheme> colorMap =
      getColorScheme<ZFlyEmSequencerColorScheme>(BODY_COLOR_SEQUENCER);
  if (colorMap.get() != NULL) {
    *(colorMap.get()) = colorScheme;
    if (isActive(BODY_COLOR_SEQUENCER)) {
      updateBodyColor(BODY_COLOR_SEQUENCER);
    }
  }
}

#if 0
void ZFlyEmProofDoc::useBodyNameMap(bool on)
{
  if (getDvidLabelSlice() != NULL) {
    if (on) {
      if (m_activeBodyColorMap.get() == NULL) {
        ZSharedPointer<ZFlyEmNameBodyColorScheme> colorMap =
            getColorScheme<ZFlyEmNameBodyColorScheme>(BODY_COLOR_NAME);
        if (colorMap.get() != NULL) {
          colorMap->setDvidTarget(getDvidTarget());
          colorMap->prepareNameMap();
        }

        m_activeBodyColorMap =
            ZSharedPointer<ZFlyEmBodyColorScheme>(colorMap);
      }
      getDvidLabelSlice()->setCustomColorMap(m_activeBodyColorMap);
    } else {
      getDvidLabelSlice()->removeCustomColorMap();
    }

    processObjectModified(getDvidLabelSlice());
    notifyObjectModified();
  }
}
#endif

void ZFlyEmProofDoc::updateBodyColor(EBodyColorMap type)
{
  ZDvidLabelSlice *slice = getDvidLabelSlice(NeuTube::Z_AXIS);
  if (slice != NULL) {
    ZSharedPointer<ZFlyEmBodyColorScheme> colorMap = getColorScheme(type);
    if (colorMap.get() != NULL) {
      slice->setCustomColorMap(colorMap);
    } else {
      slice->removeCustomColorMap();
    }

    processObjectModified(slice);
    notifyObjectModified();
  }
}

void ZFlyEmProofDoc::selectBody(uint64_t bodyId)
{
  QList<ZDvidLabelSlice*> sliceList = getDvidLabelSliceList();
//  ZDvidLabelSlice *slice = getDvidLabelSlice();
  for (QList<ZDvidLabelSlice*>::iterator iter = sliceList.begin();
       iter != sliceList.end(); ++iter) {
    ZDvidLabelSlice *slice = *iter;
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

ZSharedPointer<ZFlyEmBodyColorScheme>
ZFlyEmProofDoc::getColorScheme(EBodyColorMap type)
{
  if (!m_colorMapConfig.contains(type)) {
    switch (type) {
    case BODY_COLOR_NORMAL:
      m_colorMapConfig[type] = ZSharedPointer<ZFlyEmBodyColorScheme>();
      break;
    case BODY_COLOR_NAME:
    {
      ZFlyEmNameBodyColorScheme *colorScheme = new ZFlyEmNameBodyColorScheme;
      colorScheme->setDvidTarget(getDvidTarget());
      m_colorMapConfig[type] =
          ZSharedPointer<ZFlyEmBodyColorScheme>(colorScheme);
    }
      break;
    case BODY_COLOR_SEQUENCER:
      m_colorMapConfig[type] =
          ZSharedPointer<ZFlyEmBodyColorScheme>(new ZFlyEmSequencerColorScheme);
      break;
    }
  }

  return m_colorMapConfig[type];
}

template <typename T>
ZSharedPointer<T> ZFlyEmProofDoc::getColorScheme(EBodyColorMap type)
{
  ZSharedPointer<ZFlyEmBodyColorScheme> colorScheme = getColorScheme(type);
  if (colorScheme.get() != NULL) {
    return Shared_Dynamic_Cast<T>(colorScheme);
  }

  return ZSharedPointer<T>();
}

void ZFlyEmProofDoc::activateBodyColorMap(const QString &option)
{
  if (option == "Name") {
    activateBodyColorMap(BODY_COLOR_NAME);
  } else if (option == "Sequencer") {
    activateBodyColorMap(BODY_COLOR_SEQUENCER);
  } else {
    activateBodyColorMap(BODY_COLOR_NORMAL);
  }
}

void ZFlyEmProofDoc::activateBodyColorMap(EBodyColorMap colorMap)
{
  if (!isActive(colorMap)) {
    updateBodyColor(colorMap);
    m_activeBodyColorMap = getColorScheme(colorMap);
  }
}

bool ZFlyEmProofDoc::isActive(EBodyColorMap type)
{
  return m_activeBodyColorMap.get() == getColorScheme(type).get();
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

void ZFlyEmProofDoc::recordBodySelection()
{
  ZDvidLabelSlice *slice = getDvidLabelSlice(NeuTube::Z_AXIS);
  if (slice != NULL) {
    slice->recordSelection();
  }
}

void ZFlyEmProofDoc::processBodySelection()
{
  ZDvidLabelSlice *slice = getDvidLabelSlice(NeuTube::Z_AXIS);
  if (slice != NULL) {
    slice->processSelection();
  }
}

void ZFlyEmProofDoc::executeUnlinkSynapseCommand()
{

  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(NeuTube::Z_AXIS);
  if (se != NULL) {
    const std::set<ZIntPoint> &selected = se->getSelector().getSelectedSet();
    if (selected.size() > 1) {
      ZStackDocCommand::DvidSynapseEdit::UnlinkSynapse *command =
          new ZStackDocCommand::DvidSynapseEdit::UnlinkSynapse(this, selected);
      pushUndoCommand(command);
    }
  }
}

void ZFlyEmProofDoc::executeLinkSynapseCommand()
{
  QUndoCommand *command =
      new ZStackDocCommand::DvidSynapseEdit::CompositeCommand(this);

  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(NeuTube::Z_AXIS);
  if (se != NULL) {
    const std::set<ZIntPoint> &selected =
        se->getSelector().getSelectedSet();
    std::vector<ZDvidSynapse> selectedPresyn;
    std::vector<ZDvidSynapse> selectedPostsyn;

    for (std::set<ZIntPoint>::const_iterator iter = selected.begin();
         iter != selected.end(); ++iter) {
      ZDvidSynapse &synapse =
          se->getSynapse(*iter, ZDvidSynapseEnsemble::DATA_GLOBAL);
      if (synapse.getKind() == ZDvidSynapse::KIND_PRE_SYN) {
        selectedPresyn.push_back(synapse);
      } else if (synapse.getKind() == ZDvidSynapse::KIND_POST_SYN) {
        selectedPostsyn.push_back(synapse);
      }
    }

    if (selectedPresyn.size() == 1) {
      ZDvidSynapse &presyn = selectedPresyn.front();
      ZStackDocCommand::DvidSynapseEdit::LinkSynapse *linkCommand =
          new ZStackDocCommand::DvidSynapseEdit::LinkSynapse(
            this, presyn.getPosition(), command);
      for (std::vector<ZDvidSynapse>::const_iterator
           iter = selectedPostsyn.begin(); iter != selectedPostsyn.end();
           ++iter) {
        const ZDvidSynapse& postsyn = *iter;
        linkCommand->addRelation(
              postsyn.getPosition(), ZDvidSynapse::Relation::GetName(
                ZDvidSynapse::Relation::RELATION_PRESYN_TO));
        new ZStackDocCommand::DvidSynapseEdit::LinkSynapse(
              this, postsyn.getPosition(), presyn.getPosition(),
              ZDvidSynapse::Relation::GetName(
                ZDvidSynapse::Relation::RELATION_POSTSYN_TO),
              command);
      }
    }

//    qDebug() << "#Commands: " << command->childCount();

    if (command->childCount() > 0) {
      pushUndoCommand(command);
    }
  }
}

void ZFlyEmProofDoc::executeRemoveBookmarkCommand()
{
  QList<ZFlyEmBookmark*> bookmarkList =
      getSelectedObjectList<ZFlyEmBookmark>(ZStackObject::TYPE_FLYEM_BOOKMARK);
  executeRemoveBookmarkCommand(bookmarkList);
}

void ZFlyEmProofDoc::executeRemoveBookmarkCommand(ZFlyEmBookmark *bookmark)
{
  ZStackDocCommand::FlyEmBookmarkEdit::RemoveBookmark *command =
      new ZStackDocCommand::FlyEmBookmarkEdit::RemoveBookmark(this, bookmark);
  if (command->hasRemoving()) {
    pushUndoCommand(command);
  }
}

void ZFlyEmProofDoc::executeRemoveBookmarkCommand(
    const QList<ZFlyEmBookmark *> &bookmarkList)
{
  ZStackDocCommand::FlyEmBookmarkEdit::RemoveBookmark *command =
      new ZStackDocCommand::FlyEmBookmarkEdit::RemoveBookmark(this, NULL);
  for (QList<ZFlyEmBookmark*>::const_iterator iter = bookmarkList.begin();
       iter != bookmarkList.end(); ++iter) {
    command->addRemoving(*iter);
  }

  if (command->hasRemoving()) {
    pushUndoCommand(command);
  }
}

void ZFlyEmProofDoc::executeAddBookmarkCommand(ZFlyEmBookmark *bookmark)
{
  if (bookmark != NULL) {
    ZStackDocCommand::FlyEmBookmarkEdit::AddBookmark *command =
        new ZStackDocCommand::FlyEmBookmarkEdit::AddBookmark(this, bookmark);
    pushUndoCommand(command);
  }
}

void ZFlyEmProofDoc::executeRemoveSynapseCommand()
{
  QUndoCommand *command =
      new ZStackDocCommand::DvidSynapseEdit::CompositeCommand(this);

  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(NeuTube::Z_AXIS);
  if (se != NULL) {
    const std::set<ZIntPoint> &selected =
        se->getSelector().getSelectedSet();
    for (std::set<ZIntPoint>::const_iterator iter = selected.begin();
         iter != selected.end(); ++iter) {
      const ZIntPoint &pt = *iter;
      new ZStackDocCommand::DvidSynapseEdit::RemoveSynapse(
            this, pt.getX(), pt.getY(), pt.getZ(), command);
    }
    se->getSelector().deselectAll();

    if (command->childCount() > 0) {
      pushUndoCommand(command);
    }
  }
}

void ZFlyEmProofDoc::executeAddSynapseCommand(const ZDvidSynapse &synapse)
{
  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(NeuTube::Z_AXIS);
  if (se != NULL) {
    QUndoCommand *command = new ZStackDocCommand::DvidSynapseEdit::AddSynapse(
          this, synapse);
    pushUndoCommand(command);
  }
}

void ZFlyEmProofDoc::executeMoveSynapseCommand(const ZIntPoint &dest)
{
  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(NeuTube::Z_AXIS);
  if (se != NULL) {
    const std::set<ZIntPoint> &selectedSet = se->getSelector().getSelectedSet();
    if (selectedSet.size() == 1) {
      QUndoCommand *command =
          new ZStackDocCommand::DvidSynapseEdit::MoveSynapse(
            this, *selectedSet.begin(), dest);
      pushUndoCommand(command);
    }
  }
}

void ZFlyEmProofDoc::removeLocalBookmark(ZFlyEmBookmark *bookmark)
{
  if (bookmark != NULL) {
    if (bookmark->isCustom()) {
      bookmark->setSelected(false);
      removeObject(bookmark, false);
      emit userBookmarkModified();
    }
  }
}

void ZFlyEmProofDoc::updateLocalBookmark(ZFlyEmBookmark *bookmark)
{
  if (bookmark != NULL) {
    if (bookmark->isCustom()) {
      processObjectModified(bookmark);
      notifyObjectModified();
      emit userBookmarkModified();
    }
  }
}

void ZFlyEmProofDoc::copyBookmarkFrom(const ZFlyEmProofDoc *doc)
{
  QList<ZFlyEmBookmark*> objList = doc->getObjectList<ZFlyEmBookmark>();

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  for (QList<ZFlyEmBookmark*>::const_iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    const ZFlyEmBookmark *bookmark = *iter;
    addObject(bookmark->clone(), false);
  }
  endObjectModifiedMode();
  notifyObjectModified();
}

void ZFlyEmProofDoc::notifySynapseEdited(const ZDvidSynapse &synapse)
{
  emit synapseEdited(synapse.getPosition().getX(),
                     synapse.getPosition().getY(),
                     synapse.getPosition().getZ());
}

void ZFlyEmProofDoc::notifySynapseEdited(const ZIntPoint &synapse)
{
  emit synapseEdited(synapse.getX(), synapse.getY(), synapse.getZ());
}


void ZFlyEmProofDoc::notifyBookmarkEdited(const ZFlyEmBookmark *bookmark)
{
  if (bookmark != NULL) {
    ZIntPoint center = bookmark->getCenter().toIntPoint();
    emit bookmarkEdited(center.getX(), center.getY(), center.getZ());
  }
}

void ZFlyEmProofDoc::notifyBookmarkEdited(
    const std::vector<ZFlyEmBookmark *> &bookmarkArray)
{
  for (std::vector<ZFlyEmBookmark *>::const_iterator iter = bookmarkArray.begin();
       iter != bookmarkArray.end(); ++iter) {
    notifyBookmarkEdited(*iter);
  }
}

void ZFlyEmProofDoc::removeLocalBookmark(
    const std::vector<ZFlyEmBookmark*> &bookmarkArray)
{
  for (std::vector<ZFlyEmBookmark*>::const_iterator iter = bookmarkArray.begin();
       iter != bookmarkArray.end(); ++iter) {
    ZFlyEmBookmark *bookmark = *iter;
    bookmark->setSelected(false);
    removeObject(bookmark, false);
  }

  if (!bookmarkArray.empty()) {
    emit userBookmarkModified();
  }
}

void ZFlyEmProofDoc::addLocalBookmark(ZFlyEmBookmark *bookmark)
{
  if (bookmark != NULL) {
    addObject(bookmark, false);

    emit userBookmarkModified();
  }
}

void ZFlyEmProofDoc::addLocalBookmark(
    const std::vector<ZFlyEmBookmark *> &bookmarkArray)
{
  for (std::vector<ZFlyEmBookmark*>::const_iterator iter = bookmarkArray.begin();
       iter != bookmarkArray.end(); ++iter) {
    ZFlyEmBookmark *bookmark = *iter;
    addObject(bookmark, false);
  }

  if (!bookmarkArray.empty()) {
    emit userBookmarkModified();
  }
}

ZFlyEmBookmark* ZFlyEmProofDoc::getBookmark(int x, int y, int z) const
{
  QList<ZFlyEmBookmark*> bookmarkList = getObjectList<ZFlyEmBookmark>();

  ZFlyEmBookmark *bookmark = NULL;
  for (QList<ZFlyEmBookmark*>::iterator iter = bookmarkList.begin();
       iter != bookmarkList.end(); ++iter) {
    bookmark = *iter;
    if (iround(bookmark->getCenter().x()) == x &&
        iround(bookmark->getCenter().y()) == y &&
        iround(bookmark->getCenter().z()) == z) {
      break;
    }
    bookmark = NULL;
  }

  return bookmark;
}

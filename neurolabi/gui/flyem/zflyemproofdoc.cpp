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
#include "dvid/zdvidsynapsecommand.h"
#include "dvid/zflyembookmarkcommand.h"
#include "dvid/zdvidannotation.h"
#include "dvid/zdvidannotationcommand.h"
#include "flyem/zflyemproofdoccommand.h"
#include "dialogs/zflyemsynapseannotationdialog.h"
#include "zprogresssignal.h"
#include "zstackwatershed.h"
#include "zstackarray.h"
#include "zsleeper.h"

ZFlyEmProofDoc::ZFlyEmProofDoc(QObject *parent) :
  ZStackDoc(parent)
{
  init();
}

void ZFlyEmProofDoc::init()
{
  setTag(NeuTube::Document::FLYEM_PROOFREAD);

  m_loadingAssignedBookmark = false;
  m_analyzer.setDvidReader(&m_dvidReader);
  m_supervisor = new ZFlyEmSupervisor(this);

  m_routineCheck = true;

  initTimer();
  initAutoSave();

  connectSignalSlot();
}

void ZFlyEmProofDoc::initTimer()
{
//  m_bookmarkTimer = new QTimer(this);
//  m_bookmarkTimer->setInterval(60000);
//  m_bookmarkTimer->start();

  m_routineTimer = new QTimer(this);
  m_routineTimer->setInterval(10000);
  if (m_routineCheck) {
    m_routineTimer->start();
  }
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
    connect(m_routineTimer, SIGNAL(timeout()), this, SLOT(runRoutineCheck()));

  /*
  connect(m_bookmarkTimer, SIGNAL(timeout()),
          this, SLOT(saveCustomBookmarkSlot()));
          */
}

ZFlyEmSupervisor* ZFlyEmProofDoc::getSupervisor() const
{
  if (getDvidTarget().isSupervised()) {
    return m_supervisor;
  }

  return NULL;
}

void ZFlyEmProofDoc::runRoutineCheck()
{
  if (m_routineCheck) {
    if (NeutubeConfig::GetVerboseLevel() >= 5) {
      if (getSupervisor() != NULL) {
        QElapsedTimer timer;
        timer.start();
        int statusCode = getSupervisor()->testServer();
        if (statusCode == 200) {
          ZOUT(LTRACE(), 5) << "HEAD time:"
                            << getSupervisor()->getMainUrl() + ":"
                            << timer.elapsed() << "ms";
        } else {
          LWARN() << "API load failed:" << getDvidTarget().getAddressWithPort();
        }
      }
    }
#if 0
      if (!m_routineReader.isReady()) {
        m_routineReader.open(getDvidTarget());
      }

      if (m_routineReader.isReady()) {
        QElapsedTimer timer;
        timer.start();
        m_routineReader.testApiLoad();

        if (m_routineReader.getStatusCode() == 200) {
          ZOUT(LTRACE(), 5) << "API load time:"
                            << getDvidTarget().getAddressWithPort() + ":"
                            << timer.elapsed() << "ms";
        } else {
          LWARN() << "API load failed:" << getDvidTarget().getAddressWithPort();
        }
      }
    }
#endif
  }
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

bool ZFlyEmProofDoc::hasBodySelected() const
{
  QList<ZDvidLabelSlice*> sliceList = getDvidLabelSliceList();

  for (QList<ZDvidLabelSlice*>::const_iterator iter = sliceList.begin();
       iter != sliceList.end(); ++iter) {
    const ZDvidLabelSlice *labelSlice = *iter;
    if (!labelSlice->getSelectedOriginal().empty()) {
      return true;
    }
//    finalSet.insert(selected.begin(), selected.end());
  }

  return false;
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
      LWARN() << "Inconsistent body selection: " << bodyId;
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

void ZFlyEmProofDoc::unmergeSelected()
{
  ZFlyEmProofDocCommand::UnmergeBody *command =
      new ZFlyEmProofDocCommand::UnmergeBody(this);
  pushUndoCommand(command);
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
                        QString("Failed to merge. Is the librarian sever (%1) ready?").
                        arg(getDvidTarget().getSupervisor().c_str()),
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
    ZOUT(LTRACE(), 5) << writer.getStandardOutput();
    emit messageGenerated(
          ZWidgetMessage("Cannot save annotation.", NeuTube::MSG_ERROR));
  }
}

void ZFlyEmProofDoc::initData(
    const std::string &type, const std::string &dataName)
{
  if (!type.empty() && !dataName.empty()) {
    if (!m_dvidReader.hasData(dataName)) {
      m_dvidWriter.createData(type, dataName);
      if (!m_dvidWriter.isStatusOk()) {
        emit messageGenerated(
              ZWidgetMessage(QString("Failed to create data: ") + dataName.c_str(),
                             NeuTube::MSG_ERROR));
      }
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
    initData("keyvalue", ZDvidData::GetName(ZDvidData::ROLE_MERGE_OPERATION));
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

  if (getSupervisor() != NULL) {
    getSupervisor()->setDvidTarget(m_dvidTarget);
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

    //Download ROI
    if (!getDvidTarget().getRoiName().empty()) {
      ZObject3dScan *obj =
          m_dvidReader.readRoi(getDvidTarget().getRoiName(), NULL);
      if (obj != NULL) {
        if (!obj->isEmpty()) {
#ifdef _DEBUG_
          std::cout << "ROI Size:" << obj->getVoxelNumber() << std::endl;
#endif
          obj->setColor(0, 255, 0);
          obj->setZOrder(2);
          obj->setTarget(ZStackObject::TARGET_WIDGET);
          obj->useCosmeticPen(true);
          obj->addRole(ZStackObjectRole::ROLE_ROI_MASK);
          obj->setDsIntv(31, 31, 31);
          obj->addVisualEffect(NeuTube::Display::SparseObject::VE_PLANE_BOUNDARY);
          addObject(obj);
//          obj->setTarget(ZStackObject::TARGET_TILE_CANVAS);
        } else {
          delete obj;
        }
      }

    }
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
  ZOUT(LTRACE(), 5) << "Update dvid target";
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
  UpdateDvidTargetForObject<ZFlyEmToDoList>(this);
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
  ZOUT(LTRACE(), 5) << "Get dvid synapses";
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
  ZOUT(LTRACE(), 5) << "Get dvid synapses";
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

bool ZFlyEmProofDoc::hasTodoItemSelected() const
{
  ZOUT(LTRACE(), 5) << "Check todo selection";
  QList<ZFlyEmToDoList*> todoList = getObjectList<ZFlyEmToDoList>();
  for (QList<ZFlyEmToDoList*>::const_iterator iter = todoList.begin();
       iter != todoList.end(); ++iter) {
    ZFlyEmToDoList *td = *iter;
    if (td->hasSelected()) {
      return true;
    }
  }

  return false;
}

void ZFlyEmProofDoc::notifyTodoItemModified(
    const ZIntPoint &pt, bool emitingEdit)
{
  uint64_t bodyId = m_dvidReader.readBodyIdAt(pt);
  if (bodyId > 0) {
    emit todoModified(bodyId);
  }

  if (emitingEdit) {
    emit todoEdited(pt.getX(), pt.getY(), pt.getZ());
  }
}

void ZFlyEmProofDoc::notifyTodoItemModified(
    const std::vector<ZIntPoint> &ptArray, bool emitingEdit)
{
  std::vector<uint64_t> bodyIdArray = m_dvidReader.readBodyIdAt(ptArray);
  std::set<uint64_t> bodyIdSet;
  bodyIdSet.insert(bodyIdArray.begin(), bodyIdArray.end());
  for (std::vector<uint64_t>::const_iterator iter = bodyIdArray.begin();
       iter != bodyIdArray.end(); ++iter) {
    emit todoModified(*iter);
  }

  if (emitingEdit) {
    for (std::vector<ZIntPoint>::const_iterator iter = ptArray.begin();
         iter != ptArray.end(); ++iter) {
      const ZIntPoint &pt = *iter;
      emit todoEdited(pt.getX(), pt.getY(), pt.getZ());
    }
  }
}

void ZFlyEmProofDoc::checkTodoItem(bool checking)
{
  ZOUT(LTRACE(), 5) << "Check to do items";
  QList<ZFlyEmToDoList*> todoList = getObjectList<ZFlyEmToDoList>();

  std::vector<ZIntPoint> ptArray;
  for (QList<ZFlyEmToDoList*>::const_iterator iter = todoList.begin();
       iter != todoList.end(); ++iter) {
    ZFlyEmToDoList *td = *iter;
    const std::set<ZIntPoint> &selectedSet = td->getSelector().getSelectedSet();
    for (std::set<ZIntPoint>::const_iterator iter = selectedSet.begin();
         iter != selectedSet.end(); ++iter) {
      ZFlyEmToDoItem item = td->getItem(*iter, ZFlyEmToDoList::DATA_LOCAL);
      if (item.isValid()) {
        if (checking != item.isChecked()) {
          item.setChecked(checking);
          td->addItem(item, ZFlyEmToDoList::DATA_GLOBAL);
          ptArray.push_back(item.getPosition());
        }
      }
    }
    if (!selectedSet.empty()) {
      processObjectModified(td);
      notifyTodoItemModified(ptArray, true);
    }
  }

  notifyObjectModified();
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
}

void ZFlyEmProofDoc::annotateSelectedSynapse(
    ZFlyEmSynapseAnnotationDialog *dlg, NeuTube::EAxis axis)
{
  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(axis);
  if (se != NULL) {
    if (se->getSelector().getSelectedSet().size() == 1) {
      ZIntPoint pt = *(se->getSelector().getSelectedSet().begin());
      ZDvidSynapse synapse =
          se->getSynapse(pt, ZDvidSynapseEnsemble::DATA_GLOBAL);
      dlg->setOption(synapse.getKind());
      dlg->setConfidence(synapse.getConfidence());
      dlg->setAnnotation(synapse.getAnnotation().c_str());
      if (dlg->exec()) {
        annotateSynapse(pt, dlg->getPropJson(), axis);
      }
    }
  }
}

void ZFlyEmProofDoc::setRoutineCheck(bool on)
{
  m_routineCheck = on;
}

void ZFlyEmProofDoc::annotateSynapse(
    const ZIntPoint &pt, ZJsonObject propJson, NeuTube::EAxis axis)
{
  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(axis);
  if (se != NULL) {
    se->annotateSynapse(pt, propJson, ZDvidSynapseEnsemble::DATA_GLOBAL);
    processObjectModified(se);

    QList<ZDvidSynapseEnsemble*> seList = getDvidSynapseEnsembleList();
    for (QList<ZDvidSynapseEnsemble*>::const_iterator iter = seList.begin();
         iter != seList.end(); ++iter) {
      ZDvidSynapseEnsemble *buddySe = *iter;
      if (buddySe != se) {
        buddySe->annotateSynapse(
              pt, propJson, ZDvidSynapseEnsemble::DATA_LOCAL);
        processObjectModified(se);
      }
    }

    notifySynapseEdited(pt);

    notifyObjectModified();
  }
}

void ZFlyEmProofDoc::annotateSelectedSynapse(
    ZJsonObject propJson, NeuTube::EAxis axis)
{
  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(axis);
  if (se != NULL) {
    if (se->getSelector().getSelectedSet().size() == 1) {
      ZIntPoint pt = *(se->getSelector().getSelectedSet().begin());
      annotateSynapse(pt, propJson, axis);
    }
  }
}

void ZFlyEmProofDoc::addSynapse(
    const ZIntPoint &pt, ZDvidSynapse::EKind kind,
    ZDvidSynapseEnsemble::EDataScope scope)
{
  ZDvidSynapse synapse;
  synapse.setPosition(pt);
  synapse.setKind(kind);
  synapse.setDefaultRadius();
  synapse.setDefaultColor();
  synapse.setUserName(NeuTube::GetCurrentUserName());

//  ZDvidSynapseEnsemble::EDataScope scope = ZDvidSynapseEnsemble::DATA_GLOBAL;
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

void ZFlyEmProofDoc::removeTodoItem(
    const ZIntPoint &pos, ZFlyEmToDoList::EDataScope scope)
{
  ZOUT(LTRACE(), 5) << "Remove to do items";
  QList<ZFlyEmToDoList*> todoList = getObjectList<ZFlyEmToDoList>();
  for (QList<ZFlyEmToDoList*>::const_iterator iter = todoList.begin();
       iter != todoList.end(); ++iter) {
    ZFlyEmToDoList *se = *iter;
    se->removeItem(pos, scope);
    scope = ZFlyEmToDoList::DATA_LOCAL;
    processObjectModified(se);
  }

  notifyTodoItemModified(pos);

  notifyObjectModified();
}

void ZFlyEmProofDoc::addTodoItem(const ZIntPoint &pos)
{
  ZFlyEmToDoItem item(pos);
  item.setUserName(NeuTube::GetCurrentUserName());

  addTodoItem(item, ZFlyEmToDoList::DATA_GLOBAL);
}

void ZFlyEmProofDoc::addTodoItem(
    const ZFlyEmToDoItem &item, ZFlyEmToDoList::EDataScope scope)
{
  ZOUT(LTRACE(), 5) << "Add to do item";
  QList<ZFlyEmToDoList*> seList = getObjectList<ZFlyEmToDoList>();
  for (QList<ZFlyEmToDoList*>::const_iterator iter = seList.begin();
       iter != seList.end(); ++iter) {
    ZFlyEmToDoList *se = *iter;
    se->addItem(item, scope);
    scope = ZFlyEmToDoList::DATA_LOCAL;
    processObjectModified(se);
  }

  notifyTodoItemModified(item.getPosition());

  notifyObjectModified();
}

void ZFlyEmProofDoc::addSynapse(
    const ZDvidSynapse &synapse, ZDvidSynapseEnsemble::EDataScope scope)
{
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  QList<ZDvidSynapseEnsemble*> synapseList = getDvidSynapseEnsembleList();
  for (QList<ZDvidSynapseEnsemble*>::const_iterator iter = synapseList.begin();
       iter != synapseList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    se->addSynapse(synapse, scope);
    scope = ZDvidSynapseEnsemble::DATA_LOCAL;
    processObjectModified(se);
  }
  endObjectModifiedMode();

  notifyObjectModified();
}

void ZFlyEmProofDoc::moveSynapse(
    const ZIntPoint &from, const ZIntPoint &to,
    ZDvidSynapseEnsemble::EDataScope scope)
{
//  ZDvidSynapseEnsemble::EDataScope scope = ZDvidSynapseEnsemble::DATA_GLOBAL;
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  QList<ZDvidSynapseEnsemble*> synapseList = getDvidSynapseEnsembleList();
  for (QList<ZDvidSynapseEnsemble*>::const_iterator iter = synapseList.begin();
       iter != synapseList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    se->moveSynapse(from, to, scope);
//    se->setConfidence(to, 1.0, scope);
    scope = ZDvidSynapseEnsemble::DATA_LOCAL;
    processObjectModified(se);
  }
  endObjectModifiedMode();

  notifyObjectModified();
}

void ZFlyEmProofDoc::syncSynapse(const ZIntPoint &pt)
{
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  QList<ZDvidSynapseEnsemble*> synapseList = getDvidSynapseEnsembleList();
  for (QList<ZDvidSynapseEnsemble*>::const_iterator iter = synapseList.begin();
       iter != synapseList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    se->update(pt);
//    se->getSynapse(pt, ZDvidSynapseEnsemble::DATA_SYNC);
    processObjectModified(se);
  }
  endObjectModifiedMode();

  notifyObjectModified();
}

void ZFlyEmProofDoc::repairSynapse(const ZIntPoint &pt)
{
//  QList<ZDvidSynapseEnsemble*> synapseList = getDvidSynapseEnsembleList();

//  getDvidReader().readSynapseJson(pt);
  ZJsonObject synapseJson = getDvidReader().readSynapseJson(pt);
  ZJsonArray modifiedJson;
  if (synapseJson.isEmpty()) {
    removeSynapse(pt, ZDvidSynapseEnsemble::DATA_LOCAL);
  } else {
    ZJsonArray relJsonArray = ZDvidAnnotation::GetRelationJson(synapseJson);
    int removalCount = 0;
    std::vector<ZIntPoint> partnerArray =
        ZDvidAnnotation::GetPartners(synapseJson);
//    ZDvidAnnotation::EKind hostKind = ZDvidAnnotation::GetKind(synapseJson);
    for (size_t i = 0; i < relJsonArray.size(); ++i) {
      const ZIntPoint &partnerPos = partnerArray[i];
      ZJsonObject partnerJson = getDvidReader().readSynapseJson(partnerPos);
      if (partnerJson.isEmpty()) {
        removeSynapse(partnerPos, ZDvidSynapseEnsemble::DATA_LOCAL);
        relJsonArray.remove(i);
        ++removalCount;
        --i;
      } else {
        ZJsonObject forwardRelJson(relJsonArray.value(i));
        ZJsonArray backwardRelJsonArray =
            ZDvidAnnotation::GetRelationJson(partnerJson);

        if (ZDvidAnnotation::MatchRelation(
              backwardRelJsonArray, pt, forwardRelJson) < 0) {
          ZDvidAnnotation::RemoveRelation(partnerJson, pt);
          ZDvidAnnotation::AddRelation(
                partnerJson, pt, ZDvidAnnotation::GetMatchingRelation(
                  ZDvidAnnotation::GetRelationType(forwardRelJson)));
          modifiedJson.append(partnerJson);
        }
      }
    }

    if (removalCount > 0) {
      modifiedJson.append(synapseJson);
    }
  }
  if (!modifiedJson.isEmpty()) {
    getDvidWriter().writeSynapse(modifiedJson);
    updateSynapsePartner(pt);
  }
}

void ZFlyEmProofDoc::syncMoveSynapse(const ZIntPoint &from, const ZIntPoint &to)
{
  moveSynapse(from, to, ZDvidSynapseEnsemble::DATA_LOCAL);
  syncSynapse(to);
}

void ZFlyEmProofDoc::updateSynapsePartner(const ZIntPoint &pos)
{
  QList<ZDvidSynapseEnsemble*> synapseList = getDvidSynapseEnsembleList();
  for (QList<ZDvidSynapseEnsemble*>::const_iterator iter = synapseList.begin();
       iter != synapseList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    se->updatePartner(pos);
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
      se->updatePartner(*iter);
    }
    processObjectModified(se);
  }

  notifyObjectModified();
}

void ZFlyEmProofDoc::highlightPsd(bool on)
{
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  QList<ZDvidSynapseEnsemble*> synapseList = getDvidSynapseEnsembleList();
  for (QList<ZDvidSynapseEnsemble*>::const_iterator iter = synapseList.begin();
       iter != synapseList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    if (on) {
      se->addVisualEffect(NeuTube::Display::VE_GROUP_HIGHLIGHT);
    } else {
      se->removeVisualEffect(NeuTube::Display::VE_GROUP_HIGHLIGHT);
    }
    processObjectModified(se);
  }
  endObjectModifiedMode();

  notifyObjectModified();
}

void ZFlyEmProofDoc::verifySelectedSynapse()
{
  const std::string &userName = NeuTube::GetCurrentUserName();
  QList<ZDvidSynapseEnsemble*> synapseList = getDvidSynapseEnsembleList();
  ZDvidSynapseEnsemble::EDataScope scope = ZDvidSynapseEnsemble::DATA_GLOBAL;
  const std::set<ZIntPoint> &selected = getSelectedSynapse();
  for (QList<ZDvidSynapseEnsemble*>::const_iterator iter = synapseList.begin();
       iter != synapseList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    for (std::set<ZIntPoint>::const_iterator iter = selected.begin();
         iter != selected.end(); ++iter) {
      const ZIntPoint &pt = *iter;
      se->setUserName(pt, userName, scope);
      se->setConfidence(pt, 1.0, scope);
      emit synapseVerified(pt.getX(), pt.getY(), pt.getZ(), true);
    }
    scope = ZDvidSynapseEnsemble::DATA_LOCAL;
    processObjectModified(se);
  }
  notifyObjectModified();
}

void ZFlyEmProofDoc::unverifySelectedSynapse()
{
  const std::string &userName = NeuTube::GetCurrentUserName();
  QList<ZDvidSynapseEnsemble*> synapseList = getDvidSynapseEnsembleList();
  ZDvidSynapseEnsemble::EDataScope scope = ZDvidSynapseEnsemble::DATA_GLOBAL;
  const std::set<ZIntPoint> &selected = getSelectedSynapse();
  for (QList<ZDvidSynapseEnsemble*>::const_iterator iter = synapseList.begin();
       iter != synapseList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    for (std::set<ZIntPoint>::const_iterator iter = selected.begin();
         iter != selected.end(); ++iter) {
      const ZIntPoint &pt = *iter;
      se->setUserName(pt, "$" + userName, scope);
      se->setConfidence(pt, 0.5, scope);
      emit synapseVerified(pt.getX(), pt.getY(), pt.getZ(), false);
    }
    scope = ZDvidSynapseEnsemble::DATA_LOCAL;
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
    }
    scope = ZDvidSynapseEnsemble::DATA_LOCAL;
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
    slice->paintBuffer();
//    slice->clearSelection();
//    slice->updateLabelColor();
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
  ZOUT(LINFO(), 3) << "Checking splittable:" << bodyId;

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

/*
void ZFlyEmProofDoc::downloadBodyMask()
{
  if (getBodyForSplit() != NULL) {
    getBodyForSplit()->downloadBodyMask();
    notifyObjectModified();
  }
}
*/

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
  ZOUT(LTRACE(), 5) << "Update dvid label";
  TStackObjectList &objList = getObjectList(ZStackObject::TYPE_DVID_LABEL_SLICE);
  for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
       ++iter) {
    ZDvidLabelSlice *obj = dynamic_cast<ZDvidLabelSlice*>(*iter);
    obj->clearCache();
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
    std::string currentUserName = NeuTube::GetCurrentUserName();
    ZJsonArray bookmarkJson =
        m_dvidReader.readTaggedBookmark("user:" + currentUserName);
    beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
    int bookmarkCount = 0;
    for (size_t i = 0; i < bookmarkJson.size(); ++i) {
      ZFlyEmBookmark *bookmark = new ZFlyEmBookmark;
      ZJsonObject bookmarkObj = ZJsonObject(bookmarkJson.value(i));
      bookmark->loadDvidAnnotation(bookmarkObj);
      if (bookmark->getUserName().length() == (int) currentUserName.length()) {
        if (m_dvidReader.isBookmarkChecked(bookmark->getCenter().toIntPoint())) {
          bookmark->setChecked(true);
          ZDvidAnnotation::AddProperty(bookmarkObj, "checked", true);
          //        bookmarkObj.setProperty("checked", "1");
          m_dvidWriter.writeBookmark(bookmarkObj);
          m_dvidWriter.deleteBookmarkKey(*bookmark);
        }
        addObject(bookmark, true);
        ++bookmarkCount;
      } else {
        delete bookmark;
      }
    }
    endObjectModifiedMode();
    notifyObjectModified();

    if (bookmarkCount == 0) {
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

void ZFlyEmProofDoc::downloadTodo(int x, int y, int z)
{
  ZOUT(LTRACE(), 5) << "Download to do items";
  QList<ZFlyEmToDoList*> todoList = getObjectList<ZFlyEmToDoList>();
  for (QList<ZFlyEmToDoList*>::iterator iter = todoList.begin();
       iter != todoList.end(); ++iter) {
    ZFlyEmToDoList *td = *iter;
    td->update(x, y, z);
    processObjectModified(td);
  }

  notifyObjectModified();
}

void ZFlyEmProofDoc::downloadSynapse(int x, int y, int z)
{
  ZOUT(LTRACE(), 5) << "Download synapses";
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
ZFlyEmProofDoc::getSynapse(uint64_t bodyId) const
{
  QElapsedTimer timer;
  timer.start();

  std::pair<std::vector<ZPunctum*>, std::vector<ZPunctum*> > synapse;
  ZDvidReader reader;
//  reader.setVerbose(false);
  const double radius = 50.0;
  if (reader.open(getDvidTarget())) {
    std::vector<ZDvidSynapse> synapseArray =
        reader.readSynapse(bodyId, FlyEM::LOAD_PARTNER_RELJSON);

    std::vector<ZPunctum*> &tbar = synapse.first;
    std::vector<ZPunctum*> &psd = synapse.second;

    for (std::vector<ZDvidSynapse>::const_iterator iter = synapseArray.begin();
         iter != synapseArray.end(); ++iter) {
      const ZDvidSynapse &synapse = *iter;
      ZPunctum *punctum = new ZPunctum(synapse.getPosition(), radius);
#if defined(_FLYEM_)
      if (GET_FLYEM_CONFIG.anayzingMb6()) {
        punctum->setName(m_analyzer.getPunctumName(synapse));
      }
#endif
      if (synapse.getKind() == ZDvidSynapse::KIND_PRE_SYN) {
        tbar.push_back(punctum);
      } else if (synapse.getKind() == ZDvidSynapse::KIND_POST_SYN) {
        psd.push_back(punctum);
      }
    }
    ZOUT(LTRACE(), 5) << "Synapse loading time: " << timer.restart();
  }

  return synapse;
}

std::vector<ZFlyEmToDoItem*> ZFlyEmProofDoc::getTodoItem(uint64_t bodyId) const
{
  std::vector<ZFlyEmToDoItem*> puncta;
  ZDvidReader reader;
//  reader.setVerbose(false);
  if (reader.open(getDvidTarget())) {
    ZJsonArray annotationJson = reader.readAnnotation(
          getDvidTarget().getTodoListName(), bodyId);

    for (size_t i = 0; i < annotationJson.size(); ++i) {
      ZFlyEmToDoItem *item = new ZFlyEmToDoItem;

      ZJsonObject objJson(annotationJson.value(i));
      item->loadJsonObject(objJson, FlyEM::LOAD_NO_PARTNER);

      item->setBodyId(bodyId);

      puncta.push_back(item);
      /*
      if (item.isChecked()) {
        punctum->setColor()
      }
      */
    }
  }

  return puncta;
}


std::vector<ZPunctum*> ZFlyEmProofDoc::getTodoPuncta(uint64_t bodyId) const
{
  std::vector<ZPunctum*> puncta;
  ZDvidReader reader;
//  reader.setVerbose(false);
  const double radius = 50.0;
  if (reader.open(getDvidTarget())) {
    ZJsonArray annotationJson = reader.readAnnotation(
          getDvidTarget().getTodoListName(), bodyId);

    for (size_t i = 0; i < annotationJson.size(); ++i) {
      ZFlyEmToDoItem item;

      ZJsonObject objJson(annotationJson.value(i));
      item.loadJsonObject(objJson, FlyEM::LOAD_PARTNER_RELJSON);

      ZPunctum *punctum = new ZPunctum(item.getPosition(), radius);
      punctum->setColor(item.getDisplayColor());
      puncta.push_back(punctum);
      /*
      if (item.isChecked()) {
        punctum->setColor()
      }
      */
    }
  }

  return puncta;
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
  ZOUT(LTRACE(), 5) << "Find bookmark";
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

void ZFlyEmProofDoc::readBookmarkBodyId(QList<ZFlyEmBookmark *> &bookmarkArray)
{
  if (!bookmarkArray.isEmpty()) {
    std::vector<ZIntPoint> ptArray;
    for (QList<ZFlyEmBookmark*>::const_iterator iter = bookmarkArray.begin();
         iter != bookmarkArray.end(); ++iter) {
      const ZFlyEmBookmark *bookmark = *iter;
      ptArray.push_back(bookmark->getLocation());
    }

    std::vector<uint64_t> idArray = getDvidReader().readBodyIdAt(ptArray);
    if (bookmarkArray.size() == (int) idArray.size()) {
      for (int i = 0; i < bookmarkArray.size(); ++i) {
        ZFlyEmBookmark *bookmark = bookmarkArray[i];
        bookmark->setBodyId(idArray[i]);
      }
    }
  }
}

QList<ZFlyEmBookmark*> ZFlyEmProofDoc::importFlyEmBookmark(
    const std::string &filePath)
{
  ZOUT(LINFO(), 3) << "Importing flyem bookmarks";

  QList<ZFlyEmBookmark*> bookmarkList;

  m_loadingAssignedBookmark = true;

  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  if (!filePath.empty()) {
//    removeObject(ZStackObject::TYPE_FLYEM_BOOKMARK, true);
#if 1
    TStackObjectList objList = getObjectList(ZStackObject::TYPE_FLYEM_BOOKMARK);
    ZOUT(LINFO(), 3) << objList.size() << " bookmarks";
    std::vector<ZStackObject*> removed;

//    ZUndoCommand *command = new ZUndoCommand;

//    ZStackDocCommand::FlyEmBookmarkEdit::RemoveBookmark *removeCommand =
//        new ZStackDocCommand::FlyEmBookmarkEdit::RemoveBookmark(this, NULL, command);

    for (TStackObjectList::iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      ZStackObject *obj = *iter;
      ZFlyEmBookmark *bookmark = dynamic_cast<ZFlyEmBookmark*>(obj);
      if (bookmark != NULL) {
        if (!bookmark->isCustom()) {
          ZOUT(LTRACE(), 5) << "Removing bookmark: " << bookmark;
          removeObject(*iter, false);
          removed.push_back(*iter);
//          removeCommand->addRemoving(bookmark);
        }
      }
    }
#endif

//    ZStackDocCommand::FlyEmBookmarkEdit::AddBookmark *addCommand =
//        new ZStackDocCommand::FlyEmBookmarkEdit::AddBookmark(this, NULL, command);

    ZJsonObject obj;

    obj.load(filePath);

    ZJsonArray bookmarkArrayObj(obj["data"], ZJsonValue::SET_INCREASE_REF_COUNT);
    QList<ZFlyEmBookmark*> nullIdBookmarkList;
    for (size_t i = 0; i < bookmarkArrayObj.size(); ++i) {
      ZJsonObject bookmarkObj(bookmarkArrayObj.at(i),
                              ZJsonValue::SET_INCREASE_REF_COUNT);
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
          if (m_dvidReader.isBookmarkChecked(bookmark->getCenter().toIntPoint())) {
            bookmark->setChecked(true);
          }
          //            addCommand->addBookmark(bookmark);
          ZOUT(LTRACE(), 5) << "Adding bookmark: " << bookmark;
          bookmarkList.append(bookmark);
          if (bodyId <= 0) {
            nullIdBookmarkList.append(bookmark);
          }
          addObject(bookmark);
        }

      }
    }

    readBookmarkBodyId(nullIdBookmarkList);

//    pushUndoCommand(command);

    for (std::vector<ZStackObject*>::iterator iter = removed.begin();
         iter != removed.end(); ++iter) {
      ZOUT(LINFO(), 5) << "Deleting bookmark: " << *iter;
      delete *iter;
    }
  }
  endObjectModifiedMode();

  notifyObjectModified();

  m_loadingAssignedBookmark = false;

  ZOUT(LINFO(), 3) << "Bookmark imported";

  return bookmarkList;
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
    if (!m_loadingAssignedBookmark) {
      emit userBookmarkModified();
    }
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

void ZFlyEmProofDoc::notifyBodyLock(uint64_t bodyId, bool locking)
{
  emit requestingBodyLock(bodyId, locking);
}

void ZFlyEmProofDoc::refreshDvidLabelBuffer(unsigned long delay)
{
  if (delay > 0) {
    ZSleeper::msleep(delay);
  }
  QList<ZDvidLabelSlice*> sliceList = getDvidLabelSliceList();
  foreach (ZDvidLabelSlice *slice, sliceList) {
    if (!slice->refreshReaderBuffer()) {
      notify(ZWidgetMessage("Failed to refresh labels.", NeuTube::MSG_WARNING));
    }
  }
}

ZIntCuboidObj* ZFlyEmProofDoc::getSplitRoi() const
{
  return dynamic_cast<ZIntCuboidObj*>(
      getObjectGroup().findFirstSameSource(
        ZStackObject::TYPE_INT_CUBOID,
        ZStackObjectSourceFactory::MakeFlyEmSplitRoiSource()));
}

void ZFlyEmProofDoc::runSplit()
{
  QList<ZDocPlayer*> playerList =
      getPlayerList(ZStackObjectRole::ROLE_SEED);

  ZOUT(LINFO(), 3) << "Retrieving label set";

  QSet<int> labelSet;
  foreach (const ZDocPlayer *player, playerList) {
    labelSet.insert(player->getLabel());
  }

  if (labelSet.size() < 2) {
    ZWidgetMessage message(
          QString("The seed has no more than one label. No split is done"));
    message.setType(NeuTube::MSG_WARNING);

    emit messageGenerated(message);
    return;
  }

  const QString threadId = "seededWatershed";
  if (!m_futureMap.isAlive(threadId)) {
    m_futureMap.removeDeadThread();
    QFuture<void> future =
        QtConcurrent::run(this, &ZFlyEmProofDoc::runSplitFunc);
    m_futureMap[threadId] = future;
  }
}

void ZFlyEmProofDoc::runLocalSplit()
{
  QList<ZDocPlayer*> playerList =
      getPlayerList(ZStackObjectRole::ROLE_SEED);

  ZOUT(LINFO(), 3) << "Retrieving label set";

  QSet<int> labelSet;
  foreach (const ZDocPlayer *player, playerList) {
    labelSet.insert(player->getLabel());
  }

  if (labelSet.size() < 2) {
    ZWidgetMessage message(
          QString("The seed has no more than one label. No split is done"));
    message.setType(NeuTube::MSG_WARNING);

    emit messageGenerated(message);
    return;
  }

  const QString threadId = "seededWatershed";
  if (!m_futureMap.isAlive(threadId)) {
    m_futureMap.removeDeadThread();
    QFuture<void> future =
        QtConcurrent::run(this, &ZFlyEmProofDoc::localSplitFunc);
    m_futureMap[threadId] = future;
  }
}

void ZFlyEmProofDoc::runSplitFunc()
{
  getProgressSignal()->startProgress("Splitting ...");

  ZOUT(LINFO(), 3) << "Removing old result ...";
  removeObject(ZStackObjectRole::ROLE_TMP_RESULT, true);
//  m_isSegmentationReady = false;
  setSegmentationReady(false);

  getProgressSignal()->advanceProgress(0.1);
  //removeAllObj3d();
  ZStackWatershed engine;

  ZOUT(LINFO(), 3) << "Creating seed mask ...";
  ZStackArray seedMask = createWatershedMask(false);

  getProgressSignal()->advanceProgress(0.1);

  if (!seedMask.empty()) {
    ZStack *signalStack = getStack();
    ZIntPoint dsIntv(0, 0, 0);
    if (signalStack->isVirtual()) {
      signalStack = NULL;
      ZOUT(LINFO(), 3) << "Retrieving signal stack";
      ZIntCuboid cuboid = estimateSplitRoi();
      ZDvidSparseStack *sparseStack = getDvidSparseStack(cuboid);
      if (sparseStack != NULL) {
        signalStack = sparseStack->getStack();
        dsIntv = sparseStack->getDownsampleInterval();
      }
    }
    getProgressSignal()->advanceProgress(0.1);

    if (signalStack != NULL) {
      ZOUT(LINFO(), 3) << "Downsampling ..." << dsIntv.toString();
      seedMask.downsampleMax(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());

#ifdef _DEBUG_2
      seedMask[0]->save(GET_TEST_DATA_DIR + "/test.tif");
      signalStack->save(GET_TEST_DATA_DIR + "/test2.tif");
#endif

      ZStack *out = engine.run(signalStack, seedMask);
      out->setDsIntv(dsIntv);
      getProgressSignal()->advanceProgress(0.3);

      ZOUT(LINFO(), 3) << "Updating watershed boundary object";
      updateWatershedBoundaryObject(out, dsIntv);
      getProgressSignal()->advanceProgress(0.1);

//      notifyObj3dModified();

      ZOUT(LINFO(), 3) << "Setting label field";
      setLabelField(out);
//      m_isSegmentationReady = true;
      setSegmentationReady(true);

      emit messageGenerated(ZWidgetMessage(
            ZWidgetMessage::appendTime("Split done. Ready to upload.")));
    } else {
      std::cout << "No signal for watershed." << std::endl;
    }
  }
  getProgressSignal()->endProgress();
  emit labelFieldModified();
}

void ZFlyEmProofDoc::localSplitFunc()
{
  getProgressSignal()->startProgress("Splitting ...");

  ZOUT(LINFO(), 3) << "Removing old result ...";
  removeObject(ZStackObjectRole::ROLE_TMP_RESULT, true);
//  m_isSegmentationReady = false;
  setSegmentationReady(false);

  getProgressSignal()->advanceProgress(0.1);
  //removeAllObj3d();
  ZStackWatershed engine;

  ZOUT(LINFO(), 3) << "Creating seed mask ...";
  ZStackArray seedMask = createWatershedMask(false);

  getProgressSignal()->advanceProgress(0.1);

  if (!seedMask.empty()) {
    ZStack *signalStack = getStack();
    ZIntPoint dsIntv(0, 0, 0);
    if (signalStack->isVirtual()) {
      signalStack = NULL;
      ZOUT(LINFO(), 3) << "Retrieving signal stack";
      ZIntCuboid cuboid = estimateLocalSplitRoi();
      ZDvidSparseStack *sparseStack = getDvidSparseStack(cuboid);
      if (sparseStack != NULL) {
        signalStack = sparseStack->getStack();
        dsIntv = sparseStack->getDownsampleInterval();
      }
    }
    getProgressSignal()->advanceProgress(0.1);

    if (signalStack != NULL) {
      ZOUT(LINFO(), 3) << "Downsampling ..." << dsIntv.toString();
      seedMask.downsampleMax(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());

#ifdef _DEBUG_2
      seedMask[0]->save(GET_TEST_DATA_DIR + "/test.tif");
      signalStack->save(GET_TEST_DATA_DIR + "/test2.tif");
#endif

      ZStack *out = engine.run(signalStack, seedMask);
      getProgressSignal()->advanceProgress(0.3);

      ZOUT(LINFO(), 3) << "Updating watershed boundary object";
      updateWatershedBoundaryObject(out, dsIntv);
      getProgressSignal()->advanceProgress(0.1);

//      notifyObj3dModified();

      ZOUT(LINFO(), 3) << "Setting label field";
      setLabelField(out);
//      m_isSegmentationReady = true;
      setSegmentationReady(true);

      emit messageGenerated(ZWidgetMessage(
            ZWidgetMessage::appendTime("Split done. Ready to upload.")));
    } else {
      std::cout << "No signal for watershed." << std::endl;
    }
  }
  getProgressSignal()->endProgress();
  emit labelFieldModified();
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
  deprecateSplitSource();
//  m_splitSource.reset();

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

ZIntCuboid ZFlyEmProofDoc::estimateLocalSplitRoi()
{
  ZIntCuboid cuboid;

  ZStackArray seedMask = createWatershedMask(true);

  Cuboid_I box;
  seedMask.getBoundBox(&box);
  const int xMargin = 10;
  const int yMargin = 10;
  const int zMargin = 20;
  Cuboid_I_Expand_X(&box, xMargin);
  Cuboid_I_Expand_Y(&box, yMargin);
  Cuboid_I_Expand_Z(&box, zMargin);

  cuboid.set(box.cb[0], box.cb[1], box.cb[2], box.ce[0], box.ce[1],
      box.ce[2]);

  return cuboid;
}

ZIntCuboid ZFlyEmProofDoc::estimateSplitRoi()
{
  ZIntCuboid cuboid;

  ZDvidSparseStack *originalStack = ZStackDoc::getDvidSparseStack();
  if (originalStack != NULL) {
    ZIntCuboidObj *roi = getSplitRoi();
    if (roi == NULL) {
      if (originalStack->stackDownsampleRequired()) {
        ZStackArray seedMask = createWatershedMask(true);

        Cuboid_I box;
        seedMask.getBoundBox(&box);

        Cuboid_I_Expand_Z(&box, 10);

        int v = Cuboid_I_Volume(&box);

        double s = Cube_Root(ZSparseStack::GetMaxStackVolume() / 2 / v);
        if (s > 1) {
          int dw = iround(Cuboid_I_Width(&box) * s) - Cuboid_I_Width(&box);
          int dh = iround(Cuboid_I_Height(&box) * s) - Cuboid_I_Height(&box);
          int dd = iround(Cuboid_I_Depth(&box) * s) - Cuboid_I_Depth(&box);

          const int xMargin = dw / 2;
          const int yMargin = dh / 2;
          const int zMargin = dd / 2;
          Cuboid_I_Expand_X(&box, xMargin);
          Cuboid_I_Expand_Y(&box, yMargin);
          Cuboid_I_Expand_Z(&box, zMargin);
        }

        cuboid.set(box.cb[0], box.cb[1], box.cb[2], box.ce[0], box.ce[1],
            box.ce[2]);
      }
    } else {
      cuboid = roi->getCuboid();
    }
  }

  return cuboid;
}

ZDvidSparseStack* ZFlyEmProofDoc::getDvidSparseStack(const ZIntCuboid &roi) const
{
  ZDvidSparseStack *stack = NULL;

  ZDvidSparseStack *originalStack = ZStackDoc::getDvidSparseStack();
  if (originalStack != NULL) {
    if (!roi.isEmpty()) {
      if (m_splitSource.get() != NULL) {
        if (!roi.equals(m_splitSource->getBoundBox())) {
          m_splitSource.reset();
        }
      }

      if (m_splitSource.get() == NULL) {
        m_splitSource = ZSharedPointer<ZDvidSparseStack>(
              originalStack->getCrop(roi));

        originalStack->runFillValueFunc(roi, true);

        ZDvidInfo dvidInfo;
        dvidInfo.setFromJsonString(
              m_dvidReader.readInfo(getDvidTarget().getGrayScaleName().c_str()).
              toStdString());

        ZObject3dScan *objMask = m_splitSource->getObjectMask();
        ZObject3dScan blockObj = dvidInfo.getBlockIndex(*objMask);
        size_t stripeNumber = blockObj.getStripeNumber();

        for (size_t s = 0; s < stripeNumber; ++s) {
          const ZObject3dStripe &stripe = blockObj.getStripe(s);
          int segmentNumber = stripe.getSegmentNumber();
          int y = stripe.getY();
          int z = stripe.getZ();
          for (int i = 0; i < segmentNumber; ++i) {
            int x0 = stripe.getSegmentStart(i);
            int x1 = stripe.getSegmentEnd(i);

            ZIntPoint blockIndex =
                ZIntPoint(x0, y, z) - dvidInfo.getStartBlockIndex();
            for (int x = x0; x <= x1; ++x) {
              ZStack *stack =
                  originalStack->getStackGrid()->getStack(blockIndex);
              m_splitSource->getStackGrid()->consumeStack(
                    blockIndex, stack->clone());
              blockIndex.setX(blockIndex.getX() + 1);
            }
          }
        }
      }

      stack = m_splitSource.get();
    }
  }

  if (stack == NULL) {
    stack = originalStack;
  }

  return stack;
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

void ZFlyEmProofDoc::selectBodyInRoi(int z, bool appending, bool removingRoi)
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
    if (removingRoi) {
      executeRemoveRectRoiCommand();
//      removeRect2dRoi();
      /*
      ZStackObject *obj = getObjectGroup().findFirstSameSource(
            ZStackObject::TYPE_RECT2D,
            ZStackObjectSourceFactory::MakeRectRoiSource());
      executeRemoveObjectCommand(obj);
      */
    }
  }
}

void ZFlyEmProofDoc::rewriteSegmentation()
{
  ZIntCuboid box = getCuboidRoi();
  if (box.getDepth() > 1) {
    getDvidWriter().refreshLabel(
          box, getSelectedBodySet(NeuTube::BODY_LABEL_ORIGINAL));
    if (getDvidWriter().getStatusCode() != 200) {
      emit messageGenerated(
            ZWidgetMessage("Failed to rewite segmentations.", NeuTube::MSG_ERROR));
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

    if (!selectedPresyn.empty()) {
      if (!selectedPostsyn.empty()) {
        ZStackDocCommand::DvidSynapseEdit::UnlinkSynapse *command =
            new ZStackDocCommand::DvidSynapseEdit::UnlinkSynapse(this, selected);
        pushUndoCommand(command);
      } else {
        ZStackDocCommand::DvidSynapseEdit::UngroupSynapse *command =
            new ZStackDocCommand::DvidSynapseEdit::UngroupSynapse(this, NULL);
        for (std::vector<ZDvidSynapse>::const_iterator
             iter = selectedPresyn.begin(); iter != selectedPresyn.end();
             ++iter) {
          const ZDvidSynapse& presyn = *iter;
          command->addSynapse(presyn.getPosition());
        }
        pushUndoCommand(command);
      }
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
    } else {
      ZStackDocCommand::DvidSynapseEdit::GroupSynapse *groupCommand =
          new ZStackDocCommand::DvidSynapseEdit::GroupSynapse(
            this, command);
      for (std::vector<ZDvidSynapse>::const_iterator
           iter = selectedPresyn.begin(); iter != selectedPresyn.end();
           ++iter) {
        const ZDvidSynapse& presyn = *iter;
        groupCommand->addSynapse(presyn.getPosition());
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

void ZFlyEmProofDoc::repairSelectedSynapses()
{
  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(NeuTube::Z_AXIS);
  if (se != NULL) {
    const std::set<ZIntPoint> &selected =
        se->getSelector().getSelectedSet();
    for (std::set<ZIntPoint>::const_iterator iter = selected.begin();
         iter != selected.end(); ++iter) {
      repairSynapse(*iter);
    }
  }
}

void ZFlyEmProofDoc::executeRemoveSynapseCommand()
{
//  QUndoCommand *command =
//      new ZStackDocCommand::DvidSynapseEdit::CompositeCommand(this);


  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(NeuTube::Z_AXIS);
  if (se != NULL) {
    const std::set<ZIntPoint> &selected =
        se->getSelector().getSelectedSet();
    if (!selected.empty()) {
      ZStackDocCommand::DvidSynapseEdit::RemoveSynapseOp *command =
          new ZStackDocCommand::DvidSynapseEdit::RemoveSynapseOp(this);
      for (std::set<ZIntPoint>::const_iterator iter = selected.begin();
           iter != selected.end(); ++iter) {
        command->addRemoval(*iter);
      }
      pushUndoCommand(command);
    }

#if 0
    for (std::set<ZIntPoint>::const_iterator iter = selected.begin();
         iter != selected.end(); ++iter) {
      const ZIntPoint &pt = *iter;
      ZDvidSynapse &synapse =
          se->getSynapse(pt, ZDvidSynapseEnsemble::DATA_GLOBAL);
      removingSet.insert(pt);

      if (synapse.getKind() == ZDvidSynapse::KIND_PRE_SYN) {
        std::vector<ZIntPoint> partners = synapse.getPartners();
        removingSet.insert(partners.begin(), partners.end());
      }
    }

    ZStackDocCommand::DvidSynapseEdit::RemoveSynapses *command =
        new ZStackDocCommand::DvidSynapseEdit::RemoveSynapses(this);
    command->setRemoval(removingSet);

    pushUndoCommand(command);
#endif

#if 0
    for (std::set<ZIntPoint>::const_iterator iter = removingSet.begin();
         iter != removingSet.end(); ++iter) {
      const ZIntPoint &pt = *iter;
      new ZStackDocCommand::DvidSynapseEdit::RemoveSynapse(
            this, pt.getX(), pt.getY(), pt.getZ(), command);
    }
    se->getSelector().deselectAll();

    if (command->childCount() > 0) {
      pushUndoCommand(command);
    }
#endif
  }
}

void ZFlyEmProofDoc::executeAddSynapseCommand(
    const ZDvidSynapse &synapse, bool tryingLink)
{
  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(NeuTube::Z_AXIS);
  if (se != NULL) {
    ZUndoCommand *command =
        new ZStackDocCommand::DvidSynapseEdit::CompositeCommand(this);
    new ZStackDocCommand::DvidSynapseEdit::AddSynapse(
          this, synapse, command);
    if (tryingLink) {
      if (synapse.getKind() == ZDvidAnnotation::KIND_POST_SYN) {
        const std::set<ZIntPoint> &selected =
            se->getSelector().getSelectedSet();
        std::vector<ZDvidSynapse> selectedPresyn;
        for (std::set<ZIntPoint>::const_iterator iter = selected.begin();
             iter != selected.end(); ++iter) {
          ZDvidSynapse &synapse =
              se->getSynapse(*iter, ZDvidSynapseEnsemble::DATA_GLOBAL);
          if (synapse.getKind() == ZDvidSynapse::KIND_PRE_SYN) {
            selectedPresyn.push_back(synapse);
          }
          if (selectedPresyn.size() > 1) {
            break;
          }
        }
        if (selectedPresyn.size() == 1) {
          ZDvidSynapse &presyn = selectedPresyn.front();
          new ZStackDocCommand::DvidSynapseEdit::LinkSynapse(
                this, presyn.getPosition(), synapse.getPosition(),
                ZDvidSynapse::Relation::GetName(
                  ZDvidSynapse::Relation::RELATION_PRESYN_TO), command);
          new ZStackDocCommand::DvidSynapseEdit::LinkSynapse(
                this, synapse.getPosition(), presyn.getPosition(),
                ZDvidSynapse::Relation::GetName(
                  ZDvidSynapse::Relation::RELATION_POSTSYN_TO), command);
        }
      }
    }
    pushUndoCommand(command);
  } else {
    emit messageGenerated(
          ZWidgetMessage(
            "Failed to add synapse. Have you specified the synapse data name?",
            NeuTube::MSG_WARNING));
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

void ZFlyEmProofDoc::executeAddTodoItemCommand(
    int x, int y, int z, bool checked)
{
  executeAddTodoItemCommand(ZIntPoint(x, y, z), checked);
}

void ZFlyEmProofDoc::executeAddTodoItemCommand(const ZIntPoint &pt, bool checked)
{
  ZFlyEmToDoItem item(pt);
  item.setUserName(NeuTube::GetCurrentUserName());
  if (checked) {
    item.setChecked(checked);
  }

  executeAddTodoItemCommand(item);
}

void ZFlyEmProofDoc::executeAddTodoItemCommand(ZFlyEmToDoItem &item)
{
  QUndoCommand *command =
      new ZStackDocCommand::FlyEmToDoItemEdit::AddItem(this, item);
  pushUndoCommand(command);
}

std::set<ZIntPoint> ZFlyEmProofDoc::getSelectedTodoItemPosition() const
{
  std::set<ZIntPoint> selected;
  ZOUT(LTRACE(), 5) << "Get selected todo positions";
  QList<ZFlyEmToDoList*> objList = getObjectList<ZFlyEmToDoList>();
  for (QList<ZFlyEmToDoList*>::const_iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    const ZFlyEmToDoList *td = *iter;
    const std::set<ZIntPoint> &subset = td->getSelector().getSelectedSet();
    selected.insert(subset.begin(), subset.end());
  }

  return selected;
}


void ZFlyEmProofDoc::executeRemoveTodoItemCommand()
{
  std::set<ZIntPoint> selected = getSelectedTodoItemPosition();

  if (!selected.empty()) {
    QUndoCommand *command = new QUndoCommand;
    for (std::set<ZIntPoint>::const_iterator iter = selected.begin();
         iter != selected.end(); ++iter) {
      const ZIntPoint &pt = *iter;
      new ZStackDocCommand::FlyEmToDoItemEdit::RemoveItem(
            this, pt.getX(), pt.getY(), pt.getZ(), command);
    }
    beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
    pushUndoCommand(command);
    endObjectModifiedMode();
    notifyObjectModified();
  }
}

void ZFlyEmProofDoc::copyBookmarkFrom(const ZFlyEmProofDoc *doc)
{
  ZOUT(LTRACE(), 5) << "Copy bookmarks";
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

void ZFlyEmProofDoc::notifySynapseMoved(
    const ZIntPoint &from, const ZIntPoint &to)
{
  emit synapseMoved(from, to);
}

void ZFlyEmProofDoc::notifyTodoEdited(const ZIntPoint &item)
{
  emit todoEdited(item.getX(), item.getY(), item.getZ());
}

void ZFlyEmProofDoc::notifyAssignedBookmarkModified()
{
  emit assignedBookmarkModified();
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
  ZOUT(LTRACE(), 5) << "Get bookmarks";
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


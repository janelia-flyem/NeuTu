#include "zflyemproofdoc.h"

#include <QSet>
#include <QList>
#include <QTimer>
#include <QDir>
#include <QtConcurrentRun>
#include <QMessageBox>
#include <QElapsedTimer>
#include <sstream>

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
#include "dvid/zdvidgrayslice.h"
#include "flyem/zflyemproofdoccommand.h"
#include "dialogs/zflyemsynapseannotationdialog.h"
#include "zprogresssignal.h"
#include "imgproc/zstackwatershed.h"
#include "zstackarray.h"
#include "zsleeper.h"
#include "zdvidutil.h"
#include "zstackdocdatabuffer.h"
#include "flyem/zserviceconsumer.h"
#include "zstroke2d.h"
#include "flyem/zflyemmisc.h"
#include "zstackwatershedcontainer.h"
#include "zmeshfactory.h"
#include "zswctree.h"
#include "zflyemroutinechecktask.h"
#include "dvid/zdviddataslicehelper.h"
#include "zarray.h"
#include "zflyembodymanager.h"

const char* ZFlyEmProofDoc::THREAD_SPLIT = "seededWatershed";

ZFlyEmProofDoc::ZFlyEmProofDoc(QObject *parent) :
  ZStackDoc(parent)
{
  init();
}

ZFlyEmProofDoc::~ZFlyEmProofDoc()
{
  endWorkThread();
  LDEBUG() << "ZFlyEmProofDoc destroyed";
}

void ZFlyEmProofDoc::init()
{
  setTag(neutube::Document::FLYEM_PROOFREAD);

  m_loadingAssignedBookmark = false;
  m_analyzer.setDvidReader(&m_synapseReader);
  m_supervisor = new ZFlyEmSupervisor(this);
  m_mergeProject = new ZFlyEmBodyMergeProject(this);

  m_routineCheck = false;

  initTimer();
  initAutoSave();

  connectSignalSlot();

  startWorkThread();
}

void ZFlyEmProofDoc::initTimer()
{
//  m_bookmarkTimer = new QTimer(this);
//  m_bookmarkTimer->setInterval(60000);
//  m_bookmarkTimer->start();

  m_routineTimer = new QTimer(this);
  m_routineTimer->setInterval(10000);
}

void ZFlyEmProofDoc::startTimer()
{
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
                           neutube::MSG_ERROR));
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
  connect(getMergeProject(), SIGNAL(mergeUploaded()),
          this, SIGNAL(bodyMergeUploaded()));
  connect(this, SIGNAL(objectSelectorChanged(ZStackObjectSelector)),
          getMergeProject(), SIGNAL(selectionChanged(ZStackObjectSelector)));

  m_mergeProject->getProgressSignal()->connectProgress(getProgressSignal());

  //    connect(getMergeProject(), SIGNAL(dvidLabelChanged()),
  //            this, SLOT(updateDvidLabelObject()));
  connect(getMergeProject(), SIGNAL(checkingInBody(uint64_t, flyem::EBodySplitMode)),
          this, SLOT(checkInBodyWithMessage(uint64_t, flyem::EBodySplitMode)));
  connect(getMergeProject(), SIGNAL(dvidLabelChanged()),
          this, SLOT(updateDvidLabelObjectSliently()));

  ZWidgetMessage::ConnectMessagePipe(getMergeProject(), this, false);

  connect(m_routineTimer, SIGNAL(timeout()), this, SLOT(scheduleRoutineCheck()));

  connect(this, SIGNAL(updatingLabelSlice(ZArray*,ZStackViewParam,int,int,int,bool)),
          this, SLOT(updateLabelSlice(ZArray*,ZStackViewParam,int,int,int,bool)),
          Qt::QueuedConnection);

  connect(this, SIGNAL(updatingGraySlice(ZStack*,ZStackViewParam,int,int,int,bool)),
          this, SLOT(updateGraySlice(ZStack*,ZStackViewParam,int,int,int,bool)),
          Qt::QueuedConnection);
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

void ZFlyEmProofDoc::syncMergeWithDvid()
{
  getMergeProject()->syncWithDvid();
}

/*
void ZFlyEmProofDoc::uploadMergeResult()
{
  getMergeProject()->uploadResult();
}
*/

void ZFlyEmProofDoc::scheduleRoutineCheck()
{
  if (m_routineCheck) {
    ZFlyEmRoutineCheckTask *task = new ZFlyEmRoutineCheckTask;
    task->setDoc(this);
    addTask(task);
  }
}

void ZFlyEmProofDoc::runRoutineCheck()
{
  if (m_routineCheck) {
    if (NeutubeConfig::GetVerboseLevel() >= 5) {
      if (getSupervisor() != NULL) {
        if (!getSupervisor()->isEmpty()) {
          QElapsedTimer timer;
          timer.start();
          int statusCode = getSupervisor()->testServer();
          if (statusCode == 200) {
            ZOUT(LTRACE(), 5) << "HEAD time:"
                              << getSupervisor()->getMainUrl() + ":"
                              << timer.elapsed() << "ms";
          } else {
            LWARN() << "API load failed:" << getSupervisor()->getMainUrl();
          }
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

QString ZFlyEmProofDoc::getBodySelectionMessage() const
{
  QString msg;

  const std::set<uint64_t> &selected =
      getSelectedBodySet(neutube::BODY_LABEL_MAPPED);

  for (std::set<uint64_t>::const_iterator iter = selected.begin();
       iter != selected.end(); ++iter) {
    uint64_t bodyId = *iter;
    msg += QString("%1 ").arg(bodyId);
    const QSet<uint64_t> &originalBodySet =
        getBodyMerger()->getOriginalLabelSet(bodyId);
    if (originalBodySet.size() > 1) {
      msg += "<font color=#888888>(";
      for (QSet<uint64_t>::const_iterator iter = originalBodySet.begin();
           iter != originalBodySet.end(); ++iter) {
        if (selected.count(*iter) == 0) {
          msg += QString("_%1").arg(*iter);
        }
      }
      msg += ")</font> ";
    }
  }

  if (msg.isEmpty()) {
    msg = "No body selected.";
  } else {
    msg += " selected.";
  }

  return msg;
}

void ZFlyEmProofDoc::addSelectedBody(
    const std::set<uint64_t> &selected, neutube::EBodyLabelType labelType)
{
  std::set<uint64_t> currentSelected = getSelectedBodySet(labelType);
  currentSelected.insert(selected.begin(), selected.end());
  setSelectedBody(currentSelected, labelType);
}

void ZFlyEmProofDoc::setSelectedBody(
    const std::set<uint64_t> &selected, neutube::EBodyLabelType labelType)
{
  std::set<uint64_t> currentSelected = getSelectedBodySet(labelType);

  if (currentSelected != selected) {
    QList<ZDvidLabelSlice*> sliceList = getDvidLabelSliceList();
    if (!sliceList.isEmpty()) {
      for (QList<ZDvidLabelSlice*>::iterator iter = sliceList.begin();
           iter != sliceList.end(); ++iter) {
        ZDvidLabelSlice *slice = *iter;
        slice->recordSelection();
        slice->setSelection(selected, labelType);
        slice->processSelection();
      }

      notifyBodySelectionChanged();
    }
  }
}

void ZFlyEmProofDoc::setSelectedBody(
    uint64_t bodyId, neutube::EBodyLabelType labelType)
{
  std::set<uint64_t> selected;
  selected.insert(bodyId);
  setSelectedBody(selected, labelType);
}

void ZFlyEmProofDoc::toggleBodySelection(
    uint64_t bodyId, neutube::EBodyLabelType labelType)
{
  std::set<uint64_t> currentSelected = getSelectedBodySet(labelType);
  if (currentSelected.count(bodyId) > 0) {
    currentSelected.erase(bodyId);
  } else {
    currentSelected.insert(bodyId);
  }
  setSelectedBody(currentSelected, labelType);
}

void ZFlyEmProofDoc::deselectMappedBody(
    uint64_t bodyId, neutube::EBodyLabelType labelType)
{
  std::set<uint64_t> currentSelected =
      getSelectedBodySet(neutube::BODY_LABEL_ORIGINAL);
  std::set<uint64_t> newSelected;
  uint64_t mappedBodyId = bodyId;
  if (labelType == neutube::BODY_LABEL_ORIGINAL) {
    mappedBodyId = getBodyMerger()->getFinalLabel(bodyId);
  }
  for (std::set<uint64_t>::const_iterator iter = currentSelected.begin();
       iter != currentSelected.end(); ++iter) {
    if (getBodyMerger()->getFinalLabel(*iter) != mappedBodyId) {
      newSelected.insert(*iter);
    }
  }

  setSelectedBody(newSelected, neutube::BODY_LABEL_ORIGINAL);
}

void ZFlyEmProofDoc::deselectMappedBody(
    const std::set<uint64_t> &bodySet, neutube::EBodyLabelType labelType)
{
  std::set<uint64_t> currentSelected =
      getSelectedBodySet(neutube::BODY_LABEL_ORIGINAL);
  std::set<uint64_t> newSelected;
  std::set<uint64_t> mappedBodySet = bodySet;
  if (labelType == neutube::BODY_LABEL_ORIGINAL) {
    mappedBodySet = getBodyMerger()->getFinalLabel(bodySet);
  }
  for (std::set<uint64_t>::const_iterator iter = currentSelected.begin();
       iter != currentSelected.end(); ++iter) {
    if (mappedBodySet.count(getBodyMerger()->getFinalLabel(*iter)) == 0) {
      newSelected.insert(*iter);
    }
  }

  setSelectedBody(newSelected, neutube::BODY_LABEL_ORIGINAL);
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
    neutube::EBodyLabelType labelType) const
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
  case neutube::BODY_LABEL_ORIGINAL:
    break;
  case neutube::BODY_LABEL_MAPPED:
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
  std::set<uint64_t> selected = getSelectedBodySet(neutube::BODY_LABEL_ORIGINAL);
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
  std::set<uint64_t> selected = getSelectedBodySet(neutube::BODY_LABEL_ORIGINAL);
  for (QMap<uint64_t, ZFlyEmBodyAnnotation>::const_iterator
       iter = m_annotationMap.begin(); iter != m_annotationMap.end(); ++iter) {
    uint64_t bodyId = iter.key();
    if (selected.count(bodyId) == 0) {
      emit messageGenerated(
            ZWidgetMessage(
              QString("Inconsistent body selection: %1").arg(bodyId),
              neutube::MSG_WARNING));
    }
  }
}

void ZFlyEmProofDoc::clearBodyMergeStage()
{
  clearBodyMerger();
//  saveMergeOperation();
  notifyBodyUnmerged();
}

void ZFlyEmProofDoc::unmergeSelected()
{
  ZFlyEmProofDocCommand::UnmergeBody *command =
      new ZFlyEmProofDocCommand::UnmergeBody(this);
  pushUndoCommand(command);
}

void ZFlyEmProofDoc::mergeSelectedWithoutConflict(ZFlyEmSupervisor *supervisor)
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
    okToContinue = false;
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
            if (supervisor->checkOut(*iter, flyem::BODY_SPLIT_NONE)) {
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
                        neutube::MSG_ERROR));
              } else {
                emit messageGenerated(
                      ZWidgetMessage(
                        QString("Failed to merge. %1 has been locked by %2").
                        arg(*iter).arg(owner.c_str()), neutube::MSG_ERROR));
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

QString ZFlyEmProofDoc::getAnnotationNameWarningDetail(
    const QMap<uint64_t, QVector<QString> > &nameMap) const
{
  QString detail;

  if (nameMap.size() > 1) {
    detail = "<p>Details: </p>";
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
  }

  return detail;
}

QString ZFlyEmProofDoc::getAnnotationFinalizedWarningDetail(
    const std::vector<uint64_t> &finalizedBodyArray) const
{
  QString detail;

  if (!finalizedBodyArray.empty()) {
    detail = "<p>Finalized: </p>";
    detail += "<ul>";
    for (std::vector<uint64_t>::const_iterator iter = finalizedBodyArray.begin();
         iter != finalizedBodyArray.end(); ++iter) {
      uint64_t bodyId = *iter;
      detail += QString("<li>%1</li>").arg(bodyId);
    }
    detail += "</ul>";
  }

  return detail;
}

void ZFlyEmProofDoc::mergeSelected(ZFlyEmSupervisor *supervisor)
{
  bool okToContinue = true;

  cleanBodyAnnotationMap();

  QMap<uint64_t, QVector<QString> > nameMap;
  std::vector<uint64_t> finalizedBodyArray;
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
    if (anno.getStatus() == "Finalized") {
      finalizedBodyArray.push_back(iter.key());
    }
  }

  if (!finalizedBodyArray.empty()) {
    QString detail = getAnnotationFinalizedWarningDetail(finalizedBodyArray);
    okToContinue = ZDialogFactory::Ask(
          "Merging Finalized Body",
          "At least one of the bodies to be merged is finalized. Do you want to continue?" +
          detail,
          NULL);
  }

  if (okToContinue) {
    if (nameMap.size() > 1) {
      QString detail = getAnnotationNameWarningDetail(nameMap);
      okToContinue = ZDialogFactory::Ask(
            "Conflict to Resolve",
            "You are about to merge multiple names. Do you want to continue?" +
            detail,
            NULL);
    }
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
            if (supervisor->checkOut(*iter, flyem::BODY_SPLIT_NONE)) {
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
                        neutube::MSG_ERROR));
              } else {
                emit messageGenerated(
                      ZWidgetMessage(
                        QString("Failed to merge. %1 has been locked by %2").
                        arg(*iter).arg(owner.c_str()), neutube::MSG_ERROR));
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
  ZDvidWriter &writer = m_dvidWriter;
  if (writer.good()) {
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

    processObjectModified();
  }
  if (writer.getStatusCode() == 200) {
    if (getSelectedBodySet(neutube::BODY_LABEL_ORIGINAL).count(bodyId) > 0) {
      m_annotationMap[bodyId] = annotation;
    }
    emit messageGenerated(
          ZWidgetMessage(QString("Body %1 is annotated.").arg(bodyId)));
  } else {
    ZOUT(LTRACE(), 5) << writer.getStandardOutput();
    emit messageGenerated(
          ZWidgetMessage("Cannot save annotation.", neutube::MSG_ERROR));
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
                             neutube::MSG_ERROR));
      }
    }
  }
}

void ZFlyEmProofDoc::initData(const ZDvidTarget &target)
{
  if (m_dvidReader.isReady() && !getDvidTarget().readOnly()) {
    initData("annotation", target.getBookmarkName());
    initData("annotation", target.getTodoListName());
    initData("keyvalue", target.getSkeletonName());
    initData("keyvalue", target.getThumbnailName());
    initData("keyvalue", target.getBookmarkKeyName());
    initData("keyvalue", target.getBodyAnnotationName());
    initData("keyvalue", target.getSplitLabelName());
    initData("keyvalue", ZDvidData::GetName(ZDvidData::ROLE_MERGE_OPERATION));
  }
}

const ZDvidTarget& ZFlyEmProofDoc::getDvidTarget() const
{
  return m_dvidReader.getDvidTarget();
}

void ZFlyEmProofDoc::setDvidTarget(const ZDvidTarget &target)
{
  LINFO() << "Setting dvid env in ZFlyEmProofDoc";
  if (m_dvidReader.open(target)) {
    std::ostringstream flowInfo;

    flowInfo << "Prepare DVID readers";
    m_dvidWriter.open(target);
    m_synapseReader.openRaw(m_dvidReader.getDvidTarget());
    m_todoReader.openRaw(m_dvidReader.getDvidTarget());
    m_sparseVolReader.openRaw(m_dvidReader.getDvidTarget());
    m_grayscaleReader.openRaw(m_dvidReader.getDvidTarget().getGrayScaleTarget());
//    m_dvidTarget = target;
    m_activeBodyColorMap.reset();
    m_mergeProject->setDvidTarget(m_dvidReader.getDvidTarget());

    flowInfo << "->Prepare DVID data instances";
    initData(getDvidTarget());
    if (getSupervisor() != NULL) {
      flowInfo << "->Prepare librarian";
      getSupervisor()->setDvidTarget(m_dvidReader.getDvidTarget());
      if (!getSupervisor()->isEmpty() && !target.readOnly()) {
        int statusCode = getSupervisor()->testServer();
        if (statusCode != 200) {
          emit messageGenerated(
                ZWidgetMessage(
                  QString("WARNING: Failed to connect to the librarian %1. "
                          "Please do NOT proofread segmentation "
                          "until you fix the problem.").
                  arg(getSupervisor()->getMainUrl().c_str()),
                  neutube::MSG_WARNING));
        }
      }
    }

    flowInfo << "->Read DVID Info";
    readInfo();

    flowInfo << "->Prepare DVID-related data";
    prepareDvidData();

    flowInfo << "->Update DVID target for objects";
    updateDvidTargetForObject();

    flowInfo << "->Update DVID info for objects";
    updateDvidInfoForObject();

#ifdef _DEBUG_2
//    m_dvidReader.getDvidTarget().setReadOnly(true);
    const_cast<ZDvidTarget&>(target).setReadOnly(true);
#endif

    //Run check anyway to get around a strange bug of showing grayscale
    flowInfo << "->Check proofreading data instances";
    int missing = m_dvidReader.checkProofreadingData();
    if (!getDvidTarget().readOnly()) {
      if (missing > 0) {
        emit messageGenerated(
              ZWidgetMessage(
                QString("WARNING: Some data for proofreading are missing in "
                        "the database. "
                        "Please do NOT proofread segmentation "
                        "until you fix the problem."),
                neutube::MSG_WARNING));
      }
    }

    LDEBUG() << flowInfo.str();
    startTimer();
  } else {
    m_dvidReader.clear();
//    m_dvidTarget.clear();
    ZWidgetMessage msg("Failed to open the node.", neutube::MSG_ERROR);
    QString detail = "Detail: ";
    if (!m_dvidReader.getErrorMsg().empty()) {
      detail += m_dvidReader.getErrorMsg().c_str();
    }
    msg.appendMessage(detail);
    emit messageGenerated(msg);
  }
}

bool ZFlyEmProofDoc::isDataValid(const std::string &data) const
{
  return ZDvid::IsDataValid(data, getDvidTarget(), m_infoJson, m_versionDag);
}

namespace {
  static bool emitBodySelectionMessage = true;
}

void ZFlyEmProofDoc::enableBodySelectionMessage(bool enable)
{
  emitBodySelectionMessage = enable;
}

bool ZFlyEmProofDoc::bodySelectionMessageEnabled()
{
  return emitBodySelectionMessage;
}

void ZFlyEmProofDoc::notifyBodySelectionChanged()
{
  if (emitBodySelectionMessage) {
    emit messageGenerated(ZWidgetMessage(getBodySelectionMessage()));
  }
  emit bodySelectionChanged();
}

void ZFlyEmProofDoc::updateMaxLabelZoom()
{
  m_dvidReader.updateMaxLabelZoom();
//  m_dvidReader.updateMaxLabelZoom(m_infoJson, m_versionDag);
}

void ZFlyEmProofDoc::updateMaxGrayscaleZoom()
{
  m_grayscaleReader.updateMaxGrayscaleZoom();
//  m_dvidReader.updateMaxGrayscaleZoom(m_infoJson, m_versionDag);
}

void ZFlyEmProofDoc::readInfo()
{
  m_grayScaleInfo = m_grayscaleReader.readGrayScaleInfo();
  m_labelInfo = m_dvidReader.readLabelInfo();
  m_versionDag = m_dvidReader.readVersionDag();

#ifdef _DEBUG_2
  std::cout << "Label Info:" << std::endl;
  m_labelInfo.print();
#endif

#ifdef _DEBUG_2
  m_versionDag.print();
#endif

  m_infoJson = m_dvidReader.readInfo();

  std::string startLog = "Start using UUID " +
      m_dvidReader.getDvidTarget().getUuid() + "@" +
      m_dvidReader.getDvidTarget().getAddressWithPort();

  if (m_infoJson.hasKey("Alias")) {
    startLog += std::string("; Alias: ") +
        ZJsonParser::stringValue(m_infoJson["Alias"]);
  }
  if (m_infoJson.hasKey("Description")) {
    startLog += std::string("; Description: ") +
        ZJsonParser::stringValue(m_infoJson["Description"]);
  }

  updateMaxLabelZoom();
  updateMaxGrayscaleZoom();

  LINFO() << startLog;
}

void ZFlyEmProofDoc::loadRoiFunc()
{
  if (!getDvidTarget().getRoiName().empty()) {
    if (!m_roiReader.isReady()) {
      m_roiReader.open(m_dvidReader.getDvidTarget());
    }
    ZObject3dScan *obj =
        m_roiReader.readRoi(getDvidTarget().getRoiName(), (ZObject3dScan*) NULL);
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
        obj->useCosmeticPen(true);
        //          obj->setDsIntv(31, 31, 31);
        obj->addVisualEffect(neutube::display::SparseObject::VE_PLANE_BOUNDARY);
//        obj->setHittable(false);
        obj->setHitProtocal(ZStackObject::HIT_NONE);
        //      addObject(obj);
        m_dataBuffer->addUpdate(obj, ZStackDocObjectUpdate::ACTION_ADD_UNIQUE);
        m_dataBuffer->deliver();
        //          obj->setTarget(ZStackObject::TARGET_TILE_CANVAS);
      } else {
        delete obj;
      }
    }
  }
}

ZDvidGraySlice* ZFlyEmProofDoc::getDvidGraySlice() const
{
  return getDvidGraySlice(neutube::Z_AXIS);
}

ZDvidGraySlice* ZFlyEmProofDoc::getDvidGraySlice(neutube::EAxis axis) const
{
  ZStackObject *obj = getObject(ZStackObject::TYPE_DVID_GRAY_SLICE,
            ZStackObjectSourceFactory::MakeDvidGraySliceSource(axis));

  return dynamic_cast<ZDvidGraySlice*>(obj);
}


const ZDvidInfo& ZFlyEmProofDoc::getDvidInfo() const
{
  if (getDvidTarget().hasGrayScaleData()) {
    return m_grayScaleInfo;
  }

  return m_labelInfo;
}

void ZFlyEmProofDoc::prepareDvidData()
{
  if (m_dvidReader.isReady()) {
    ZDvidInfo dvidInfo = getDvidInfo();

    ZIntCuboid boundBox;
    if (dvidInfo.isValid()) {
      boundBox = ZIntCuboid(dvidInfo.getStartCoordinates(),
                       dvidInfo.getEndCoordinates());
    } else {
      boundBox = ZIntCuboid(ZIntPoint(0, 0, 0), ZIntPoint(512, 512, 512));
    }

    ZStack *stack = ZStackFactory::MakeVirtualStack(boundBox);
    loadStack(stack);

    //Download ROI
    m_futureMap["loadRoiFunc"] =
        QtConcurrent::run(this, &ZFlyEmProofDoc::loadRoiFunc);
  }


  if (getDvidTarget().hasTileData()) {
    initTileData();
  } else {
    initGrayscaleSlice();
  }

  addDvidLabelSlice(neutube::Z_AXIS);
}

void ZFlyEmProofDoc::initTileData()
{
  ZDvidTileEnsemble *ensemble = new ZDvidTileEnsemble;
  ensemble->addRole(ZStackObjectRole::ROLE_ACTIVE_VIEW);
  ensemble->setSource(ZStackObjectSourceFactory::MakeDvidTileSource());
  ensemble->setDvidTarget(getDvidTarget());
  addObject(ensemble, true);
}

void ZFlyEmProofDoc::initGrayscaleSlice()
{
  if (getDvidTarget().hasGrayScaleData()) {
    ZDvidGraySlice *slice = new ZDvidGraySlice;
    slice->addRole(ZStackObjectRole::ROLE_ACTIVE_VIEW);
    slice->setSource(
          ZStackObjectSourceFactory::MakeDvidGraySliceSource(neutube::Z_AXIS));
    slice->setDvidTarget(m_grayscaleReader.getDvidTarget());
    prepareGraySlice(slice);
    addObject(slice, true);
  }
}

void ZFlyEmProofDoc::setGraySliceCenterCut(int width, int height)
{
  m_graySliceCenterCutWidth = width;
  m_graySliceCenterCutHeight = height;
  prepareGraySlice(getDvidGraySlice());
}

void ZFlyEmProofDoc::setSegmentationCenterCut(int width, int height)
{
  m_labelSliceCenterCutWidth = width;
  m_labelSliceCenterCutHeight = height;
  prepareLabelSlice();
}

void ZFlyEmProofDoc::addDvidLabelSlice(neutube::EAxis axis)
{
  ZDvidLabelSlice *labelSlice = new ZDvidLabelSlice;
  labelSlice->setSliceAxis(axis);
  labelSlice->setRole(ZStackObjectRole::ROLE_ACTIVE_VIEW);
  labelSlice->setDvidTarget(getDvidTarget());

  LINFO() << "Max label zoom:" << getDvidTarget().getMaxLabelZoom();

  labelSlice->setSource(
        ZStackObjectSourceFactory::MakeDvidLabelSliceSource(axis));
  labelSlice->setBodyMerger(&m_bodyMerger);
  labelSlice->setCenterCut(
        m_labelSliceCenterCutWidth, m_labelSliceCenterCutHeight);
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
static void UpdateDvidInfoForObject(ZFlyEmProofDoc *doc)
{
  ZOUT(LTRACE(), 5) << "Update dvid target";
  QList<T*> objList = doc->getObjectList<T>();
  for (typename QList<T*>::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    T *obj = *iter;
    obj->setDvidInfo(doc->getDvidInfo());
//    doc->processObjectModified(obj);
  }
}

void ZFlyEmProofDoc::updateDvidInfoForObject()
{
  UpdateDvidInfoForObject<ZDvidSynapseEnsemble>(this);
  UpdateDvidInfoForObject<ZFlyEmToDoList>(this);
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
//  processObjectModified();
}


ZDvidLabelSlice* ZFlyEmProofDoc::getDvidLabelSlice(neutube::EAxis axis) const
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
    neutube::EAxis axis) const
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
  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(neutube::Z_AXIS);
  if (se != NULL) {
    return getDvidSynapseEnsemble(neutube::Z_AXIS)->hasSelected();
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
{ //Duplicated code with setTodoItemAction
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

  processObjectModified();
}

void ZFlyEmProofDoc::setTodoItemAction(neutube::EToDoAction action)
{ //Duplicated code with checkTodoItem
  ZOUT(LTRACE(), 5) << "Set action of to do items";
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
        if (action != item.getAction()) {
          item.setAction(action);
          td->addItem(item, ZFlyEmToDoList::DATA_GLOBAL);
          ptArray.push_back(item.getPosition());
        }
      }
    }
    if (!selectedSet.empty()) {
      bufferObjectModified(td);
      notifyTodoItemModified(ptArray, true);
    }
  }

  processObjectModified();
}

void ZFlyEmProofDoc::setTodoItemToNormal()
{
  setTodoItemAction(neutube::TO_DO);
}

void ZFlyEmProofDoc::setTodoItemIrrelevant()
{
  setTodoItemAction(neutube::TO_DO_IRRELEVANT);
}

void ZFlyEmProofDoc::setTodoItemToMerge()
{
  setTodoItemAction(neutube::TO_MERGE);
}

void ZFlyEmProofDoc::setTodoItemToSplit()
{
  setTodoItemAction(neutube::TO_SPLIT);
}

void ZFlyEmProofDoc::tryMoveSelectedSynapse(
    const ZIntPoint &dest, neutube::EAxis axis)
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

      processObjectModified();
    }
  }
}

void ZFlyEmProofDoc::annotateSelectedSynapse(
    ZFlyEmSynapseAnnotationDialog *dlg, neutube::EAxis axis)
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

QColor ZFlyEmProofDoc::getSeedColor(int label) const
{
  return ZStroke2d::GetLabelColor(label);
}

uint64_t ZFlyEmProofDoc::getBodyIdForSplit() const
{
  uint64_t label = 0;

  const ZDvidSparseStack *spStack = getBodyForSplit();

  if (spStack != NULL) {
    label = spStack->getLabel();
  }

  return label;
}

void ZFlyEmProofDoc::setRoutineCheck(bool on)
{
  m_routineCheck = on;
}

void ZFlyEmProofDoc::annotateSynapse(
    const ZIntPoint &pt, ZJsonObject propJson, neutube::EAxis axis)
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

    processObjectModified();
  }
}

void ZFlyEmProofDoc::annotateSelectedSynapse(
    ZJsonObject propJson, neutube::EAxis axis)
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
  synapse.setUserName(neutube::GetCurrentUserName());

//  ZDvidSynapseEnsemble::EDataScope scope = ZDvidSynapseEnsemble::DATA_GLOBAL;
  QList<ZDvidSynapseEnsemble*> seList = getDvidSynapseEnsembleList();
  for (QList<ZDvidSynapseEnsemble*>::const_iterator iter = seList.begin();
       iter != seList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    se->addSynapse(synapse, scope);
    scope = ZDvidSynapseEnsemble::DATA_LOCAL;
    processObjectModified(se);
  }

  processObjectModified();
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

  processObjectModified();
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

  processObjectModified();
}

void ZFlyEmProofDoc::addTodoItem(const ZIntPoint &pos)
{
  ZFlyEmToDoItem item(pos);
  item.setUserName(neutube::GetCurrentUserName());

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

  processObjectModified();
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

  processObjectModified();
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

  processObjectModified();
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

  processObjectModified();
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

  processObjectModified();
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

  processObjectModified();
}

void ZFlyEmProofDoc::highlightPsd(bool on)
{
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  QList<ZDvidSynapseEnsemble*> synapseList = getDvidSynapseEnsembleList();
  for (QList<ZDvidSynapseEnsemble*>::const_iterator iter = synapseList.begin();
       iter != synapseList.end(); ++iter) {
    ZDvidSynapseEnsemble *se = *iter;
    if (on) {
      se->addVisualEffect(neutube::display::VE_GROUP_HIGHLIGHT);
    } else {
      se->removeVisualEffect(neutube::display::VE_GROUP_HIGHLIGHT);
    }
    processObjectModified(se);
  }
  endObjectModifiedMode();

  processObjectModified();
}

/*
bool ZFlyEmProofDoc::checkInBody(uint64_t bodyId)
{
  if (getSupervisor() != NULL) {
    return getSupervisor()->checkIn(bodyId);
  }

  return true;
}
*/

bool ZFlyEmProofDoc::checkBodyWithMessage(
    uint64_t bodyId, bool checkingOut, flyem::EBodySplitMode mode)
{
  bool succ = true;

  if (checkingOut) {
    succ = checkOutBody(bodyId, mode);
  } else {
    succ = checkInBodyWithMessage(bodyId, mode);
  }

  return succ;
}

bool ZFlyEmProofDoc::checkInBodyWithMessage(
    uint64_t bodyId, flyem::EBodySplitMode mode)
{
  if (getSupervisor() != NULL) {
    if (bodyId > 0) {
      if (getSupervisor()->checkIn(bodyId, mode)) {
        emit messageGenerated(QString("Body %1 is unlocked.").arg(bodyId));
        return true;
      } else {
        emit messageGenerated(
              ZWidgetMessage(
                QString("Failed to unlock body %1.").arg(bodyId),
                neutube::MSG_ERROR));
      }
    }
  }

  return true;
}

bool ZFlyEmProofDoc::checkOutBody(uint64_t bodyId, flyem::EBodySplitMode mode)
{
  if (getSupervisor() != NULL) {
    return getSupervisor()->checkOut(bodyId, mode);
  }

  return true;
}


std::set<uint64_t> ZFlyEmProofDoc::getCurrentSelectedBodyId(
    neutube::EBodyLabelType type) const
{
  const ZDvidLabelSlice *labelSlice = getDvidLabelSlice(neutube::Z_AXIS);
  if (labelSlice != NULL) {
    return labelSlice->getSelected(type);
  }

  return std::set<uint64_t>();
}

void ZFlyEmProofDoc::deselectMappedBodyWithOriginalId(
    const std::set<uint64_t> &bodySet)
{
  deselectMappedBody(bodySet, neutube::BODY_LABEL_ORIGINAL);
}

/*
void ZFlyEmProofDoc::makeAction(ZActionFactory::EAction item)
{
  if (!m_actionMap.contains(item)) {
    QAction *action = NULL;
    switch (item) {
    case ZActionFactory::ACTION_DESELECT_BODY:
      connect(action, SIGNAL(triggered()), this, SLOT(deselectMappedBodyWithOriginalId(uint64_t))
    }
  }

  ZStackDoc::makeAction(item);
}
*/

void ZFlyEmProofDoc::checkInSelectedBody(flyem::EBodySplitMode mode)
{
  if (getSupervisor() != NULL) {
    std::set<uint64_t> bodyIdArray =
        getCurrentSelectedBodyId(neutube::BODY_LABEL_ORIGINAL);
    for (std::set<uint64_t>::const_iterator iter = bodyIdArray.begin();
         iter != bodyIdArray.end(); ++iter) {
      uint64_t bodyId = *iter;
      if (bodyId > 0) {
        if (getSupervisor()->checkIn(bodyId, mode)) {
          emit messageGenerated(QString("Body %1 is unlocked.").arg(bodyId));
        } else {
          emit messageGenerated(
                ZWidgetMessage(
                  QString("Failed to unlock body %1.").arg(bodyId),
                  neutube::MSG_ERROR));
        }
      }
    }
  } else {
    emit messageGenerated(QString("Body lock service is not available."));
  }
}

void ZFlyEmProofDoc::checkInSelectedBodyAdmin()
{
  if (getSupervisor() != NULL) {
    std::set<uint64_t> bodyIdArray =
        getCurrentSelectedBodyId(neutube::BODY_LABEL_ORIGINAL);
    for (std::set<uint64_t>::const_iterator iter = bodyIdArray.begin();
         iter != bodyIdArray.end(); ++iter) {
      uint64_t bodyId = *iter;
      if (bodyId > 0) {
        if (getSupervisor()->isLocked(bodyId)) {
          if (getSupervisor()->checkInAdmin(bodyId)) {
            emit messageGenerated(QString("Body %1 is unlocked.").arg(bodyId));
          } else {
            emit messageGenerated(
                  ZWidgetMessage(
                    QString("Failed to unlock body %1.").arg(bodyId),
                    neutube::MSG_ERROR));
          }
        } else {
          emit messageGenerated(QString("Body %1 is unlocked.").arg(bodyId));
        }
      }
    }
  } else {
    emit messageGenerated(QString("Body lock service is not available."));
  }
}

void ZFlyEmProofDoc::checkOutBody(flyem::EBodySplitMode mode)
{
  if (getSupervisor() != NULL) {
    std::set<uint64_t> bodyIdArray =
        getCurrentSelectedBodyId(neutube::BODY_LABEL_ORIGINAL);
    for (std::set<uint64_t>::const_iterator iter = bodyIdArray.begin();
         iter != bodyIdArray.end(); ++iter) {
      uint64_t bodyId = *iter;
      if (bodyId > 0) {
        if (getSupervisor()->checkOut(bodyId, mode)) {
          emit messageGenerated(QString("Body %1 is locked.").arg(bodyId));
        } else {
          std::string owner = getSupervisor()->getOwner(bodyId);
          if (owner.empty()) {
            emit messageGenerated(
                  ZWidgetMessage(
                    QString("Failed to lock body %1. Is the librarian sever (%2) ready?").
                    arg(bodyId).arg(getDvidTarget().getSupervisor().c_str()),
                    neutube::MSG_ERROR));
          } else {
            emit messageGenerated(
                  ZWidgetMessage(
                    QString("Failed to lock body %1 because it has been locked by %2").
                    arg(bodyId).arg(owner.c_str()), neutube::MSG_ERROR));
          }
        }
      }
    }
  } else {
    emit messageGenerated(QString("Body lock service is not available."));
  }
}


void ZFlyEmProofDoc::verifySelectedSynapse()
{
  const std::string &userName = neutube::GetCurrentUserName();
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
  processObjectModified();
}

void ZFlyEmProofDoc::unverifySelectedSynapse()
{
  const std::string &userName = neutube::GetCurrentUserName();
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
  processObjectModified();
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
  processObjectModified();

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
      processObjectModified();
    }
  }
  */
}

void ZFlyEmProofDoc::clearBodyForSplit()
{
  removeObject(ZStackObjectSourceFactory::MakeSplitObjectSource(), true);
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
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  QList<ZDvidLabelSlice*> sliceList = getDvidLabelSliceList();
  foreach (ZDvidLabelSlice *slice, sliceList) {
    slice->paintBuffer();
    processObjectModified(slice);
//    slice->clearSelection();
//    slice->updateLabelColor();
  }

  QList<ZDvidSparsevolSlice*> sparsevolSliceList = getDvidSparsevolSliceList();
  foreach (ZDvidSparsevolSlice *slice, sparsevolSliceList) {
//    slice->setLabel(m_bodyMerger.getFinalLabel(slice->getLabel()));
//    uint64_t finalLabel = m_bodyMerger.getFinalLabel(slice->getLabel());
    slice->setColor(getDvidLabelSlice(neutube::Z_AXIS)->getLabelColor(
                      slice->getLabel(), neutube::BODY_LABEL_ORIGINAL));
    processObjectModified(slice);
    //slice->updateSelection();
  }
  endObjectModifiedMode();
  processObjectModified();
}

void ZFlyEmProofDoc::clearData()
{
  ZStackDoc::clearData();
  m_bodyMerger.clear();
  m_dvidReader.clear();
//  m_dvidTarget.clear();
  m_grayScaleInfo.clear();
  m_labelInfo.clear();
  m_versionDag.clear();
  m_infoJson.clear();
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

void ZFlyEmProofDoc::processExternalBodyMergeUpload()
{
  getMergeProject()->clearBodyMerger();
  refreshDvidLabelBuffer(2000);
  updateDvidLabelObjectSliently();

  emit bodyMergeUploadedExternally();
}

void ZFlyEmProofDoc::saveMergeOperation()
{
  ZDvidWriter &writer = m_dvidWriter;
  if (writer.good()) {
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
                           neutube::MSG_ERROR));
    }
  }
}

ZJsonArray ZFlyEmProofDoc::getMergeOperation() const
{
  return m_bodyMerger.toJsonArray();
}

void ZFlyEmProofDoc::backupMergeOperation()
{
  if (!m_mergeAutoSavePath.isEmpty()) {
    if (!m_bodyMerger.isEmpty()) {
      m_bodyMerger.toJsonArray().dump(m_mergeAutoSavePath.toStdString());
    }
  }
}

void ZFlyEmProofDoc::prepareDvidLabelSlice(
    const ZStackViewParam &viewParam, int zoom, int centerCutX, int centerCutY,
    bool usingCenterCut)
{
  if (!m_workWriter.good()) {
    m_workWriter.open(getDvidTarget());
  }

  ZArray *array = NULL;

  if (m_workWriter.good()) {
    if (viewParam.getSliceAxis() == neutube::A_AXIS) {
      ZArbSliceViewParam svp = viewParam.getSliceViewParam();
      array = m_workWriter.getDvidReader().readLabels64Lowtis(
            svp.getCenter(), svp.getPlaneV1(), svp.getPlaneV2(),
            svp.getWidth(), svp.getHeight(),
            zoom, centerCutX, centerCutY, usingCenterCut);
    } else {
      ZIntCuboid box = ZDvidDataSliceHelper::GetBoundBox(
            viewParam.getViewPort(), viewParam.getZ());

      array = m_workWriter.getDvidReader().readLabels64Lowtis(
            box.getFirstCorner().getX(), box.getFirstCorner().getY(),
            box.getFirstCorner().getZ(), box.getWidth(), box.getHeight(),
            zoom, centerCutX, centerCutY, usingCenterCut);
    }
  }

  if (array != NULL) {
    emit updatingLabelSlice(array, viewParam, zoom, centerCutX, centerCutY,
                            usingCenterCut);
  }
}

void ZFlyEmProofDoc::prepareDvidGraySlice(
    const ZStackViewParam &viewParam, int zoom, int centerCutX, int centerCutY,
    bool usingCenterCut)
{
  if (!m_grayscaleWorkReader.good()) {
    m_grayscaleWorkReader.open(getDvidTarget().getGrayScaleTarget());
  }

  ZStack *array = NULL;

  if (m_grayscaleWorkReader.good()) {
    if (viewParam.getSliceAxis() == neutube::A_AXIS) {
      ZArbSliceViewParam svp = viewParam.getSliceViewParam();
      array = m_grayscaleWorkReader.readGrayScaleLowtis(
            svp.getCenter(), svp.getPlaneV1(), svp.getPlaneV2(),
            svp.getWidth(), svp.getHeight(),
            zoom, centerCutX, centerCutY, usingCenterCut);
    } else {
      ZIntCuboid box = ZDvidDataSliceHelper::GetBoundBox(
            viewParam.getViewPort(), viewParam.getZ());

      array = m_grayscaleWorkReader.readGrayScaleLowtis(
            box.getFirstCorner().getX(), box.getFirstCorner().getY(),
            box.getFirstCorner().getZ(), box.getWidth(), box.getHeight(),
            zoom, centerCutX, centerCutY, usingCenterCut);
    }
  }

  if (array != NULL) {
    emit updatingGraySlice(array, viewParam, zoom, centerCutX, centerCutY,
                           usingCenterCut);
  }
}

/*
void ZFlyEmProofDoc::downloadBodyMask()
{
  if (getBodyForSplit() != NULL) {
    getBodyForSplit()->downloadBodyMask();
    processObjectModified();
  }
}
*/

ZWidgetMessage ZFlyEmProofDoc::getAnnotationFailureMessage(uint64_t bodyId) const
{
  ZWidgetMessage msg;

  if (getSupervisor() != NULL) {
    std::string owner = getSupervisor()->getOwner(bodyId);
    if (owner.empty()) {
//            owner = "unknown user";
       msg = ZWidgetMessage(
              QString("Failed to lock body %1. Is the librarian sever (%2) ready?").
              arg(bodyId).arg(getDvidTarget().getSupervisor().c_str()),
              neutube::MSG_ERROR);
    } else {
      msg = ZWidgetMessage(
              QString("Failed to start annotation. %1 has been locked by %2").
              arg(bodyId).arg(owner.c_str()), neutube::MSG_ERROR);
    }
  }

  return msg;
}

void ZFlyEmProofDoc::updateLabelSlice(
    ZArray *array, const ZStackViewParam &viewParam, int zoom,
    int centerCutX, int centerCutY, bool usingCenterCut)
{
  if (array != NULL) {
    ZDvidLabelSlice *slice = getDvidLabelSlice(viewParam.getSliceAxis());
    if (slice != NULL) {
      if (slice->consume(
            array, viewParam, zoom, centerCutX, centerCutY, usingCenterCut)) {
        bufferObjectModified(slice->getTarget());
        processObjectModified();
      }
    } else {
      delete array;
    }
  }
}

void ZFlyEmProofDoc::updateGraySlice(
    ZStack *array, const ZStackViewParam &viewParam, int zoom,
    int centerCutX, int centerCutY, bool usingCenterCut)
{
  if (array != NULL) {
    ZDvidGraySlice *slice = getDvidGraySlice(viewParam.getSliceAxis());
    if (slice != NULL) {
      if (slice->consume(array, viewParam, zoom, centerCutX, centerCutY,
                         usingCenterCut)) {
        bufferObjectModified(slice->getTarget());
        processObjectModified();
      }
    } else {
      delete array;
    }
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

void ZFlyEmProofDoc::notifyBodyMergeSaved()
{
  emit bodyMergeSaved();
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

void ZFlyEmProofDoc::updateDvidLabelSlice(neutube::EAxis axis)
{
  beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  ZOUT(LTRACE(), 5) << "Update dvid label";
  TStackObjectList &objList = getObjectList(ZStackObject::TYPE_DVID_LABEL_SLICE);
  for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
       ++iter) {
    ZDvidLabelSlice *obj = dynamic_cast<ZDvidLabelSlice*>(*iter);
    if (obj->getSliceAxis() == axis) {
//      obj->clearCache();
      obj->forceUpdate(false);
      processObjectModified(obj);
    }
  }
  endObjectModifiedMode();
  processObjectModified();
}

void ZFlyEmProofDoc::loadSplitFromService()
{
//  std::string path = GET_FLYEM_CONFIG.getSplitResultUrl(
//        getDvidTarget(), getBodyIdForSplit());

  QList<ZObject3dScan*> objList = ZServiceConsumer::ReadSplitResult(
        GET_FLYEM_CONFIG.getTaskServer().c_str(), getDvidTarget(),
        getBodyIdForSplit());
      //ZDvidResultService::ReadSplitResult(path);
//  int index = 1;
  foreach (ZObject3dScan *obj, objList) {
    obj->setColor(getSeedColor(obj->getLabel()));
    obj->setObjectClass(ZStackObjectSourceFactory::MakeSplitResultSource());
    obj->setHitProtocal(ZStackObject::HIT_NONE);
    obj->setVisualEffect(neutube::display::SparseObject::VE_PLANE_BOUNDARY);
    obj->setProjectionVisible(false);
    obj->setRole(ZStackObjectRole::ROLE_TMP_RESULT);
    obj->addRole(ZStackObjectRole::ROLE_SEGMENTATION);
  }

  removeObject(ZStackObjectRole::ROLE_TMP_RESULT, true);
  m_dataBuffer->addUpdate(
        objList.begin(), objList.end(),
        ZStackDocObjectUpdate::ACTION_ADD_UNIQUE);
  m_dataBuffer->deliver();
}

void ZFlyEmProofDoc::loadSplitTaskFromService()
{
  uint64_t originalId = getBodyIdForSplit();
  if (originalId > 0) {
    QList<ZStackObject*> seedList = ZServiceConsumer::ReadSplitTaskSeed(
          GET_FLYEM_CONFIG.getTaskServer().c_str(), getDvidTarget(), originalId);
    getDataBuffer()->addUpdate(seedList, ZStackDocObjectUpdate::ACTION_ADD_NONUNIQUE);
    getDataBuffer()->deliver();
  }
}

void ZFlyEmProofDoc::commitSplitFromService()
{
  uint64_t originalId = getBodyIdForSplit();

  QList<ZObject3dScan*> objList = ZServiceConsumer::ReadSplitResult(
        GET_FLYEM_CONFIG.getTaskServer().c_str(), getDvidTarget(), originalId);
  foreach (ZObject3dScan *obj, objList) {
    uint64_t newBodyId = getDvidWriter().writeSplit(*obj, originalId, 0);
    emit messageGenerated(QString("%1 uploaded").arg(newBodyId));
  }
}

int ZFlyEmProofDoc::removeDvidSparsevol(neutube::EAxis axis)
{
  int count = 0;

  TStackObjectList objList =
      getObjectList(ZStackObject::TYPE_DVID_SPARSEVOL_SLICE);

  for (TStackObjectList::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZStackObject *obj = *iter;
    if (obj->getSliceAxis() == axis) {
      removeObject(obj, true);
      ++count;
    }
  }

  return count;
}


std::vector<ZDvidSparsevolSlice*> ZFlyEmProofDoc::makeSelectedDvidSparsevol(
    const ZDvidLabelSlice *labelSlice)
{
  std::vector<ZDvidSparsevolSlice*> sparsevolList;
  if (labelSlice != NULL) {
    const std::set<uint64_t> &selected = labelSlice->getSelectedOriginal();
    for (std::set<uint64_t>::const_iterator iter = selected.begin();
         iter != selected.end(); ++iter) {
      uint64_t bodyId = *iter;
      ZDvidSparsevolSlice *obj = makeDvidSparsevol(labelSlice, bodyId);
      sparsevolList.push_back(obj);
    }
  }

  return sparsevolList;
}

ZDvidSparsevolSlice* ZFlyEmProofDoc::makeDvidSparsevol(
    const ZDvidLabelSlice *labelSlice, uint64_t bodyId)
{
  ZDvidSparsevolSlice *obj = NULL;
  if (bodyId > 0) {
    obj = new ZDvidSparsevolSlice;
    obj->setTarget(ZStackObject::TARGET_DYNAMIC_OBJECT_CANVAS);
    obj->setSliceAxis(labelSlice->getSliceAxis());
    obj->setReader(getSparseVolReader());
    //          obj->setDvidTarget(getDvidTarget());
    obj->setLabel(bodyId);
    obj->setRole(ZStackObjectRole::ROLE_ACTIVE_VIEW);
    obj->setColor(labelSlice->getLabelColor(
                    bodyId, neutube::BODY_LABEL_ORIGINAL));
    obj->setVisible(!labelSlice->isVisible());
  }

  return obj;
}

void ZFlyEmProofDoc::updateDvidLabelObject(neutube::EAxis axis)
{
  beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  ZDvidLabelSlice *labelSlice = getDvidLabelSlice(axis);
  if (labelSlice != NULL) {
//    labelSlice->clearCache();
    labelSlice->forceUpdate(false);
  }

  removeDvidSparsevol(axis);

  std::vector<ZDvidSparsevolSlice*> sparsevolList =
      makeSelectedDvidSparsevol(labelSlice);
  for (std::vector<ZDvidSparsevolSlice*>::iterator iter = sparsevolList.begin();
       iter != sparsevolList.end(); ++iter) {
    ZDvidSparsevolSlice *sparsevol = *iter;
    addObject(sparsevol);
  }

  endObjectModifiedMode();
  processObjectModified();
}

void ZFlyEmProofDoc::updateDvidLabelObjectSliently()
{
  updateDvidLabelObject(OBJECT_MODIFIED_SLIENT);
}

#if 0
void ZFlyEmProofDoc::updateDvidLabelSlice()
{
  ZOUT(LTRACE(), 5) << "Update dvid label";
  TStackObjectList &objList = getObjectList(ZStackObject::TYPE_DVID_LABEL_SLICE);
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
       ++iter) {
    ZDvidLabelSlice *obj = dynamic_cast<ZDvidLabelSlice*>(*iter);
    obj->clearCache();
    obj->forceUpdate(false);
    processObjectModified(obj);
  }
  endObjectModifiedMode();
  processObjectModified();
}
#endif

void ZFlyEmProofDoc::updateDvidLabelObject(EObjectModifiedMode updateMode)
{
  beginObjectModifiedMode(updateMode);

  updateDvidLabelObject(neutube::X_AXIS);
  updateDvidLabelObject(neutube::Y_AXIS);
  updateDvidLabelObject(neutube::Z_AXIS);

  endObjectModifiedMode();
  processObjectModified();

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
    std::string currentUserName = neutube::GetCurrentUserName();
    ZJsonArray bookmarkJson =
        m_dvidReader.readTaggedBookmark("user:" + currentUserName);
    beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
    int bookmarkCount = 0;
    for (size_t i = 0; i < bookmarkJson.size(); ++i) {
      ZFlyEmBookmark *bookmark = new ZFlyEmBookmark;
      ZJsonObject bookmarkObj = ZJsonObject(bookmarkJson.value(i));
      bookmark->loadDvidAnnotation(bookmarkObj);
      bool good =
          (bookmark->getUserName().length() == (int) currentUserName.length());
      if (good) {
        ZJsonObject checkJson =
            m_dvidReader.readBookmarkJson(bookmark->getCenter().toIntPoint());
        good = (!checkJson.isEmpty());
      }
      if (good) {
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
    processObjectModified();

    if (bookmarkCount == 0) {
      ZDvidUrl url(getDvidTarget());
      ZDvidBufferReader reader;
      reader.read(url.getCustomBookmarkUrl(neutube::GetCurrentUserName()).c_str());
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
        processObjectModified();
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
      flyem::ZSynapseAnnotationArray synapseArray;
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

  processObjectModified();
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
  processObjectModified();
}

void ZFlyEmProofDoc::downloadSynapse()
{
  if (!getDvidTarget().getSynapseName().empty()) {
    ZDvidSynapseEnsemble *synapseEnsemble = new ZDvidSynapseEnsemble;
    synapseEnsemble->setDvidTarget(getDvidTarget());
    synapseEnsemble->setDvidInfo(getDvidInfo());
    synapseEnsemble->setSource(
          ZStackObjectSourceFactory::MakeDvidSynapseEnsembleSource());
    synapseEnsemble->setResolution(m_grayScaleInfo.getVoxelResolution());

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
  todoList->setDvidInfo(getDvidInfo());
  todoList->setSource(ZStackObjectSourceFactory::MakeTodoListEnsembleSource());
  addObject(todoList);
}

void ZFlyEmProofDoc::processBookmarkAnnotationEvent(ZFlyEmBookmark * /*bookmark*/)
{
//  m_isCustomBookmarkSaved = false;
}

void ZFlyEmProofDoc::decorateTBar(ZPuncta *puncta)
{
  puncta->setSource(ZStackObjectSourceFactory::MakeFlyEmTBarSource());
  puncta->pushCosmeticPen(true);
  puncta->pushColor(QColor(0, 255, 0));
  puncta->pushVisualEffect(neutube::display::Sphere::VE_CROSS_CENTER |
                           neutube::display::Sphere::VE_OUT_FOCUS_DIM);
}

void ZFlyEmProofDoc::decoratePsd(ZPuncta *puncta)
{
  puncta->setSource(ZStackObjectSourceFactory::MakeFlyEmPsdSource());
  puncta->pushCosmeticPen(true);
  puncta->pushColor(QColor(0, 0, 255));
  puncta->pushVisualEffect(neutube::display::Sphere::VE_CROSS_CENTER |
                           neutube::display::Sphere::VE_OUT_FOCUS_DIM);
}

void ZFlyEmProofDoc::decorateTBar(ZSlicedPuncta *puncta)
{
  puncta->setSource(ZStackObjectSourceFactory::MakeFlyEmTBarSource());
  puncta->pushCosmeticPen(true);
  puncta->pushColor(QColor(0, 255, 0));
  puncta->pushVisualEffect(neutube::display::Sphere::VE_CROSS_CENTER |
                           neutube::display::Sphere::VE_OUT_FOCUS_DIM);
}

void ZFlyEmProofDoc::decoratePsd(ZSlicedPuncta *puncta)
{
  puncta->setSource(ZStackObjectSourceFactory::MakeFlyEmPsdSource());
  puncta->pushCosmeticPen(true);
  puncta->pushColor(QColor(0, 0, 255));
  puncta->pushVisualEffect(neutube::display::Sphere::VE_CROSS_CENTER |
                           neutube::display::Sphere::VE_OUT_FOCUS_DIM);
}

void ZFlyEmProofDoc::prepareGraySlice(ZDvidGraySlice *slice)
{
  if (slice != NULL) {
    slice->setCenterCut(m_graySliceCenterCutWidth, m_graySliceCenterCutHeight);
  }
}

void ZFlyEmProofDoc::prepareLabelSlice()
{
  auto sliceList = getDvidLabelSliceList();
  for (auto &slice : sliceList) {
    slice->setCenterCut(m_labelSliceCenterCutWidth, m_labelSliceCenterCutHeight);
  }
}

std::vector<ZPunctum*> ZFlyEmProofDoc::getTbar(ZObject3dScan &body)
{
  std::vector<ZPunctum*> puncta;
  ZSlicedPuncta  *tbar = dynamic_cast<ZSlicedPuncta*>(
        getObjectGroup().findFirstSameSource(
          ZStackObject::TYPE_SLICED_PUNCTA,
          ZStackObjectSourceFactory::MakeFlyEmTBarSource()));

  if (tbar != NULL) {
    ZIntCuboid box = body.getBoundBox();
    int minZ = box.getFirstCorner().getZ();
    int maxZ = box.getLastCorner().getZ();

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
    QMutexLocker locker(&m_synapseReaderMutex);
    ZDvidReader &reader = m_synapseReader;
//    reader.setVerbose(false);
    if (reader.isReady()) {
      ZIntCuboid box = reader.readBodyBoundBox(bodyId);
      int minZ = box.getFirstCorner().getZ();
      int maxZ = box.getLastCorner().getZ();

      ZObject3dScan coarseBody = reader.readCoarseBody(bodyId);
      ZDvidInfo dvidInfo = m_grayScaleInfo;

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
//  reader.setVerbose(false);
  const double radius = 50.0;
  QMutexLocker locker(&m_synapseReaderMutex);
  ZDvidReader &reader = m_synapseReader;
  if (reader.isReady()) {
    std::vector<ZDvidSynapse> synapseArray =
        reader.readSynapse(bodyId, flyem::LOAD_PARTNER_RELJSON);

    std::vector<ZPunctum*> &tbar = synapse.first;
    std::vector<ZPunctum*> &psd = synapse.second;

    for (std::vector<ZDvidSynapse>::const_iterator iter = synapseArray.begin();
         iter != synapseArray.end(); ++iter) {
      const ZDvidSynapse &synapse = *iter;
      ZPunctum *punctum = new ZPunctum(synapse.getPosition(), radius);
      punctum->setName(getSynapseName(synapse).c_str());

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

std::string ZFlyEmProofDoc::getSynapseName(const ZDvidSynapse &synapse) const
{
  std::string name;

  if (GET_FLYEM_CONFIG.anayzingMb6()) {
    name = m_analyzer.getPunctumName(synapse).toStdString();
  } else {
    name = std::to_string(synapse.getBodyId());
    if (GET_FLYEM_CONFIG.psdNameDetail()) {
      if (synapse.getKind() == ZDvidSynapse::KIND_POST_SYN) {
        ZJsonArray jsonArray = synapse.getRelationJson();
        ZJsonObject jsonObj(jsonArray.value(0));
        if (jsonObj.hasKey("To")) {
          ZJsonArray ptJson(jsonObj.value("To"));
          ZIntPoint pt;
          pt.set(ZJsonParser::integerValue(ptJson.at(0)),
                 ZJsonParser::integerValue(ptJson.at(1)),
                 ZJsonParser::integerValue(ptJson.at(2)));
          uint64_t tbarId = m_synapseReader.readBodyIdAt(pt);
          name = name + "<-" + std::to_string(tbarId);
        }
      }
    }
  }

  return name;
}

std::vector<ZFlyEmToDoItem*> ZFlyEmProofDoc::getTodoItem(uint64_t bodyId)
{
  std::vector<ZFlyEmToDoItem*> puncta;
  QMutexLocker locker(&m_todoReaderMutex);
  ZDvidReader &reader = m_todoReader;
  if (reader.isReady()) {
    ZJsonArray annotationJson = reader.readAnnotation(
          getDvidTarget().getTodoListName(), bodyId);

    for (size_t i = 0; i < annotationJson.size(); ++i) {
      ZFlyEmToDoItem *item = new ZFlyEmToDoItem;

      ZJsonObject objJson(annotationJson.value(i));
      item->loadJsonObject(objJson, flyem::LOAD_NO_PARTNER);

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

std::vector<ZPunctum*> ZFlyEmProofDoc::getTodoPuncta(uint64_t bodyId)
{
  std::vector<ZPunctum*> puncta;
//  reader.setVerbose(false);
  const double radius = 50.0;
  QMutexLocker locker(&m_todoReaderMutex);
  ZDvidReader &reader = m_todoReader;
  if (reader.isReady()) {
    ZJsonArray annotationJson = reader.readAnnotation(
          getDvidTarget().getTodoListName(), bodyId);

    for (size_t i = 0; i < annotationJson.size(); ++i) {
      ZFlyEmToDoItem item;

      ZJsonObject objJson(annotationJson.value(i));
      item.loadJsonObject(objJson, flyem::LOAD_PARTNER_RELJSON);

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
          bodyId = ZString::FirstInteger(ZJsonParser::stringValue(idJson.getData()));
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
          bookmark->setHitProtocal(ZStackObject::HIT_NONE);
//          bookmark->setHittable(false);
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

  processObjectModified();

  m_loadingAssignedBookmark = false;

  ZOUT(LINFO(), 3) << "Bookmark imported";

  return bookmarkList;
}

QString ZFlyEmProofDoc::getInfo() const
{
  QString info = getDvidTarget().toJsonObject().dumpString(2).c_str();
  if (getDvidTarget().hasGrayScaleData()) {
    info.append("\n");
    info.append("Grayscale setup:\n");
    info.append(m_grayscaleReader.getDvidTarget().toJsonObject().dumpString(2).c_str());
  }

  return info;
}

uint64_t ZFlyEmProofDoc::getBodyId(int x, int y, int z)
{
  uint64_t bodyId = 0;
  ZDvidReader &reader = getDvidReader();
  if (reader.good()) {
    bodyId = m_bodyMerger.getFinalLabel(reader.readBodyIdAt(x, y, z));
  }

  return bodyId;
}

uint64_t ZFlyEmProofDoc::getBodyId(const ZIntPoint &pt)
{
  return getBodyId(pt.getX(), pt.getY(), pt.getZ());
}

uint64_t ZFlyEmProofDoc::getLabelId(int x, int y, int z)
{
  uint64_t bodyId = 0;
  ZDvidReader &reader = getDvidReader();
  if (reader.good()) {
    bodyId = reader.readBodyIdAt(x, y, z);
  }

  return bodyId;
}

uint64_t ZFlyEmProofDoc::getSupervoxelId(int x, int y, int z)
{
  uint64_t bodyId = 0;
  ZDvidReader &reader = getDvidReader();
  if (reader.good()) {
    bodyId = reader.readSupervoxelIdAt(x, y, z);
    if (bodyId > 0) {
      bodyId = ZFlyEmBodyManager::encodeSupervoxel(bodyId);
    }
  }

  return bodyId;
}

void ZFlyEmProofDoc::autoSave()
{
  autoSaveSwc();
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
      processObjectModified();
    }
  } else {
    LDEBUG() << "Updating contrast:" << highContrast;
    ZDvidGraySlice *slice = getDvidGraySlice();
    if (slice != NULL) {
      slice->updateContrast(highContrast);
      bufferObjectModified(slice->getTarget());
      processObjectModified();
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
      notify(ZWidgetMessage("Failed to refresh labels.", neutube::MSG_WARNING));
    }
  }
}

void ZFlyEmProofDoc::updateMeshForSelected()
{
  const std::set<uint64_t> &bodySet =
      getSelectedBodySet(neutube::BODY_LABEL_ORIGINAL);
  for (uint64_t bodyId : bodySet) {
    ZObject3dScan obj;
    getDvidReader().readBody(bodyId, true, &obj);
    ZMesh *mesh = ZMeshFactory::MakeMesh(obj);
    getDvidWriter().writeMesh(*mesh, bodyId, 0);
    delete mesh;
  }
}

/*
void ZFlyEmProofDoc::setLabelSliceAlpha(int alpha)
{
  QList<ZDvidLabelSlice*> sliceList = getDvidLabelSliceList();
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  foreach (ZDvidLabelSlice *slice, sliceList) {
    slice->setAlpha(alpha);
    processObjectModified(slice->getTarget());
  }
  endObjectModifiedMode();
  processObjectModified();
}
*/

ZIntCuboidObj* ZFlyEmProofDoc::getSplitRoi() const
{
  return dynamic_cast<ZIntCuboidObj*>(
      getObjectGroup().findFirstSameSource(
        ZStackObject::TYPE_INT_CUBOID,
        ZStackObjectSourceFactory::MakeFlyEmSplitRoiSource()));
}

bool ZFlyEmProofDoc::isSplitRunning() const
{
  return m_futureMap.isAlive(THREAD_SPLIT);
}

void ZFlyEmProofDoc::exitSplit()
{
  m_futureMap.waitForFinished(THREAD_SPLIT);
}

void ZFlyEmProofDoc::runSplit(flyem::EBodySplitMode mode)
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
    message.setType(neutube::MSG_WARNING);

    emit messageGenerated(message);
    return;
  }

  if (!m_futureMap.isAlive(THREAD_SPLIT)) {
    m_futureMap.removeDeadThread();
    QFuture<void> future =
        QtConcurrent::run(this, &ZFlyEmProofDoc::runSplitFunc, mode);
    m_futureMap[THREAD_SPLIT] = future;
  }
}

void ZFlyEmProofDoc::runFullSplit(flyem::EBodySplitMode mode)
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
    message.setType(neutube::MSG_WARNING);

    emit messageGenerated(message);
    return;
  }

  if (!m_futureMap.isAlive(THREAD_SPLIT)) {
    m_futureMap.removeDeadThread();
    QFuture<void> future =
        QtConcurrent::run(this, &ZFlyEmProofDoc::runFullSplitFunc, mode);
    m_futureMap[THREAD_SPLIT] = future;
  }
}


#if 0
void ZFlyEmProofDoc::runLocalSplit(FlyEM::EBodySplitMode mode)
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
        QtConcurrent::run(this, &ZFlyEmProofDoc::localSplitFunc, mode);
    m_futureMap[threadId] = future;
  }
}
#endif

void ZFlyEmProofDoc::runSplitFunc(flyem::EBodySplitMode mode)
{
  runSplitFunc(mode, flyem::RANGE_SEED);
}

void ZFlyEmProofDoc::runLocalSplit(flyem::EBodySplitMode mode)
{
  runSplitFunc(mode, flyem::RANGE_LOCAL);
}

/*
void ZFlyEmProofDoc::runLocalSplit()
{
  runLocalSplit(flyem::BODY_SPLIT_ONLINE);
}
*/

void ZFlyEmProofDoc::runFullSplitFunc(flyem::EBodySplitMode mode)
{
  runSplitFunc(mode, flyem::RANGE_FULL);
}

void ZFlyEmProofDoc::runSplitFunc(
    flyem::EBodySplitMode mode, flyem::EBodySplitRange range)
{
  getProgressSignal()->startProgress("Splitting ...");

  ZOUT(LINFO(), 3) << "Removing old result ...";
  removeObject(ZStackObjectRole::ROLE_SEGMENTATION, true);
//  m_isSegmentationReady = false;
  setSegmentationReady(false);

  getProgressSignal()->advanceProgress(0.1);

  QList<ZStackObject*> seedList = getObjectList(ZStackObjectRole::ROLE_SEED);
  if (seedList.size() > 1) {
    ZStackWatershedContainer container(NULL, NULL);
    foreach (ZStackObject *seed, seedList) {
      container.addSeed(seed);
    }

    switch (range) {
    case flyem::RANGE_SEED:
      container.setRangeHint(ZStackWatershedContainer::RANGE_SEED_ROI);
      break;
    case flyem::RANGE_LOCAL:
      container.setRangeHint(ZStackWatershedContainer::RANGE_SEED_BOUND);
      break;
    case flyem::RANGE_FULL:
      container.setRangeHint(ZStackWatershedContainer::RANGE_FULL);
      break;
    }

    if (range == flyem::RANGE_SEED || range == flyem::RANGE_FULL) {
      ZIntCuboidObj *roi = getSplitRoi();
      if (roi != NULL) {
        ZIntCuboid box;
        roi->boundBox(&box);
        container.setRange(box);
      }
    }

    ZDvidSparseStack *sparseStack =
        getDvidSparseStack(container.getRange(), mode);
    container.setData(NULL, sparseStack->getSparseStack(container.getRange()));

    container.run();

    setHadSegmentationSampled(container.computationDowsampled());
    ZObject3dScanArray result;
    container.makeSplitResult(1, &result);
    for (ZObject3dScanArray::iterator iter = result.begin();
         iter != result.end(); ++iter) {
      ZObject3dScan *obj = *iter;
      getDataBuffer()->addUpdate(
            obj, ZStackDocObjectUpdate::ACTION_ADD_NONUNIQUE);
    }
    getDataBuffer()->deliver();

    result.shallowClear();

    setSegmentationReady(true);
    emit segmentationUpdated();

    ZOUT(LINFO(), 3) << "Segmentation ready";

    emit messageGenerated(
          ZWidgetMessage(
            ZWidgetMessage::appendTime("Split done. Ready to upload.")));
  }
#if 0
//  ZStackArray seedMask = createWatershedMask(true);

//  if (!seedMask.empty()) {
  if (seedList.size() > 1)
//    ZStack *signalStack = getStack();
    ZDvidSparseStack *sparseStack = NULL;
    ZIntCuboid cuboid;
    if (signalStack->isVirtual()) {
      signalStack = NULL;
      ZOUT(LINFO(), 3) << "Retrieving signal stack";
      switch (range) {
      case flyem::RANGE_SEED:
        cuboid = estimateSplitRoi(seedMask);
        break;
      case flyem::RANGE_LOCAL:
        cuboid = estimateLocalSplitRoi();
        break;
      case flyem::RANGE_FULL:
        break;
      }

      sparseStack = getDvidSparseStack(cuboid, mode);
    }

    getProgressSignal()->advanceProgress(0.1);
    ZStackWatershedContainer container(
          signalStack, sparseStack->getSparseStack(cuboid));
    container.setRange(cuboid);
    container.setCcaPost(false);
    for (ZStackArray::const_iterator iter = seedMask.begin();
         iter != seedMask.end(); ++iter) {
      container.addSeed(**iter);
    }
    container.run();

    setHadSegmentationSampled(container.computationDowsampled());

#ifdef _DEBUG_2
//    container.getResultStack()->save(GET_TEST_DATA_DIR + "/test.tif");
    container.exportSource(GET_TEST_DATA_DIR + "/test2.tif");
//    container.exportMask(GET_TEST_DATA_DIR + "/test.tif");
#endif

    ZObject3dScanArray result;
    container.makeSplitResult(1, &result);
    for (ZObject3dScanArray::iterator iter = result.begin();
         iter != result.end(); ++iter) {
      ZObject3dScan *obj = *iter;
      getDataBuffer()->addUpdate(
            obj, ZStackDocObjectUpdate::ACTION_ADD_NONUNIQUE);
    }
    getDataBuffer()->deliver();

    result.shallowClear();

    setSegmentationReady(true);
    emit segmentationUpdated();

    ZOUT(LINFO(), 3) << "Segmentation ready";

    emit messageGenerated(
          ZWidgetMessage(
            ZWidgetMessage::appendTime("Split done. Ready to upload.")));
  }
#endif
  getProgressSignal()->endProgress();
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
//  roi->addVisualEffect(neutube::display::Box::VE_GRID); //For testing
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
  processObjectModified();
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

ZIntCuboid ZFlyEmProofDoc::estimateSplitRoi(const ZStackArray &seedMask)
{
  ZIntCuboid cuboid;

  ZDvidSparseStack *originalStack = ZStackDoc::getDvidSparseStack();
  if (originalStack != NULL) {
    ZIntCuboidObj *roi = getSplitRoi();
    if (roi == NULL) {
      if (originalStack->stackDownsampleRequired()) {
        cuboid = ZFlyEmMisc::EstimateSplitRoi(seedMask.getBoundBox());
      }
    } else {
      cuboid = roi->getCuboid();
    }
  }

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
        cuboid = ZFlyEmMisc::EstimateSplitRoi(seedMask.getBoundBox());
      }
    } else {
      cuboid = roi->getCuboid();
    }
  }

  return cuboid;
}

ZDvidSparseStack* ZFlyEmProofDoc::getDvidSparseStack(
    const ZIntCuboid &roi, flyem::EBodySplitMode mode) const
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

        bool cont = originalStack->prefetching();
        if (mode == flyem::BODY_SPLIT_OFFLINE) {
          cont = false;
        }

        originalStack->runFillValueFunc(roi, true, cont);

        ZDvidInfo dvidInfo = m_grayScaleInfo;

        ZObject3dScan *objMask = m_splitSource->getObjectMask();

#ifdef _DEBUG_2
        objMask->save(GET_TEST_DATA_DIR + "/test3.sobj");
#endif
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
                ZIntPoint(x0, y, z);// - dvidInfo.getStartBlockIndex();
            for (int x = x0; x <= x1; ++x) {
              ZStack *stack =
                  originalStack->getStackGrid()->getStack(blockIndex);
              if (stack != NULL) {
                m_splitSource->getStackGrid()->consumeStack(
                      blockIndex, stack->clone());
              }
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


ZDvidSparseStack* ZFlyEmProofDoc::getCachedBodyForSplit(uint64_t bodyId) const
{
  const ZDvidSparseStack *body = getBodyForSplit();

  ZOUT(LINFO(), 3) << "Get body for split:" << body;

  if (body != NULL) {
    if (body->getLabel() != bodyId) {
      body = NULL;
    }
  }

  return const_cast<ZDvidSparseStack*>(body);
}

void ZFlyEmProofDoc::deprecateSplitSource()
{
  m_splitSource.reset();
}

void ZFlyEmProofDoc::prepareNameBodyMap(const ZJsonValue &bodyInfoObj)
{
  ZSharedPointer<ZFlyEmNameBodyColorScheme> colorMap =
      getColorScheme<ZFlyEmNameBodyColorScheme>(
        ZFlyEmBodyColorOption::BODY_COLOR_NAME);
  if (colorMap.get() != NULL) {
    colorMap->prepareNameMap(bodyInfoObj);

    emit bodyMapReady();
  }
}

void ZFlyEmProofDoc::updateSequencerBodyMap(
    const ZFlyEmSequencerColorScheme &colorScheme,
    ZFlyEmBodyColorOption::EColorOption option)
{
  ZSharedPointer<ZFlyEmSequencerColorScheme> colorMap =
      getColorScheme<ZFlyEmSequencerColorScheme>(option);
  if (colorMap.get() != NULL) {
    *(colorMap.get()) = colorScheme;
    if (isActive(option)) {
      updateBodyColor(option);
    }
  }
}

void ZFlyEmProofDoc::updateSequencerBodyMap(
    const ZFlyEmSequencerColorScheme &colorScheme)
{
  updateSequencerBodyMap(colorScheme,
                         ZFlyEmBodyColorOption::BODY_COLOR_SEQUENCER);
}

void ZFlyEmProofDoc::updateProtocolColorMap(
    const ZFlyEmSequencerColorScheme &colorScheme)
{
  updateSequencerBodyMap(colorScheme,
                         ZFlyEmBodyColorOption::BODY_COLOR_PROTOCOL);
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
    processObjectModified();
  }
}
#endif

void ZFlyEmProofDoc::updateBodyColor(
    ZSharedPointer<ZFlyEmBodyColorScheme> colorMap)
{
  QList<ZDvidLabelSlice*> sliceList = getDvidLabelSliceList();
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
  for (QList<ZDvidLabelSlice*>::iterator iter = sliceList.begin();
       iter != sliceList.end(); ++iter) {
    ZDvidLabelSlice *slice = *iter;
    if (colorMap.get() != NULL) {
      colorMap->update();
      slice->setCustomColorMap(colorMap);
    } else {
      slice->removeCustomColorMap();
    }
    slice->paintBuffer();

    processObjectModified(slice);
  }
  endObjectModifiedMode();
  processObjectModified();
}

void ZFlyEmProofDoc::updateBodyColor(ZFlyEmBodyColorOption::EColorOption type)
{
  ZSharedPointer<ZFlyEmBodyColorScheme> colorMap = getColorScheme(type);
  updateBodyColor(colorMap);
}

void ZFlyEmProofDoc::selectBody(uint64_t bodyId)
{
  QList<ZDvidLabelSlice*> sliceList = getDvidLabelSliceList();
//  ZDvidLabelSlice *slice = getDvidLabelSlice();
  for (QList<ZDvidLabelSlice*>::iterator iter = sliceList.begin();
       iter != sliceList.end(); ++iter) {
    ZDvidLabelSlice *slice = *iter;
    slice->addSelection(bodyId, neutube::BODY_LABEL_MAPPED);
  }
}

void ZFlyEmProofDoc::deselectBody(uint64_t bodyId)
{
  QList<ZDvidLabelSlice*> sliceList = getDvidLabelSliceList();
  for (QList<ZDvidLabelSlice*>::iterator iter = sliceList.begin();
       iter != sliceList.end(); ++iter) {
    ZDvidLabelSlice *slice = *iter;
    slice->removeSelection(bodyId, neutube::BODY_LABEL_MAPPED);
  }
}

void ZFlyEmProofDoc::selectBodyInRoi(int z, bool appending, bool removingRoi)
{
  ZRect2d rect = getRect2dRoi();

  if (rect.isValid()) {
    ZDvidReader &reader = getDvidReader();
    if (reader.good()) {
      std::set<uint64_t> bodySet = reader.readBodyId(
            rect.getFirstX(), rect.getFirstY(), z,
            rect.getWidth(), rect.getHeight(), 1);
      if (appending) {
        addSelectedBody(bodySet, neutube::BODY_LABEL_ORIGINAL);
      } else {
        setSelectedBody(bodySet, neutube::BODY_LABEL_ORIGINAL);
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
          box, getSelectedBodySet(neutube::BODY_LABEL_ORIGINAL));
    if (getDvidWriter().getStatusCode() != 200) {
      emit messageGenerated(
            ZWidgetMessage("Failed to rewite segmentations.", neutube::MSG_ERROR));
    }
  }
}

ZSharedPointer<ZFlyEmBodyColorScheme>
ZFlyEmProofDoc::getColorScheme(ZFlyEmBodyColorOption::EColorOption type)
{
  if (!m_colorMapConfig.contains(type)) {
    switch (type) {
    case ZFlyEmBodyColorOption::BODY_COLOR_NORMAL:
      m_colorMapConfig[type] = ZSharedPointer<ZFlyEmBodyColorScheme>();
      break;
    case ZFlyEmBodyColorOption::BODY_COLOR_NAME:
    {
      ZFlyEmNameBodyColorScheme *colorScheme = new ZFlyEmNameBodyColorScheme;
      colorScheme->setDvidTarget(getDvidTarget());
      m_colorMapConfig[type] =
          ZSharedPointer<ZFlyEmBodyColorScheme>(colorScheme);
    }
      break;
    case ZFlyEmBodyColorOption::BODY_COLOR_SEQUENCER:
    case ZFlyEmBodyColorOption::BODY_COLOR_PROTOCOL:
      m_colorMapConfig[type] =
          ZSharedPointer<ZFlyEmBodyColorScheme>(new ZFlyEmSequencerColorScheme);
      break;
    }
  }

  return m_colorMapConfig[type];
}

template <typename T>
ZSharedPointer<T> ZFlyEmProofDoc::getColorScheme(
    ZFlyEmBodyColorOption::EColorOption type)
{
  ZSharedPointer<ZFlyEmBodyColorScheme> colorScheme = getColorScheme(type);
  if (colorScheme.get() != NULL) {
    return Shared_Dynamic_Cast<T>(colorScheme);
  }

  return ZSharedPointer<T>();
}

void ZFlyEmProofDoc::activateBodyColorMap(const QString &colorMapName)
{
  activateBodyColorMap(ZFlyEmBodyColorOption::GetColorOption(colorMapName));
}

void ZFlyEmProofDoc::activateBodyColorMap(
    ZFlyEmBodyColorOption::EColorOption option)
{
  if (!isActive(option)) {
    updateBodyColor(option);
    m_activeBodyColorMap = getColorScheme(option);
    emit bodyColorUpdated(this);
  }
}

bool ZFlyEmProofDoc::isActive(ZFlyEmBodyColorOption::EColorOption type)
{
  return m_activeBodyColorMap.get() == getColorScheme(type).get();
}

void ZFlyEmProofDoc::recordBodySelection()
{
  ZDvidLabelSlice *slice = getDvidLabelSlice(neutube::Z_AXIS);
  if (slice != NULL) {
    slice->recordSelection();
  }
}

void ZFlyEmProofDoc::processBodySelection()
{
  ZDvidLabelSlice *slice = getDvidLabelSlice(neutube::Z_AXIS);
  if (slice != NULL) {
    slice->processSelection();
  }
}

void ZFlyEmProofDoc::syncBodySelection(ZDvidLabelSlice *labelSlice)
{
  ZOUT(LTRACE(), 5) << "Sync dvid label selection";
  QList<ZDvidLabelSlice*> sliceList = getObjectList<ZDvidLabelSlice>();
  for (QList<ZDvidLabelSlice*>::iterator iter = sliceList.begin();
       iter != sliceList.end(); ++iter) {
    ZDvidLabelSlice *buddySlice = *iter;
    if (buddySlice != labelSlice) {
      const std::set<uint64_t> &selectedSet =
          labelSlice->getSelectedOriginal();
      buddySlice->setSelection(selectedSet, neutube::BODY_LABEL_ORIGINAL);
    }
  }
}

void ZFlyEmProofDoc::executeUnlinkSynapseCommand()
{
  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(neutube::Z_AXIS);
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

  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(neutube::Z_AXIS);
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
  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(neutube::Z_AXIS);
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


  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(neutube::Z_AXIS);
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
  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(neutube::Z_AXIS);
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
                this, synapse.getPosition(), presyn.getPosition(),
                ZDvidSynapse::Relation::GetName(
                  ZDvidSynapse::Relation::RELATION_POSTSYN_TO), command);
          new ZStackDocCommand::DvidSynapseEdit::LinkSynapse(
                this, presyn.getPosition(), synapse.getPosition(),
                ZDvidSynapse::Relation::GetName(
                  ZDvidSynapse::Relation::RELATION_PRESYN_TO), command);
        }
      }
    }
    pushUndoCommand(command);
  } else {
    emit messageGenerated(
          ZWidgetMessage(
            "Failed to add synapse. Have you specified the synapse data name?",
            neutube::MSG_WARNING));
  }
}

void ZFlyEmProofDoc::executeMoveSynapseCommand(const ZIntPoint &dest)
{
  ZDvidSynapseEnsemble *se = getDvidSynapseEnsemble(neutube::Z_AXIS);
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
      processObjectModified();
      emit userBookmarkModified();
    }
  }
}

void ZFlyEmProofDoc::executeAddTodoCommand(
    int x, int y, int z, bool checked, neutube::EToDoAction action,
    uint64_t bodyId)
{
  ZIntPoint position = getDvidReader().readPosition(bodyId, x, y, z);

  if (position.isValid()) {
    ZFlyEmToDoItem item(position);
    item.setUserName(neutube::GetCurrentUserName());
    item.setAction(action);
    item.setChecked(checked);

    executeAddTodoItemCommand(item);
  }
}

void ZFlyEmProofDoc::executeAddTodoItemCommand(
    int x, int y, int z, bool checked, uint64_t bodyId)
{
  executeAddTodoItemCommand(ZIntPoint(x, y, z), checked, bodyId);
}

void ZFlyEmProofDoc::executeAddTodoItemCommand(
    const ZIntPoint &pt, bool checked, uint64_t bodyId)
{
  ZIntPoint position = getDvidReader().readPosition(bodyId, pt);

  if (position.isValid()) {
    ZFlyEmToDoItem item(position);
    item.setUserName(neutube::GetCurrentUserName());
    if (checked) {
      item.setChecked(checked);
    }

    executeAddTodoItemCommand(item);
  }
}

void ZFlyEmProofDoc::executeAddTodoItemCommand(
    int x, int y, int z, neutube::EToDoAction action, uint64_t bodyId)
{
  executeAddTodoCommand(x, y, z, false, action, bodyId);
}

void ZFlyEmProofDoc::executeAddToMergeItemCommand(int x, int y, int z, uint64_t bodyId)
{
  executeAddTodoItemCommand(x, y, z, neutube::TO_MERGE, bodyId);
}

void ZFlyEmProofDoc::executeAddToMergeItemCommand(const ZIntPoint &pt, uint64_t bodyId)
{
  executeAddToMergeItemCommand(pt.getX(), pt.getY(), pt.getZ(), bodyId);
}

void ZFlyEmProofDoc::executeAddToSplitItemCommand(int x, int y, int z, uint64_t bodyId)
{
  executeAddTodoItemCommand(x, y, z, neutube::TO_SPLIT, bodyId);
}

void ZFlyEmProofDoc::executeAddToSplitItemCommand(const ZIntPoint &pt, uint64_t bodyId)
{
  executeAddToSplitItemCommand(pt.getX(), pt.getY(), pt.getZ(), bodyId);
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

void ZFlyEmProofDoc::executeRemoveTodoCommand()
{
  executeRemoveTodoItemCommand();
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
    processObjectModified();
  }
}

void ZFlyEmProofDoc::executeRotateRoiPlaneCommand(int z, double theta)
{
  if (theta != 0) {
    QUndoCommand *allCommand = new QUndoCommand;

    QList<ZSwcTree*> treeList = getSwcList(ZStackObjectRole::ROLE_ROI);

    foreach (ZSwcTree *tree, treeList) {
      std::vector<Swc_Tree_Node*> subNodeList =  tree->getNodeOnPlane(z);
      if (!subNodeList.empty()) {
        ZStackDocCommand::SwcEdit::RotateSwcNodeAroundZ *command =
            new ZStackDocCommand::SwcEdit::RotateSwcNodeAroundZ(this, allCommand);
        command->addNode(subNodeList);
        command->useNodeCentroid();
        command->setRotateAngle(theta);
      }
    }
    if (allCommand->childCount() > 0) {
      beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
      pushUndoCommand(allCommand);
      endObjectModifiedMode();
      processObjectModified();
    } else {
      delete allCommand;
    }
  }
}

void ZFlyEmProofDoc::executeScaleRoiPlaneCommand(int z, double sx, double sy)
{
  if ((sx != 1.0 || sy != 1.0) && sx > 0.0 && sy > 0.0) {
    QUndoCommand *allCommand = new QUndoCommand;

    QList<ZSwcTree*> treeList = getSwcList(ZStackObjectRole::ROLE_ROI);

    foreach (ZSwcTree *tree, treeList) {
      std::vector<Swc_Tree_Node*> subNodeList =  tree->getNodeOnPlane(z);
      if (!subNodeList.empty()) {
        ZStackDocCommand::SwcEdit::ScaleSwcNodeAroundZ *command =
            new ZStackDocCommand::SwcEdit::ScaleSwcNodeAroundZ(this, allCommand);
        command->addNode(subNodeList);
        command->setScale(sx, sy);
      }
    }

    if (allCommand->childCount() > 0) {
      beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
      pushUndoCommand(allCommand);
      endObjectModifiedMode();
      processObjectModified();
    } else {
      delete allCommand;
    }
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
  processObjectModified();
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

void ZFlyEmProofDoc::diagnose() const
{
  ZStackDoc::diagnose();

  LDEBUG() << "#Selected bodies (unmapped):"
           << getSelectedBodySet(neutube::BODY_LABEL_ORIGINAL).size();
}


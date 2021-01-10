#include "zflyembodymergeproject.h"

#include <QtConcurrentRun>
#include <QApplication>
#include <QItemSelectionModel>
#include <QDesktopWidget>

#include "logging/zlog.h"

#include "geometry/zintpoint.h"
#include "zstackdvidgrayscalefactory.h"

#include "zflyemproofdoc.h"
#include "mvc/zstackframe.h"
#include "mvc/zstackdoc.h"
#include "mvc/zstackpresenter.h"
#include "mvc/zstackmvc.h"

#include "zflyembodymergeframe.h"
#include "zflyembodymergedoc.h"
#include "neutubeconfig.h"
#include "zstackdocreader.h"
#include "zarrayfactory.h"
#include "dvid/zdviddata.h"
#include "zobject3dscan.h"
#include "dvid/zdvidwriter.h"
#include "zflyemdvidreader.h"
#include "zswcgenerator.h"
#include "zwindowfactory.h"
#include "z3dswcfilter.h"
#include "zstackobjectsourcefactory.h"


#include "dvid/zdvidsparsevolslice.h"
#include "dvid/zdvidlabelslice.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidbufferreader.h"
#include "dvid/zdvidurl.h"

#include "zwidgetmessage.h"
#include "z3dgraphfactory.h"
#include "mvc/zstackdochelper.h"
#include "z3dgraphfilter.h"
#include "zflyemmisc.h"
#include "zprogresssignal.h"
#include "zdialogfactory.h"
#include "flyem/zflyembookmark.h"
#include "zsleeper.h"
#include "flyem/zflyembody3ddoc.h"
#include "mvc/zstackview.h"
#include "widgets/z3dtabwidget.h"
#include "zdvidutil.h"
#include "neutuse/task.h"
#include "neutuse/taskfactory.h"
#include "zdialogfactory.h"
#include "zstring.h"
#include "zpunctum.h"
#include "flyemdatareader.h"
#include "flyemdatawriter.h"

#ifdef _WIN32
#undef GetUserName
#endif

ZFlyEmBodyMergeProject::ZFlyEmBodyMergeProject(QObject *parent) :
  QObject(parent), m_dataFrame(NULL),
//  m_bodyViewWindow(NULL),
//  m_bodyViewers(NULL),
//  m_coarseBodyWindow(NULL),
//  m_bodyWindow(NULL),
  m_isBookmarkVisible(true),
//  m_bookmarkArray(NULL),
  m_showingBodyMask(true)
{
  m_progressSignal = new ZProgressSignal(this);

  connectSignalSlot();
}

ZFlyEmBodyMergeProject::~ZFlyEmBodyMergeProject()
{
  clear();
}

void ZFlyEmBodyMergeProject::clear()
{
  if (m_dataFrame != NULL) {
    m_dataFrame->hide();
    delete m_dataFrame;
    m_dataFrame = NULL;
  }

#if 0
  if (m_coarseBodyWindow != NULL) {
    m_coarseBodyWindow->hide();
    delete m_coarseBodyWindow;
    m_coarseBodyWindow = NULL;
  }

  if (m_bodyWindow != NULL) {
    m_bodyWindow->hide();
    delete m_bodyWindow;
    m_bodyWindow = NULL;
  }
#endif

  m_selectedOriginal.clear();
}

void ZFlyEmBodyMergeProject::connectSignalSlot()
{
//  connect(this, SIGNAL(coarseBodyWindowCreatedInThread()),
//          this, SLOT(presentCoarseBodyView()));
}

ZProgressSignal* ZFlyEmBodyMergeProject::getProgressSignal() const
{
  return m_progressSignal;
}

int ZFlyEmBodyMergeProject::getCurrentZ() const
{
  int z = 0;

  if (getDocument() != NULL) {
    getDocument()->getStackOffset().getZ();
  }

  return z;
}

void ZFlyEmBodyMergeProject::test()
{
#if 0
  ZStackDocReader *reader = new ZStackDocReader();
  reader->loadStack(
        (GET_TEST_DATA_DIR + "/benchmark/em_stack_slice.tif").c_str());

  if (m_showingBodyMask) {
    emit newDocReady(reader, false);
    ZStack stack;
    stack.load(GET_TEST_DATA_DIR + "/benchmark/em_stack_slice_seg.tif");
    ZArray *array = ZArrayFactory::MakeArray(&stack);
    emit originalLabelUpdated(array, &m_selectedOriginal);
  } else {
    emit newDocReady(reader, true);
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "b6bc", 8500);
  target.setSegmentationName("labels");
  setDvidTarget(target);

  uint64_t targetId = 12532906;
  std::vector<uint64_t> merged;
  merged.push_back(12767166);
  merged.push_back(1);

  std::cout << "Target: " << getTargetId(targetId, merged, false) << std::endl;

#endif
}

void ZFlyEmBodyMergeProject::changeDvidNode(const std::string &newUuid)
{
  ZDvidTarget target = getDvidReader().getDvidTarget();
  target.setUuid(newUuid);
//  m_reader.open(target);
  m_writer.open(target);
  m_selectedOriginal.clear();
}

void ZFlyEmBodyMergeProject::loadSlice(
    int x, int y, int z, int width, int height)
{
  QtConcurrent::run(
          this, &ZFlyEmBodyMergeProject::loadSliceFunc, x, y, z, width, height);
}

void ZFlyEmBodyMergeProject::loadSliceFunc(
    int x, int y, int z, int width, int height)
{
  const ZDvidReader &reader = getDvidReader();
  if (reader.good()) {
    //int width = 512;
    //int height = 512;

    int x0 = x - width / 2;
    int y0 = y - height / 2;

    ZStackDocReader *docReader = new ZStackDocReader;

    ZStack *stack = reader.readGrayScale(x0, y0, z, width, height, 1);
    docReader->setStack(stack);

    if (m_showingBodyMask) {
      emit newDocReady(docReader, false);

#ifdef _DEBUG_2
      ZArray *array = reader.readLabels64(
            ZDvidData::getName(ZDvidData::ROLE_MERGE_TEST_BODY_LABEL),
            x0, y0, z, width, height, 1);
#else
      ZArray *array = reader.readLabels64(
            x0, y0, z, width, height, 1);
#endif
      emit originalLabelUpdated(array, &m_selectedOriginal);
    } else {
      emit newDocReady(docReader, true);
    }
  }
}

void ZFlyEmBodyMergeProject::viewGrayscale(
    const ZIntPoint &offset, int width, int height)
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    ZStackFrame *frame = getDataFrame();
    if (frame != NULL) {
      ZStack *stack = reader.readGrayScale(
            offset.getX(), offset.getY(), offset.getZ(), width, height, 1);
      frame->loadStack(stack);
    }
  }
}

void ZFlyEmBodyMergeProject::loadGrayscaleFunc(int /*z*/, bool /*lowres*/)
{
#if 0
  emit progressAdvanced(0.1);
  ZDvidReader reader;
  const ZDvidTarget &target = m_project->getDvidTarget();
  if (z >= 0 && reader.open(target)) {
    if (m_project->getRoi(z) == NULL) {
      m_project->downloadRoi(z);
    }

    QString infoString = reader.readInfo("grayscale");

    qDebug() << infoString;

    ZDvidInfo dvidInfo;
    dvidInfo.setFromJsonString(infoString.toStdString());

    //int z = m_zDlg->getValue();
    //m_project->setZ(z);

    ZStack *stack = NULL;

    bool creatingLowres = false;
    std::string lowresPath =
        target.getLocalLowResGrayScalePath(m_xintv, m_xintv, 0, z);
    if (lowres) {
      m_project->setDsIntv(m_xintv, m_yintv, 0);
      prepareQuickLoad(z, true);
      if (QFileInfo(lowresPath.c_str()).exists() && !isPreparingQuickLoad(z)) {
        stack = new ZStack;
        stack->load(lowresPath);
      } else if (QDir(target.getLocalFolder().c_str()).exists()) {
        creatingLowres = true;
      }
    }

    if (!lowres || creatingLowres){
      if (!lowres) {
        m_project->setDsIntv(0, 0, 0);
      }

      ZIntCuboid boundBox = reader.readBoundBox(z);

      if (!boundBox.isEmpty()) {
        stack = reader.readGrayScale(boundBox.getFirstCorner().getX(),
                                     boundBox.getFirstCorner().getY(),
                                     z, boundBox.getWidth(),
                                     boundBox.getHeight(), 1);
      } else {
        stack = reader.readGrayScale(
              dvidInfo.getStartCoordinates().getX(),
              dvidInfo.getStartCoordinates().getY(),
              z, dvidInfo.getStackSize()[0],
            dvidInfo.getStackSize()[1], 1);
        if (stack != NULL) {
          boundBox = ZFlyEmRoiProject::estimateBoundBox(
                *stack, getDvidTarget().getBgValue());
          if (!boundBox.isEmpty()) {
            stack->crop(boundBox);
          }
          ZDvidWriter writer;
          if (writer.open(m_project->getDvidTarget())) {
            writer.writeBoundBox(boundBox, z);
          }
        }
      }

      if (stack != NULL) {
        if (creatingLowres) {
          emit messageDumped("Creating low res data ...", true);
          stack->downsampleMin(m_xintv, m_xintv, 0);
          stack->save(lowresPath);
          emit messageDumped(
                QString("Data saved into %1").arg(lowresPath.c_str()), true);
        }
      }
    }

    if (stack != NULL) {
      //advance(0.5);
      emit progressAdvanced(0.5);
      m_docReader.clear();
      m_docReader.setStack(stack);

      ZSwcTree *tree = m_project->getRoiSwc(
            z, FlyEm::GetFlyEmRoiMarkerRadius(stack->width(), stack->height()));
      if (tree != NULL) {
        m_docReader.addObject(tree, ZDocPlayer::ROLE_ROI);
      }
#ifdef _DEBUG_
      std::cout << "Object count in docreader: "
                << m_docReader.getObjectGroup().size() << std::endl;
      std::cout << "Swc count in docreader: "
                << m_docReader.getObjectGroup().getObjectList(ZStackObject::EType::TYPE_SWC).size()
                << std::endl;
#endif
      emit newDocReady();
    } else {
      processLoadGrayscaleFailure();
    }
  } else {
    processLoadGrayscaleFailure();
  }
#endif
}

bool ZFlyEmBodyMergeProject::hasDataFrame() const
{
  return m_dataFrame != NULL;
}

void ZFlyEmBodyMergeProject::setDocData(ZStackDocReader &reader)
{
  if (m_dataFrame != NULL) {
//    TStackObjectList objList = m_dataFrame->document()->getObjectGroup().take(
//          ZStackObject::EType::TYPE_OBJECT3D_SCAN);
#ifdef _DEBUG_
//    std::cout << objList.size() << " objects taken" << std::endl;
#endif
    m_dataFrame->document()->reloadData(reader);
//    m_dataFrame->document()->getObjectGroup().add(
//          objList.begin(), objList.end(), false);
#ifdef _DEBUG_
//    std::cout <<
#endif
  }
}

void ZFlyEmBodyMergeProject::setDataFrame(ZStackFrame *frame)
{
  if (m_dataFrame != NULL) {
    frame->setObjectStyle(m_dataFrame->getObjectStyle());
    closeDataFrame();
  }

  connect(frame, SIGNAL(destroyed()), this, SLOT(shallowClear()));

  m_dataFrame = qobject_cast<ZFlyEmBodyMergeFrame*>(frame);

  connect(this, SIGNAL(originalLabelUpdated(ZArray*, QSet<uint64_t>*)),
          m_dataFrame->getCompleteDocument(),
          SLOT(updateOriginalLabel(ZArray*, QSet<uint64_t>*)));

  connect(m_dataFrame->getCompleteDocument(),
          SIGNAL(objectSelectorChanged(ZStackObjectSelector)),
          this, SIGNAL(selectionChanged(ZStackObjectSelector)));

  connect(m_dataFrame->presenter(), SIGNAL(bodySplitTriggered()),
          this, SLOT(notifySplit()));

//  connect(this, SIGNAL(selectionChanged(ZStackObjectSelector)),
//          this, SLOT(update3DBodyView(ZStackObjectSelector)));
//  connect(this, SIGNAL(selectionChanged()), this, SLOT(update3DBodyView()));
  //connect(this, SIGNAL())
}

void ZFlyEmBodyMergeProject::closeDataFrame()
{
  if (m_dataFrame != NULL) {
    m_dataFrame->hide();
    delete m_dataFrame;
    m_dataFrame = NULL;
  }
}

void ZFlyEmBodyMergeProject::shallowClear()
{
  m_dataFrame = NULL;
}

void ZFlyEmBodyMergeProject::mergeBody()
{
  if (m_dataFrame != NULL) {
    QList<uint64_t> objLabelList;
    const QList<ZObject3dScan*> &objList = m_dataFrame->getCompleteDocument()->
        getSelectedObjectList<ZObject3dScan>(ZStackObject::EType::OBJECT3D_SCAN);
    for (QList<ZObject3dScan*>::const_iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      const ZObject3dScan *obj = *iter;
      objLabelList.append(obj->getLabel());
    }
    m_dataFrame->getCompleteDocument()->mergeSelected();

    emit bodyMerged(objLabelList);
  }
}

void ZFlyEmBodyMergeProject::setLoadingLabel(bool state)
{
  m_showingBodyMask = state;
}

void ZFlyEmBodyMergeProject::saveMergeOperation()
{
  ZFlyEmBodyMergeDoc *doc = getDocument<ZFlyEmBodyMergeDoc>();
  if (doc != NULL) {
    doc->saveMergeOperation();
  } else {
    ZFlyEmProofDoc *doc2 = getDocument<ZFlyEmProofDoc>();
    if (doc2 != NULL) {
      doc2->saveMergeOperation();
    }
  }
  ZOUT(LTRACE(), 5) << "Merge operation saved.";
}

/*
void ZFlyEmBodyMergeProject::clearUndoStack()
{
  ZFlyEmBodyMergeDoc *doc = getDocument<ZFlyEmBodyMergeDoc>();
  if (doc != NULL) {
    doc->undoStack()->clear();
  } else {
    ZFlyEmProofDoc *doc2 = getDocument<ZFlyEmProofDoc>();
    if (doc2 != NULL) {
      doc2->undoStack()->clear();
    }
  }
//  m_mergeMap.clear();
//  m_annotationCache.clear();
}
*/

void ZFlyEmBodyMergeProject::clearBodyMerger()
{
  ZFlyEmBodyMergeDoc *doc = getDocument<ZFlyEmBodyMergeDoc>();
  if (doc != NULL) {
    doc->clearBodyMerger();
  } else {
    ZFlyEmProofDoc *doc2 = getDocument<ZFlyEmProofDoc>();
    if (doc2 != NULL) {
      doc2->clearBodyMerger();
    }
  }
//  m_mergeMap.clear();
//  m_annotationCache.clear();
}

QList<QString> ZFlyEmBodyMergeProject::getBodyStatusList(
    std::function<bool(const ZFlyEmBodyStatus&)> pred) const
{
  const std::vector<ZFlyEmBodyStatus> &bodyStatusList =
      m_bodyStatusProtocol.getStatusList();

  QList<QString> statusList;
  for (const ZFlyEmBodyStatus &status : bodyStatusList) {
    if (pred(status)) {
      statusList.append(status.getName().c_str());
    }
  }

  return statusList;
}

QList<QString> ZFlyEmBodyMergeProject::getBodyStatusList() const
{
  const std::vector<ZFlyEmBodyStatus> &bodyStatusList =
      m_bodyStatusProtocol.getStatusList();

  QList<QString> statusList;
  for (const ZFlyEmBodyStatus &status : bodyStatusList) {
    if (status.isAccessible(m_isAdmin)) {
      statusList.append(status.getName().c_str());
    }
  }

  return statusList;
}

QList<QString> ZFlyEmBodyMergeProject::getAdminStatusList() const
{
  return getBodyStatusList([](const ZFlyEmBodyStatus &status) {
    return status.isAdminAccessible();
  });

//  const std::vector<ZFlyEmBodyStatus> &bodyStatusList =
//      m_annotMerger.getStatusList();

//  QList<QString> statusList;
//  for (const ZFlyEmBodyStatus &status : bodyStatusList) {
//    if (status.isAdminAccessible()) {
//      statusList.append(status.getName().c_str());
//    }
//  }

//  return statusList;
}

int ZFlyEmBodyMergeProject::getStatusRank(const std::string &status) const
{
  if (!m_bodyStatusProtocol.isEmpty()) {
    return m_bodyStatusProtocol.getStatusRank(status);
  }

  return ZFlyEmBodyAnnotation::GetStatusRank(status);
}

bool ZFlyEmBodyMergeProject::isFinalStatus(const std::string &status) const
{
  if (m_bodyStatusProtocol.isEmpty()) {
    return ZString(status).lower() == "finalized";
  }

  return m_bodyStatusProtocol.isFinal(status);
}

bool ZFlyEmBodyMergeProject::isExpertStatus(const std::string &status) const
{
  return m_bodyStatusProtocol.isExpertStatus(status);
}

bool ZFlyEmBodyMergeProject::isMergableStatus(const std::string &status) const
{
  return m_bodyStatusProtocol.isMergable(status);
}

bool ZFlyEmBodyMergeProject::preservingId(const std::string &status) const
{
  return m_bodyStatusProtocol.preservingId(status);
}

namespace {

QString compose_body_status_message(
    const std::vector<uint64_t> &bodyArray,
    const QMap<uint64_t, ZFlyEmBodyAnnotation> &annotMap, int &itemCount)
{
  QString msg;
  if (!bodyArray.empty()) {
    msg += "<ul>";
    for (uint64_t bodyId : bodyArray) {
      msg += QString("<li>%1: %2</li>").
          arg(bodyId).arg(annotMap[bodyId].getStatus().c_str());
      ++itemCount;
      if (itemCount >= 5) {
        msg += "<li>...</li>";
        break;
      }
    }
    msg += "</ul>";
    msg += "<br>";
  }

  return msg;
}

}

QString ZFlyEmBodyMergeProject::composeFinalStatusMessage(
    const QMap<uint64_t, ZFlyEmBodyAnnotation> &annotMap) const
{
  std::vector<uint64_t> bodyArray;
  for (auto iter = annotMap.constBegin(); iter != annotMap.constEnd(); ++iter) {
    if (isFinalStatus(iter.value().getStatus())) {
      bodyArray.push_back(iter.key());
    }
  }

  int itemCount = 0;
  QString msg = compose_body_status_message(bodyArray, annotMap, itemCount);

  if (!msg.isEmpty()) {
    msg = "The following bodies have final statuses: " + msg +
        "<font color=\"#FF0000\">You should NOT merge them "
        "unless you want to be resposible for any side effects.</font>";
  }

  return msg;
}

QString ZFlyEmBodyMergeProject::composeStatusConflictMessage(
    const QMap<uint64_t, ZFlyEmBodyAnnotation> &annotMap) const
{
  QString msg;
  const std::vector<std::vector<uint64_t>> &conflictSet =
      m_bodyStatusProtocol.getConflictBody(annotMap);
  int itemCount = 0;
  for (const std::vector<uint64_t> &bodyArray : conflictSet) {
    msg += compose_body_status_message(bodyArray, annotMap, itemCount);
    if (itemCount >= 5) {
      break;
    }
  }

  if (!msg.isEmpty()) {
    msg = "The following bodies have conflicting statuses: " + msg +
        "<font color=\"#FF0000\">You should NOT merge them "
        "unless you want to be resposible for any side effects.</font>";
  }

  return msg;
}

void ZFlyEmBodyMergeProject::uploadMeshMerge(
    uint64_t targetId, const std::vector<uint64_t> &bodyIdArray)
{
  /*
  const ZDvidReader &reader = m_writer.getDvidReader();
  if (reader.isReady()) {
    std::vector<uint64_t> meshBodyArray = reader.readMergedMeshKeys(targetId);
    if (meshBodyArray.empty()) {
      meshBodyArray.push_back(targetId);
    }
    for (uint64_t bodyId : bodyIdArray) {
      std::vector<uint64_t> subArray = reader.readMergedMeshKeys(bodyId);
      if (subArray.empty()) {
        meshBodyArray.push_back(bodyId);
      } else {
        meshBodyArray.insert(
              meshBodyArray.end(), subArray.begin(), subArray.end());
      }
    }
      }
    */

  FlyEmDataWriter::WriteMeshMerge(m_writer, targetId, bodyIdArray);

}

void ZFlyEmBodyMergeProject::mergeBodyAnnotation(
    uint64_t targetId, const std::vector<uint64_t> &bodyIdArray)
{
  const ZDvidReader &reader = m_writer.getDvidReader();
  if (reader.isReady()) {
    ZFlyEmBodyAnnotation annotation = m_annotationCache[targetId];
    for (uint64_t bodyId : bodyIdArray) {
      if (bodyId != targetId) {
        if (m_annotationCache.contains(bodyId)) {
          ZFlyEmBodyAnnotation subann = m_annotationCache[bodyId];
          if (m_bodyStatusProtocol.isEmpty()) {
            annotation.mergeAnnotation(
                  subann, &ZFlyEmBodyAnnotation::GetStatusRank);
          } else {
            annotation.mergeAnnotation(
                  subann, [=](const std::string &status) {
              return m_bodyStatusProtocol.getStatusRank(status);
            });
          }
        }
      }
    }

    if (annotation.getStatus().empty()) {
//      annotation.setStatus("Not examined");
    }

    if (!annotation.isEmpty()) {
      if (m_writer.good()) {
        m_writer.writeBodyAnntation(annotation);
      }
    }
  }
}

void ZFlyEmBodyMergeProject::updateMergeMap()
{
  m_mergeMap.clear();
  ZFlyEmBodyMerger *bodyMerger = getBodyMerger();
  if (bodyMerger) {
    ZFlyEmBodyMerger::TLabelMap labelMap = bodyMerger->getFinalMap();
    //reorganize the map
    foreach (uint64_t sourceId, labelMap.keys()) {
      uint64_t targetId = labelMap.value(sourceId);
      if (m_mergeMap.contains(targetId)) {
        std::vector<uint64_t> &idArray = m_mergeMap[targetId];
        idArray.push_back(sourceId);
      } else {
        m_mergeMap[targetId] = std::vector<uint64_t>();
        m_mergeMap[targetId].push_back(sourceId);
      }
    }
  }
}

void ZFlyEmBodyMergeProject::refreshBodyAnnotationCache()
{
  m_annotationCache.clear();
  const ZDvidReader &reader = m_writer.getDvidReader();
  if (reader.isReady()) {
    foreach (uint64_t targetId, m_mergeMap.keys()) {
      std::vector<uint64_t> idArray = m_mergeMap[targetId];
      idArray.push_back(targetId);
      for (uint64_t bodyId : idArray) {
        if (!m_annotationCache.contains(bodyId)) {
          m_annotationCache[bodyId] =
              FlyEmDataReader::ReadBodyAnnotation(reader, bodyId);
        }
      }
    }
  }
}

void ZFlyEmBodyMergeProject::updateAffliatedData(
    uint64_t targetId, const std::vector<uint64_t> &bodyArray,
    ZWidgetMessage &warnMsg)
{
  for (uint64_t bodyId : bodyArray) {
    m_writer.deleteBodyAnnotation(bodyId);
    m_writer.deleteSkeleton(bodyId);
  }

  if (GET_FLYEM_CONFIG.neutuseAvailable(
        neutu::UsingLocalHost(getDvidTarget().getAddress()))) {
    neutuse::Task task = neutuse::TaskFactory::MakeDvidTask(
          "skeletonize", getDvidTarget(), targetId, true);
    task.setPriority(5);
    GET_FLYEM_CONFIG.getNeutuseWriter().uploadTask(task);
    if (GET_FLYEM_CONFIG.getNeutuseWriter().getStatusCode() != 200) {
      warnMsg.setMessage("Failed to upload skeletonization task");
    }
  }

  //Temporary fix for mesh update, which should be moved the remote service
//  m_writer.deleteMesh(targetId);
}

void ZFlyEmBodyMergeProject::updateSelection(const std::set<uint64_t> &newBodySet)
{
  ZFlyEmProofDoc *doc = getDocument<ZFlyEmProofDoc>();
  if (doc) {
    QList<ZDvidLabelSlice*> labelList = doc->getDvidBodySliceList();

    foreach (ZDvidLabelSlice *slice, labelList) {
      slice->setSelection(newBodySet, neutu::ELabelSource::ORIGINAL);
    }

    m_selectedOriginal.clear();

    for (std::set<uint64_t>::const_iterator iter = newBodySet.begin();
         iter != newBodySet.end(); ++iter) {
      m_selectedOriginal.insert(*iter);
    }

    ZOUT(LTRACE(), 5) << "Label slice updated";
  }
}

/*
void ZFlyEmBodyMergeProject::refreshSegmentationView()
{
  ZFlyEmProofDoc *proofDoc = getDocument<ZFlyEmProofDoc>();
  if (proofDoc != NULL) {
    proofDoc->clearBodyForSplit();
    proofDoc->refreshDvidLabelBuffer(2000);
    ZOUT(LTRACE(), 5) << "Label buffer refreshed";
  }
}
*/

void ZFlyEmBodyMergeProject::unlockBody(const std::set<uint64_t> &bodySet)
{
  for (std::set<uint64_t>::const_iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    emit checkingInBody(*iter, neutu::EBodySplitMode::NONE);
  }
}

void ZFlyEmBodyMergeProject::unlockBody(const std::vector<uint64_t> &bodyArray)
{
  for (auto bodyId : bodyArray) {
    unlockBody(bodyId);
  }
}

void ZFlyEmBodyMergeProject::unlockBody(uint64_t bodyId)
{
  emit checkingInBody(bodyId, neutu::EBodySplitMode::NONE);
}

void ZFlyEmBodyMergeProject::removeMerge(uint64_t bodyId)
{
  getBodyMerger()->unmerge(bodyId);
  unlockBody(bodyId);
}

void ZFlyEmBodyMergeProject::removeMerge(const std::vector<uint64_t> &bodyArray)
{
  for (uint64_t bodyId : bodyArray) {
    removeMerge(bodyId);
  }
}

bool ZFlyEmBodyMergeProject::preserved(uint64_t bodyId) const
{
  return (preservingId(m_annotationCache.value(bodyId).getStatus()));
}

namespace {

bool has_name(uint64_t bodyId,
             const QMap<uint64_t, ZFlyEmBodyAnnotation> &annotationCache)
{
  if (annotationCache.contains(bodyId)) {
    if (!annotationCache.value(bodyId).getName().empty()) {
      return true;
    }
  }

  return false;
}

}

bool ZFlyEmBodyMergeProject::hasName(uint64_t bodyId) const
{
  return has_name(bodyId, m_annotationCache);
}

bool ZFlyEmBodyMergeProject::mergeVerified(
    uint64_t targetId, const std::vector<uint64_t> &bodyArray) const
{
  QString msg;

  QString bodyStatusMsg;
  QString bodyNameMsg;
  if (!bodyArray.empty()) {
    if (!hasName(targetId) && hasName(bodyArray.front())) {
      msg += QString("The name of ID %1 will be transfered to ID %2\n").
          arg(bodyArray.front()).arg(targetId);
    }

    for (uint64_t bodyId : bodyArray) {
      if (preserved(bodyId)) {
        bodyStatusMsg += QString("ID: %1, Status: %2;\n").
            arg(bodyId).
            arg(m_annotationCache.value(bodyId).getStatus().c_str());
      }

      if (hasName(bodyId)) {
        bodyNameMsg += QString("ID: %1, Name: %2;\n").
            arg(bodyId).
            arg(m_annotationCache.value(bodyId).getName().c_str());
      }
    }
  }

  bool okToContinue = true;

  if (!bodyStatusMsg.isEmpty()) {
    if (neutu::IsAdminUser()) {
      okToContinue = ZDialogFactory::Ask(
            "Confirming Merge",
            "The following preserved IDs will be changed after merging\n" +
             bodyStatusMsg + "\nDo you want to continue?", nullptr);
    } else {
      ZDialogFactory::Warn(
            "Merge Forbidden",
            "The bodies cannot be merged because the following IDs "
            "need to be preserved:\n" + bodyStatusMsg +
            "\nPlease contact admin if you do want to merge the bodies.",
            nullptr);
      okToContinue = false;
    }
  }

  if (okToContinue) {
    if (!bodyNameMsg.isEmpty()) {
      msg += "The following named bodies will be gone after merging\n"
          + bodyNameMsg;
    }

    if (!msg.isEmpty()) {
      okToContinue = ZDialogFactory::Ask(
            "Confirming Merge", msg + "\nDo you want to continue?", NULL);
    }
  }

  return okToContinue;
}

void ZFlyEmBodyMergeProject::logSynapseInfo(uint64_t bodyId)
{
  if (bodyId > 0 && getDvidReader().getDvidTarget().hasSynapseLabelsz()) {
    int count = getDvidReader().readSynapseLabelszBody(
          bodyId, dvid::ELabelIndexType::ALL_SYN);
    KINFO << QString("Merge target: %1 (#synapses: %2)").arg(bodyId).arg(count);
  }
}

//Assuming m_annotationCache and m_mergeMap are up to date.
void ZFlyEmBodyMergeProject::uploadResultFunc(bool mergingToLargest)
{
//  ZFlyEmBodyMerger *bodyMerger = getBodyMerger();
//  bool anyMergeUploaded = false;

  if (!m_mergeMap.empty() && m_writer.good()) {
    getProgressSignal()->startProgress("Uploading merge result ...");

    ZWidgetMessage warnMsg;
    warnMsg.setType(neutu::EMessageType::WARNING);

    getProgressSignal()->advanceProgress(0.1);

    //      std::set<uint64_t> bodySet = getSelection(neutube::BODY_LABEL_ORIGINAL);
    auto oldSelection = getSelection(neutu::ELabelSource::ORIGINAL);

    std::set<uint64_t> newBodySet;
    getProgressSignal()->startProgress(0.5);
    int targetUploadedCount = 0;
    uint64_t lastTarget = 0;
    foreach (uint64_t targetId, m_mergeMap.keys()) {
      const std::vector<uint64_t> &merged = m_mergeMap.value(targetId);
      auto mergeConfig = dvid::GetMergeConfig(
            m_writer.getDvidReader(), targetId, merged, mergingToLargest);
      const uint64_t &newTargetId = mergeConfig.first;
      const std::vector<uint64_t> &newMerged = mergeConfig.second;

      if (mergeVerified(newTargetId, newMerged)) {
        m_writer.mergeBody(newTargetId, newMerged);

        if (m_writer.getStatusCode() != 200) {
          emit messageGenerated(
                ZWidgetMessage(
                  QString("Failed to upload merging for %1").arg(newTargetId),
                  neutu::EMessageType::ERROR,
                  ZWidgetMessage::TARGET_TEXT_APPENDING |
                  ZWidgetMessage::TARGET_KAFKA));
        } else {
          uploadMeshMerge(newTargetId, newMerged);
          mergeBodyAnnotation(newTargetId, newMerged);

          updateAffliatedData(newTargetId, newMerged, warnMsg);

          if (oldSelection.count(newTargetId) > 0) {
            newBodySet.insert(newTargetId);
          }

          removeMerge(newTargetId);
          removeMerge(newMerged);
          ++targetUploadedCount;
          lastTarget = newTargetId;

          KINFO << QString("%1 bodies are merged into %2")
                   .arg(merged.size()).arg(newTargetId);
          //            anyMergeUploaded = true;
        }
      }
      getProgressSignal()->advanceProgress(0.1);
    }
    getProgressSignal()->endProgress();

    if (targetUploadedCount > 0) {
      updateSelection(newBodySet);

      getProgressSignal()->advanceProgress(0.1);

//      refreshSegmentationView();

      ZOUT(LTRACE(), 5) << "Merge uploaded.";

      emit mergeUploaded();

      getProgressSignal()->advanceProgress(0.1);

//      clearUndoStack();
      //      clearBodyMerger();

      //      ZOUT(LTRACE(), 5) << "Body merger cleared.";

      emit dvidLabelChanged();

      saveMergeOperation();

      ZWidgetMessage message(
            "Body merge finalized.", neutu::EMessageType::INFORMATION,
            ZWidgetMessage::TARGET_TEXT_APPENDING |
            ZWidgetMessage::TARGET_KAFKA);
      emit messageGenerated(message);

      logSynapseInfo(lastTarget);
    }

    if (warnMsg.hasMessage()) {
      emit messageGenerated(warnMsg);
    }
    getProgressSignal()->endProgress();
  }
}

#if 0
void ZFlyEmBodyMergeProject::uploadResultFunc(bool mergingToLargest)
{
//  ZFlyEmBodyMerger *bodyMerger = getBodyMerger();
  if (!m_mergeMap.empty() && m_writer.good()) {
    getProgressSignal()->startProgress("Uploading merge result ...");

    if (m_writer.good()) {
//      ZFlyEmBodyMerger::TLabelMap labelMap = bodyMerger->getFinalMap();

      ZWidgetMessage warnMsg;
      warnMsg.setType(neutube::EMessageType::WARNING);

      auto oldSelection = getSelection(neutube::EBodyLabelType::ORIGINAL);

      if (!labelMap.isEmpty()) {
        /*
        //reorganize the map
        QMap<uint64_t, std::vector<uint64_t> > mergeMap;
        foreach (uint64_t sourceId, labelMap.keys()) {
          uint64_t targetId = labelMap.value(sourceId);
          if (mergeMap.contains(targetId)) {
            std::vector<uint64_t> &idArray = mergeMap[targetId];
            idArray.push_back(sourceId);
          } else {
            mergeMap[targetId] = std::vector<uint64_t>();
            mergeMap[targetId].push_back(sourceId);
          }
        }
        */
        getProgressSignal()->advanceProgress(0.1);

        std::set<uint64_t> bodySet = getSelection(neutube::EBodyLabelType::ORIGINAL);

        std::set<uint64_t> newBodySet;
        foreach (uint64_t targetId, mergeMap.keys()) {
          const std::vector<uint64_t> &merged = mergeMap.value(targetId);
          auto mergeConfig = ZDvid::GetMergeConfig(
                m_writer.getDvidReader(), targetId, merged, mergingToLargest);
          const uint64_t &newTargetId = mergeConfig.first;
          const std::vector<uint64_t> &newMerged = mergeConfig.second;

          m_writer.mergeBody(
                getDvidTarget().getBodyLabelName(), newTargetId, newMerged);
          mergeBodyAnnotation(newTargetId, newMerged);

          if (m_writer.getStatusCode() != 200) {
            emit messageGenerated(
                  ZWidgetMessage(
                    "Failed to upload merging results", neutube::EMessageType::ERROR));
          } else {
#if defined(_FLYEM_)
            std::vector<uint64_t> bodyArray = newMerged;
            if (GET_FLYEM_CONFIG.getNeutuseWriter().ready()) { //Use new server
              for (uint64_t bodyId : bodyArray) {
                m_writer.deleteBodyAnnotation(bodyId);
                m_writer.deleteSkeleton(bodyId);
              }

              neutuse::Task task = neutuse::TaskFactory::MakeDvidTask(
                    "skeletonize", getDvidTarget(), newTargetId, true);
              GET_FLYEM_CONFIG.getNeutuseWriter().uploadTask(task);
            } else {
              if (GET_FLYEM_CONFIG.getNeutuService().isNormal()) {
                if (GET_FLYEM_CONFIG.getNeutuService().requestBodyUpdate(
                      getDvidTarget(), bodyArray, ZNeutuService::UPDATE_DELETE) ==
                    ZNeutuService::REQUEST_FAILED) {
                  warnMsg.setMessage("Computing service failed");
                }

                if (GET_FLYEM_CONFIG.getNeutuService().requestBodyUpdate(
                      getDvidTarget(), newTargetId, ZNeutuService::UPDATE_ALL) ==
                    ZNeutuService::REQUEST_FAILED) {
                  warnMsg.setMessage("Computing service failed");
                }
              }
            }

            if (oldSelection.count(newTargetId) > 0) {
              newBodySet.insert(newTargetId);
            }
#endif
          }

          //Temporary fix for mesh update, which should be moved the remote service
          m_writer.deleteMesh(newTargetId);

          ZOUT(LTRACE(), 5) << "Label slice updated";
        }
        getProgressSignal()->advanceProgress(0.1);

        QList<ZDvidLabelSlice*> labelList =
            getDocument()->getDvidLabelSliceList();
        foreach (ZDvidLabelSlice *slice, labelList) {
          slice->setSelection(newBodySet, neutube::EBodyLabelType::ORIGINAL);
//            slice->mapSelection();
        }

        m_selectedOriginal.clear();
        for (std::set<uint64_t>::const_iterator iter = newBodySet.begin();
             iter != newBodySet.end(); ++iter) {
          m_selectedOriginal.insert(*iter);
        }

        ZFlyEmProofDoc *proofDoc = getDocument<ZFlyEmProofDoc>();
        if (proofDoc != NULL) {
          proofDoc->clearBodyForSplit();
          proofDoc->refreshDvidLabelBuffer(2000);
          ZOUT(LTRACE(), 5) << "Label buffer refreshed";
        }

        ZOUT(LTRACE(), 5) << "Merge uploaded.";

        emit mergeUploaded();

        for (std::set<uint64_t>::const_iterator iter = bodySet.begin();
             iter != bodySet.end(); ++iter) {
          emit checkingInBody(*iter, neutu::EBodySplitMode::NONE);
        }

        getProgressSignal()->advanceProgress(0.1);

        clearBodyMerger();

        ZOUT(LTRACE(), 5) << "Body merger cleared.";

//        ZSleeper::msleep(10000);

        emit dvidLabelChanged();
//        bodyMerger->clear();
        saveMergeOperation();

        ZOUT(LTRACE(), 5) << "Merge operation saved.";

        ZWidgetMessage message("Body merge finalized.");
        emit messageGenerated(message);

        if (warnMsg.hasMessage()) {
          emit messageGenerated(warnMsg);
        }
        getProgressSignal()->advanceProgress(0.1);
      }
    }
    getProgressSignal()->endProgress();
  }
}
#endif

void ZFlyEmBodyMergeProject::uploadResult(bool mergingToLargest)
{
//  QtConcurrent::run(this, &ZFlyEmBodyMergeProject::uploadResultFunc);
  updateMergeMap();
  refreshBodyAnnotationCache();
  uploadResultFunc(mergingToLargest);
}

#if 0
void ZFlyEmBodyMergeProject::detachBodyWindow()
{
  m_bodyWindow = NULL;
}

void ZFlyEmBodyMergeProject::detachCoarseBodyWindow()
{
  m_coarseBodyWindow = NULL;
}
#endif

#if 0
void ZFlyEmBodyMergeProject::presentCoarseBodyView()
{
//  m_bodyWindow->moveToThread(QApplication::instance()->thread());
  if (m_coarseBodyWindow != NULL) {
    m_coarseBodyWindow->show();
    m_coarseBodyWindow->raise();
  }
}

void ZFlyEmBodyMergeProject::showCoarseBody3d()
{
    if(m_bodyViewWindow == NULL){
        m_bodyViewWindow = new Z3DMainWindow(0);
        m_bodyViewWindow->setWindowTitle(QString::fromUtf8("3D Body View"));

        QSizePolicy sizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

        QVBoxLayout* bvLayout = new QVBoxLayout;

        QWidget *toolWidget = new QWidget(m_bodyViewWindow->toolBar);
        bvLayout->addWidget(toolWidget);

        if(m_bodyViewers == NULL){
            m_bodyViewers = new Z3DTabWidget(m_bodyViewWindow);
            m_bodyViewers->setSizePolicy(sizePolicy);
            bvLayout->addWidget(m_bodyViewers);
        }

        m_bodyViewWindow->setLayout(bvLayout);
        m_bodyViewWindow->setCentralWidget(m_bodyViewers);
        m_bodyViewWindow->resize(QDesktopWidget().availableGeometry(0).size()*0.7);
    }

  if (m_coarseBodyWindow == NULL) {
    ZStackDoc *doc = new ZStackDoc;

    makeCoarseBodyWindow(doc);
//    QFuture<void> result =
//        QtConcurrent::run(this, &ZFlyEmBodyMergeProject::make3DBodyWindow, doc);
//    result.waitForFinished();

//    make3DBodyWindow(doc);
  } else {
    //m_coarseBodyWindow->show();
    //m_coarseBodyWindow->raise();
  }

  m_bodyViewers->addTab(m_coarseBodyWindow, "Coarse Body View");
  m_bodyViewers->setTabsClosable(true);

  m_bodyViewWindow->show();
  m_bodyViewWindow->raise();
}

void ZFlyEmBodyMergeProject::showBody3d()
{
  if(m_bodyViewWindow == NULL){
    m_bodyViewWindow = new Z3DMainWindow(0);
    m_bodyViewWindow->setWindowTitle(QString::fromUtf8("3D Body View"));

    QSizePolicy sizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    QVBoxLayout* bvLayout = new QVBoxLayout;

    QWidget *toolWidget = new QWidget(m_bodyViewWindow->toolBar);
    bvLayout->addWidget(toolWidget);

    if(m_bodyViewers == NULL){
      m_bodyViewers = new Z3DTabWidget(m_bodyViewWindow);
      m_bodyViewers->setSizePolicy(sizePolicy);
      bvLayout->addWidget(m_bodyViewers);
    }

    m_bodyViewWindow->setLayout(bvLayout);
    m_bodyViewWindow->setCentralWidget(m_bodyViewers);
    m_bodyViewWindow->resize(QDesktopWidget().availableGeometry(0).size()*0.7);
  }

  if (m_bodyWindow == NULL) {
    makeBodyWindow();
  }

  //m_bodyWindow->show();
  //m_bodyWindow->raise();

  m_bodyViewers->addTab(m_bodyWindow, "Body View");
  m_bodyViewers->setTabsClosable(true);

  m_bodyViewWindow->show();
  m_bodyViewWindow->raise();
}

/*
void ZFlyEmBodyMergeProject::updateSelection()
{
  m_currentSelected = getBodyMerger()->getFinalLabel(m_currentSelected);
}
*/

void ZFlyEmBodyMergeProject::update3DBodyViewDeep()
{
  bool isDeep = true;

  //updateSelection();

  if (m_coarseBodyWindow != NULL) {
    std::set<std::string> currentBodySourceSet;
    std::set<uint64_t> selectedMapped =
        getBodyMerger()->getFinalLabel(m_selectedOriginal.begin(),
                                       m_selectedOriginal.end());

    for (std::set<uint64_t>::const_iterator iter = selectedMapped.begin();
         iter != selectedMapped.end(); ++iter) {
      currentBodySourceSet.insert(
            ZStackObjectSourceFactory::MakeFlyEmBodySource(*iter));
    }

    ZFlyEmDvidReader reader;
    reader.open(getDvidTarget());

    if (getDataFrame() != NULL) {
      ZStack *oldStack = m_coarseBodyWindow->getDocument()->getStack();
      ZStack *newStack = getDataFrame()->document()->getStack();
      if (oldStack != NULL) {
        if (oldStack->getBoundBox().equals(newStack->getBoundBox())) {
          newStack = NULL;
        }
      }
      if (newStack != NULL) {
        m_coarseBodyWindow->getDocument()->loadStack(newStack->clone());
      }
    }

//    ZDvidInfo dvidInfo = reader.readGrayScaleInfo();

    m_coarseBodyWindow->getDocument()->beginObjectModifiedMode(
          ZStackDoc::EObjectModifiedMode::OBJECT_MODIFIED_CACHE);
//    m_bodyWindow->getDocument()->blockSignals(true);

    if (isDeep) {
      m_coarseBodyWindow->getDocument()->removeAllSwcTree(true);
    }

    std::set<std::string> oldBodySourceSet;
    QList<ZSwcTree*> bodyList = m_coarseBodyWindow->getDocument()->getSwcList();
    for (QList<ZSwcTree*>::iterator iter = bodyList.begin();
         iter != bodyList.end(); ++iter) {
      ZSwcTree *tree = *iter;
      if (currentBodySourceSet.count(tree->getSource()) == 0) {
        m_coarseBodyWindow->getDocument()->removeObject(
              dynamic_cast<ZStackObject*>(tree), true);
      } else {
        oldBodySourceSet.insert(tree->getSource());
      }
    }

    for (std::set<uint64_t>::const_iterator iter = selectedMapped.begin();
         iter != selectedMapped.end(); ++iter) {
      uint64_t label = *iter;
      std::string source = ZStackObjectSourceFactory::MakeFlyEmBodySource(label);
      if (oldBodySourceSet.count(source) == 0) {
        ZObject3dScan body;

        QList<uint64_t> bodyList =
            getDocument<ZFlyEmProofDoc>()->getMergedSource(label);
//        bodyList.append(label);

        for (int i = 0; i < bodyList.size(); ++i) {
          body.concat(reader.readCoarseBody(bodyList[i]));
        }

        if (!body.isEmpty()) {
//          body.setColor(sparseObject->getColor());
          if (getDocument<ZFlyEmProofDoc>() != NULL) {
            ZDvidLabelSlice *labelSlice =
                getDocument<ZFlyEmProofDoc>()->getDvidLabelSlice(NeuTube::Z_AXIS);
            if (labelSlice != NULL) {
              body.setColor(labelSlice->getLabelColor(label, NeuTube::BODY_LABEL_MAPPED));
            }
          }
          body.setAlpha(255);
          ZSwcTree *tree = ZSwcGenerator::createSurfaceSwc(body);
          tree->translate(-m_dvidInfo.getStartBlockIndex());
          tree->rescale(m_dvidInfo.getBlockSize().getX(),
                        m_dvidInfo.getBlockSize().getY(),
                        m_dvidInfo.getBlockSize().getZ());
          tree->translate(m_dvidInfo.getStartCoordinates());
          tree->setSource(source);
          m_coarseBodyWindow->getDocument()->addObject(tree, true);
        }
      }
    }
//    m_bodyWindow->getDocument()->blockSignals(false);
//    m_bodyWindow->getDocument()->notifySwcModified();

    m_coarseBodyWindow->getDocument()->endObjectModifiedMode();
    m_coarseBodyWindow->getDocument()->notifyObjectModified();

    m_coarseBodyWindow->show();
    m_coarseBodyWindow->raise();
//    m_coarseBodyWindow->resetCameraCenter();
  }
}

void ZFlyEmBodyMergeProject::update3DBodyViewPlane(
    const ZDvidInfo &dvidInfo, const ZStackViewParam &viewParam)
{
  if (m_coarseBodyWindow != NULL) {
    ZFlyEmMisc::Decorate3dBodyWindowPlane(
          m_coarseBodyWindow, dvidInfo, viewParam);
  }

  if (m_bodyWindow != NULL) {
    ZFlyEmMisc::Decorate3dBodyWindowPlane(m_bodyWindow, dvidInfo, viewParam);
  }
}
#endif

#if 0
void ZFlyEmBodyMergeProject::update3DBodyViewBox(const ZDvidInfo &dvidInfo)
{
  if (m_coarseBodyWindow != NULL) {
    ZCuboid box;
    box.setFirstCorner(dvidInfo.getStartCoordinates().toPoint());
    box.setLastCorner(dvidInfo.getEndCoordinates().toPoint());
    Z3DGraph *graph = Z3DGraphFactory::MakeBox(
          box, dmax2(1.0, dmax3(box.width(), box.height(), box.depth()) / 1000.0));
    graph->setSource(ZStackObjectSourceFactory::MakeFlyEmBoundBoxSource());

    m_coarseBodyWindow->getDocument()->addObject(graph, true);
  }
}

void ZFlyEmBodyMergeProject::update3DBodyView(
    bool showingWindow, bool resettingCamera)
{
  if (m_bodyWindow != NULL) {
    std::set<uint64_t> bodySet = getSelection(NeuTube::BODY_LABEL_ORIGINAL);
    ZFlyEmBody3dDoc *doc =
        qobject_cast<ZFlyEmBody3dDoc*>(m_bodyWindow->getDocument());
    if (doc != NULL){
      doc->addBodyChangeEvent(bodySet.begin(), bodySet.end());
    }
  }

  if (m_coarseBodyWindow != NULL) {
    std::set<std::string> currentBodySourceSet;
    std::set<uint64_t> selectedMapped = getBodyMerger()->getFinalLabel(
          m_selectedOriginal.begin(), m_selectedOriginal.end());
    for (std::set<uint64_t>::const_iterator iter = selectedMapped.begin();
         iter != selectedMapped.end(); ++iter) {
      currentBodySourceSet.insert(
            ZStackObjectSourceFactory::MakeFlyEmBodySource(*iter));
    }

    std::set<std::string> oldBodySourceSet;
    QList<ZSwcTree*> bodyList = m_coarseBodyWindow->getDocument()->getSwcList();
    for (QList<ZSwcTree*>::iterator iter = bodyList.begin();
         iter != bodyList.end(); ++iter) {
      ZSwcTree *tree = *iter;
      if (currentBodySourceSet.count(tree->getSource()) == 0) {
        m_coarseBodyWindow->getDocument()->removeObject(
              dynamic_cast<ZStackObject*>(tree), true);
      } else {
        oldBodySourceSet.insert(tree->getSource());
      }
    }

    if (getDataFrame() != NULL) {
      ZStack *oldStack = m_coarseBodyWindow->getDocument()->getStack();
      ZStack *newStack = getDataFrame()->document()->getStack();
      if (oldStack != NULL) {
        if (oldStack->getBoundBox().equals(newStack->getBoundBox())) {
          newStack = NULL;
        }
      }
      if (newStack != NULL) {
        m_coarseBodyWindow->getDocument()->loadStack(newStack->clone());
      }
    }


//    m_bodyWindow->getDocument()->blockSignals(true);
    m_coarseBodyWindow->getDocument()->beginObjectModifiedMode(
          ZStackDoc::EObjectModifiedMode::OBJECT_MODIFIED_CACHE);


    ZFlyEmDvidReader reader;
    reader.open(getDvidTarget());

//    ZDvidInfo dvidInfo = reader.readGrayScaleInfo();

    if (m_doc->getParentMvc() != NULL) {
      ZFlyEmMisc::Decorate3dBodyWindow(
            m_coarseBodyWindow, m_dvidInfo,
            m_doc->getParentMvc()->getView()->getViewParameter());
    }
    /*
    update3DBodyViewBox(m_dvidInfo);
    if (m_doc->getParentMvc() != NULL) {
      update3DBodyViewPlane(
            m_dvidInfo,
            m_doc->getParentMvc()->getView()->getViewParameter());
    }
    */

    for (std::set<uint64_t>::const_iterator iter = selectedMapped.begin();
         iter != selectedMapped.end(); ++iter) {
      uint64_t label = *iter;
      std::string source = ZStackObjectSourceFactory::MakeFlyEmBodySource(label);
      if (oldBodySourceSet.count(source) == 0) {
        ZObject3dScan body;

        QList<uint64_t> bodyList =
            getDocument<ZFlyEmProofDoc>()->getMergedSource(label);
//        bodyList.append(label);

        for (int i = 0; i < bodyList.size(); ++i) {
          body.concat(reader.readCoarseBody(bodyList[i]));
        }

        if (!body.isEmpty()) {
//          body.setColor(sparseObject->getColor());
          if (getDocument<ZFlyEmProofDoc>() != NULL) {
            ZDvidLabelSlice *labelSlice =
                getDocument<ZFlyEmProofDoc>()->getDvidLabelSlice(NeuTube::Z_AXIS);
            if (labelSlice != NULL) {
              body.setColor(labelSlice->getLabelColor(
                              label, NeuTube::BODY_LABEL_MAPPED));
            }
          }
          body.setAlpha(255);
          ZSwcTree *tree = ZSwcGenerator::createSurfaceSwc(body);
          tree->translate(-m_dvidInfo.getStartBlockIndex());
          tree->rescale(m_dvidInfo.getBlockSize().getX(),
                        m_dvidInfo.getBlockSize().getY(),
                        m_dvidInfo.getBlockSize().getZ());
          tree->translate(m_dvidInfo.getStartCoordinates());
          tree->setSource(source);
          m_coarseBodyWindow->getDocument()->addObject(tree, true);
        }
      }
    }
//    m_bodyWindow->getDocument()->blockSignals(false);
//    m_bodyWindow->getDocument()->notifySwcModified();
    m_coarseBodyWindow->getDocument()->endObjectModifiedMode();
    m_coarseBodyWindow->getDocument()->notifyObjectModified();

    if (showingWindow) {
      m_coarseBodyWindow->show();
      m_coarseBodyWindow->raise();
    }
    if (resettingCamera) {
      m_coarseBodyWindow->resetCameraCenter();
    }
  }
}

void ZFlyEmBodyMergeProject::update3DBodyViewPlane(
    const ZStackViewParam &viewParam)
{
  if (m_coarseBodyWindow != NULL || m_bodyWindow != NULL) {
    ZFlyEmDvidReader reader;
    reader.open(getDvidTarget());

//    ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
    ZCuboid box;
    box.setFirstCorner(m_dvidInfo.getStartCoordinates().toPoint());
    box.setLastCorner(m_dvidInfo.getEndCoordinates().toPoint());

    update3DBodyViewPlane(m_dvidInfo, viewParam);
  }
}

void ZFlyEmBodyMergeProject::update3DBodyView(
    const ZStackObjectSelector &selector)
{
  if (m_coarseBodyWindow != NULL) {
//    m_bodyWindow->getDocument()->removeAllObject();
    std::vector<ZStackObject*> objList =
        selector.getSelectedList(ZStackObject::EType::TYPE_OBJECT3D_SCAN);
    ZFlyEmDvidReader reader;
    reader.open(getDvidTarget());

    ZStack *oldStack = m_coarseBodyWindow->getDocument()->getStack();
    ZStack *newStack = getDocument()->getStack();
    if (oldStack != NULL) {
      if (oldStack->getBoundBox().equals(newStack->getBoundBox())) {
        newStack = NULL;
      }
    }
    if (newStack != NULL) {
      m_coarseBodyWindow->getDocument()->loadStack(newStack->clone());
    }

//    ZDvidInfo dvidInfo = reader.readGrayScaleInfo();

    m_coarseBodyWindow->getDocument()->beginObjectModifiedMode(
          ZStackDoc::EObjectModifiedMode::OBJECT_MODIFIED_CACHE);
//    m_bodyWindow->getDocument()->blockSignals(true);
    for (std::vector<ZStackObject*>::const_iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      ZStackObject *obj = *iter;
      ZObject3dScan *sparseObject = dynamic_cast<ZObject3dScan*>(obj);
      if (sparseObject != NULL) {
        uint64_t label = sparseObject->getLabel();
//        tic();
        ZObject3dScan body = reader.readCoarseBody(label);
//        ptoc();
        if (!body.isEmpty()) {
          body.setColor(sparseObject->getColor());

          if (getDocument<ZFlyEmProofDoc>() != NULL) {
            ZDvidLabelSlice *labelSlice =
                getDocument<ZFlyEmProofDoc>()->getDvidLabelSlice(NeuTube::Z_AXIS);
            if (labelSlice != NULL) {
              body.setColor(labelSlice->getLabelColor(label, NeuTube::BODY_LABEL_ORIGINAL));
            }
          }
          body.setAlpha(255);
          //        tic();
          ZSwcTree *tree = ZSwcGenerator::createSurfaceSwc(body);
          tree->translate(-m_dvidInfo.getStartBlockIndex());
          tree->rescale(m_dvidInfo.getBlockSize().getX(),
                        m_dvidInfo.getBlockSize().getY(),
                        m_dvidInfo.getBlockSize().getZ());
          tree->translate(m_dvidInfo.getStartCoordinates());
          tree->setSource(ZStackObjectSourceFactory::MakeFlyEmBodySource(label));

          m_coarseBodyWindow->getDocument()->addObject(tree, true);
        }
      }
    }

    objList = selector.getDeselectedList(ZStackObject::EType::TYPE_OBJECT3D_SCAN);
#ifdef _DEBUG_
    std::cout << "Deselected: " << objList.size() << std::endl;
#endif
    for (std::vector<ZStackObject*>::const_iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      ZStackObject *obj = *iter;
      ZObject3dScan *sparseObject = dynamic_cast<ZObject3dScan*>(obj);
      if (sparseObject != NULL) {
        uint64_t label = sparseObject->getLabel();
        ZStackObject *obj = m_coarseBodyWindow->getDocument()->getObjectGroup().
            findFirstSameSource(
              ZStackObject::EType::TYPE_SWC,
              ZStackObjectSourceFactory::MakeFlyEmBodySource(label));
        if (obj != NULL) {
          m_coarseBodyWindow->getDocument()->removeObject(obj, true);
        }
      }
    }
//    m_bodyWindow->getDocument()->blockSignals(false);
//    m_bodyWindow->getDocument()->notifySwcModified();

    m_coarseBodyWindow->getDocument()->endObjectModifiedMode();
    m_coarseBodyWindow->getDocument()->notifyObject3dScanModified();

    m_coarseBodyWindow->show();
    m_coarseBodyWindow->raise();
    m_coarseBodyWindow->resetCameraCenter();
  }
}
#endif

void ZFlyEmBodyMergeProject::setBodyStatusProtocol(
    const ZFlyEmBodyAnnotationProtocal &protocol)
{
  m_bodyStatusProtocol = protocol;
}

uint64_t ZFlyEmBodyMergeProject::getSelectedBodyId() const
{
  uint64_t bodyId = 0;
  if (m_dataFrame != NULL) {
    bodyId = m_dataFrame->getCompleteDocument()->getSelectedBodyId();
    /*
    const TStackObjectSet &objSet =
        m_dataFrame->document()->getSelected(ZStackObject::EType::TYPE_OBJECT3D_SCAN);
    if (objSet.size() == 1) {
      const ZObject3dScan* obj =
          dynamic_cast<ZObject3dScan*>(*(objSet.begin()));
      bodyId = obj->getLabel();
    }
    */
  }

  return bodyId;
}


void ZFlyEmBodyMergeProject::notifySplit()
{
  uint64_t bodyId = getSelectedBodyId();
  if (bodyId > 0) {
    emit splitSent(getDvidTarget(), bodyId);
  }
}


void ZFlyEmBodyMergeProject::addSelected(uint64_t label)
{
  m_selectedOriginal.insert(label);
}

void ZFlyEmBodyMergeProject::removeSelected(uint64_t label)
{
  m_selectedOriginal.remove(label);
}

bool ZFlyEmBodyMergeProject::lockNode(const QString &message)
{
  ZDvidWriter writer;
  if (writer.open(getDvidTarget())) {
    return writer.lockNode(message.toStdString());
  }

  return false;
}

std::string ZFlyEmBodyMergeProject::createVersionBranch()
{
  ZDvidWriter writer;
  if (writer.open(getDvidTarget())) {
    return writer.createBranch();
  }

  return "";
}

ZStackDoc* ZFlyEmBodyMergeProject::getDocument() const
{
  return qobject_cast<ZStackDoc*>(parent());
//  return m_doc.get();
}

ZFlyEmBodyMerger* ZFlyEmBodyMergeProject::getBodyMerger() const
{
  if (getDocument<ZFlyEmBodyMergeDoc>() != NULL) {
    return getDocument<ZFlyEmBodyMergeDoc>()->getBodyMerger();
  }

  if (getDocument<ZFlyEmProofDoc>() != NULL) {
    return getDocument<ZFlyEmProofDoc>()->getBodyMerger();
  }

  return NULL;
}

/*
void ZFlyEmBodyMergeProject::setSelectionFromOriginal(const std::set<uint64_t> &selected)
{
  setSelection(getBodyMerger()->getFinalLabel(selected));
}
*/

std::set<uint64_t> ZFlyEmBodyMergeProject::getSelection(
    neutu::ELabelSource labelType) const
{
  ZFlyEmProofDoc *doc = getDocument<ZFlyEmProofDoc>();
  if (doc != NULL) {
    return doc->getSelectedBodySet(labelType);
  }

  return std::set<uint64_t>();
#if 0
  std::set<uint64_t> idSet;

  switch (labelType) {
  case NeuTube::BODY_LABEL_ORIGINAL:
    idSet.insert(m_selectedOriginal.begin(), m_selectedOriginal.end());
    break;
  case NeuTube::BODY_LABEL_MAPPED:
    idSet.insert(m_selectedOriginal.begin(), m_selectedOriginal.end());
    return getBodyMerger()->getFinalLabel(idSet);
  }

  return idSet;
#endif
}

void ZFlyEmBodyMergeProject::notifySelected()
{
  emit messageGenerated(ZWidgetMessage(getSelectionMessage()));
  /*
  QString msg;
  for (QSet<uint64_t>::const_iterator iter = m_selectedOriginal.begin();
       iter != m_selectedOriginal.end(); ++iter) {
    msg += QString("%1 ").arg(*iter);
  }

  if (msg.isEmpty()) {
    msg = "No body selected.";
  } else {
    msg += " selected.";
  }
  */
}

#if 0
void ZFlyEmBodyMergeProject::addSelection(
    uint64_t bodyId, NeuTube::EBodyLabelType labelType)
{
  switch (labelType) {
  case NeuTube::BODY_LABEL_ORIGINAL:
    m_selectedOriginal.insert(bodyId);
    break;
  case NeuTube::BODY_LABEL_MAPPED:
    m_selectedOriginal.unite(getBodyMerger()->getOriginalLabelSet(bodyId));
    break;
  }

  notifySelected();
}


void ZFlyEmBodyMergeProject::setSelection(
    const std::set<uint64_t> &selected, NeuTube::EBodyLabelType labelType)
{
  m_selectedOriginal.clear();
  switch (labelType) {
  case NeuTube::BODY_LABEL_ORIGINAL:
    for (std::set<uint64_t>::const_iterator iter = selected.begin();
         iter != selected.end(); ++iter) {
      m_selectedOriginal.insert(*iter);
    }
    break;
  case NeuTube::BODY_LABEL_MAPPED:
    for (std::set<uint64_t>::const_iterator iter = selected.begin();
         iter != selected.end(); ++iter) {
      QSet<uint64_t> labelSet = getBodyMerger()->getOriginalLabelSet(*iter);
      m_selectedOriginal.unite(labelSet);
    }
    break;
  }

//  QString msg;
//  for (QSet<uint64_t>::const_iterator iter = m_selectedOriginal.begin();
//       iter != m_selectedOriginal.end(); ++iter) {
//    msg += QString("%1 ").arg(*iter);
//  }

//  if (msg.isEmpty()) {
//    msg = "No body selected.";
//  } else {
//    msg += " selected.";
//  }

  emit messageGenerated(ZWidgetMessage(getSelectionMessage()));
//  emitMessage(msg);

//  emit messageGenerated(msg);
}
#endif

QString ZFlyEmBodyMergeProject::getSelectionMessage() const
{
  QString msg;
  ZFlyEmProofDoc *doc = getDocument<ZFlyEmProofDoc>();
  if (doc != NULL) {
    msg = doc->getBodySelectionMessage();
  }
  return msg;
#if 0
  QString msg;

  const std::set<uint64_t> &selected = getSelection(neutube::EBodyLabelType::MAPPED);

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
    msg = "Body selection: " + msg;
//    msg += " selected.";
  }

  return msg;
#endif
}

void ZFlyEmBodyMergeProject::emitMessage(const QString msg, bool appending)
{
  ZWidgetMessage::ETarget target = ZWidgetMessage::TARGET_TEXT;
  if (appending) {
    target = ZWidgetMessage::TARGET_TEXT_APPENDING;
  }

  emit messageGenerated(
        ZWidgetMessage(msg, neutu::EMessageType::INFORMATION,
                       target | ZWidgetMessage::TARGET_KAFKA));
}

void ZFlyEmBodyMergeProject::emitError(const QString msg, bool appending)
{
  ZWidgetMessage::ETarget target = ZWidgetMessage::TARGET_TEXT;
  if (appending) {
    target = ZWidgetMessage::TARGET_TEXT_APPENDING;
  }

  emit messageGenerated(
        ZWidgetMessage(msg, neutu::EMessageType::ERROR,
                       target | ZWidgetMessage::TARGET_KAFKA));
}


uint64_t ZFlyEmBodyMergeProject::getMappedBodyId(uint64_t label) const
{
  if (getBodyMerger() != NULL) {
    return getBodyMerger()->getFinalLabel(label);
  }

  return label;
}

void ZFlyEmBodyMergeProject::setAdmin(bool admin)
{
  m_isAdmin = admin;
}

void ZFlyEmBodyMergeProject::setDvidTarget(const ZDvidTarget &target)
{
  m_writer.open(target);

#if 0
  ZJsonObject obj = m_writer.getDvidReader().readBodyStatusV2();
  m_bodyStatusProtocol.loadJsonObject(obj);

#ifdef _DEBUG_
  m_bodyStatusProtocol.print();
#endif
#endif
}

void ZFlyEmBodyMergeProject::syncWithDvid()
{
  if (getDvidTarget().isValid()) {
    ZFlyEmBodyMerger *bodyMerger = getBodyMerger();
    if (bodyMerger != NULL) {
      QByteArray buffer = getDvidReader().readBuffer(
            ZDvidUrl(getDvidTarget()).getMergeOperationUrl(
              neutu::GetCurrentUserName()));
      bodyMerger->decodeJsonString(buffer.data());

      /*
  ZFlyEmBodyMergeDoc *doc = getDocument<ZFlyEmBodyMergeDoc>();
  doc->getBodyMerger()->decodeJsonString(buffer.data());
  */

      QList<uint64_t> objLabelList = bodyMerger->getFinalMap().keys();

      if (getDocument<ZFlyEmProofDoc>() != NULL) {
        return getDocument<ZFlyEmProofDoc>()->updateBodyObject();
      }

      emit bodyMerged(objLabelList);
    }
  }

  /*
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    reader.readKeyValue(ZDvidData::getName(ZDvidData::ROLE_MERGE_OPERATION),
                        getDvidTarget().getBodyLabelName());
  }
  */
}

#if 0
void ZFlyEmBodyMergeProject::closeBodyWindow()
{
  if (getBodyWindow() != NULL) {
    getBodyWindow()->close();
  }
}
#endif

#if 0
void ZFlyEmBodyMergeProject::highlightSelectedObject(bool hl)
{
  ZFlyEmProofDoc *doc = getDocument<ZFlyEmProofDoc>();
  if (doc != NULL /*&& !m_currentSelected.empty()*/) {
    ZDvidLabelSlice *labelSlice = doc->getDvidLabelSlice(NeuTube::Z_AXIS);
    labelSlice->setVisible(!hl);
//    doc->blockSignals(true);
    doc->beginObjectModifiedMode(ZStackDoc::EObjectModifiedMode::OBJECT_MODIFIED_CACHE);
    doc->removeObject(ZStackObject::TYPE_DVID_SPARSEVOL_SLICE, true);
    /*
    doc->getObjectGroup().removeObject(
          ZStackObject::TYPE_DVID_SPARSEVOL_SLICE, true);
          */
    if (hl) {
      QSet<uint64_t> selected = m_selectedOriginal;

      for (QSet<uint64_t>::const_iterator iter = selected.begin();
           iter != selected.end(); ++iter) {
        uint64_t bodyId = *iter;
        ZDvidSparsevolSlice *obj = new ZDvidSparsevolSlice;
        obj->setDvidTarget(getDvidTarget());
//        obj->setLabel(doc->getBodyMerger()->getFinalLabel(bodyId));
        obj->setLabel(bodyId);
//        uint64_t finalLabel = doc->getBodyMerger()->getFinalLabel(bodyId);
        obj->setRole(ZStackObjectRole::ROLE_ACTIVE_VIEW);
        obj->setColor(doc->getDvidLabelSlice(NeuTube::Z_AXIS)->getLabelColor(
                        bodyId, NeuTube::BODY_LABEL_ORIGINAL));
        doc->addObject(obj);
      }

      labelSlice->setSelection(m_selectedOriginal.begin(),
                               m_selectedOriginal.end(),
                               NeuTube::BODY_LABEL_ORIGINAL);
      /*
      labelSlice->addSelection(
            m_currentSelected.begin(), m_currentSelected.end(),
            NeuTube::BODY_LABEL_MAPPED);
            */

//        labelSlice->addSelection(bodyId);
//      }
//      doc->blockSignals(false);

//      doc->notifyActiveViewModified();
    } else {
//      doc->blockSignals(false);
      doc->notifyActiveViewModified();
    }
    doc->endObjectModifiedMode();
    doc->notifyObjectModified();
//    doc->blockSignals(false);
//    doc->notifyObjectModified();
  }
}
#endif

void ZFlyEmBodyMergeProject::clearBookmarkDecoration()
{
  if (getDocument() != NULL) {
    getDocument()->removeObject(ZStackObjectRole::ROLE_TMP_BOOKMARK, true);
  }

#if 0
  if (getDocument() != NULL) {
    for (std::vector<ZStackObject*>::iterator iter = m_bookmarkDecoration.begin();
         iter != m_bookmarkDecoration.end(); ++iter) {
      ZStackObject *obj = *iter;
      getDocument()->removeObject(obj, false);
      delete obj;
    }
  } else {
    for (std::vector<ZStackObject*>::iterator iter = m_bookmarkDecoration.begin();
         iter != m_bookmarkDecoration.end(); ++iter) {
      delete *iter;
    }
  }
  m_bookmarkDecoration.clear();
#endif
}

void ZFlyEmBodyMergeProject::addBookmarkDecoration(
    const ZFlyEmBookmarkArray &bookmarkArray)
{
  if (getDocument() != NULL) {
    QVector<ZPunctum*> punctumArray = bookmarkArray.toPunctumArray(
          m_isBookmarkVisible);
    getDocument()->addObjectFast(punctumArray.begin(), punctumArray.end());
  }

#if 0
  if (getDocument() != NULL) {

    getDocument()->beginObjectModifiedMode(ZStackDoc::EObjectModifiedMode::OBJECT_MODIFIED_CACHE);
    for (ZFlyEmBookmarkArray::const_iterator iter = bookmarkArray.begin();
         iter != bookmarkArray.end(); ++iter) {
      const ZFlyEmBookmark &bookmark = *iter;
      ZPunctum *circle = new ZPunctum;
      circle->setRole(ZStackObjectRole::ROLE_TMP_BOOKMARK);
      circle->set(bookmark.getLocation(), 5);

//      ZStackBall *circle = new ZStackBall;
//      circle->set(bookmark.getLocation(), 5);
      circle->setColor(255, 0, 0);
      circle->setVisible(m_isBookmarkVisible);
      circle->setHittable(false);
//      circle->setRole(ZStackObjectRole::ROLE_3DGRAPH_DECORATOR);
      getDocument()->addObject(circle);
//      m_bookmarkDecoration.push_back(circle);
    }
    getDocument()->endObjectModifiedMode();
    getDocument()->notifyObjectModified();
  }
#endif
}

void ZFlyEmBodyMergeProject::updateBookmarkDecoration(
    const ZFlyEmBookmarkArray &bookmarkArray)
{
  clearBookmarkDecoration();

  if (getDocument() != NULL) {
    ZFlyEmBookmarkArray filteredBookmarkArray;
    foreach (ZFlyEmBookmark bookmark, bookmarkArray) {
      if (bookmark.getBookmarkType() != ZFlyEmBookmark::EBookmarkType::FALSE_MERGE) {
        filteredBookmarkArray.append(bookmark);
      }
    }
    addBookmarkDecoration(filteredBookmarkArray);
  }
}
#if 0
void ZFlyEmBodyMergeProject::attachBookmarkArray(ZFlyEmBookmarkArray *bookmarkArray)
{
  m_bookmarkArray = bookmarkArray;
}

void ZFlyEmBodyMergeProject::updateBookmarkDecoration()
{
  clearBookmarkDecoration();

  if (getDocument() != NULL) {
    ZFlyEmBookmarkArray bookmarkArray;

    for (ZFlyEmBookmarkArray::const_iterator iter = m_bookmarkArray->begin();
         iter != m_bookmarkArray->end(); ++iter) {
      const ZFlyEmBookmark &bookmark = *iter;
      if (bookmark.getBookmarkType() == ZFlyEmBookmark::EBookmarkType::TYPE_FALSE_SPLIT) {
        bookmarkArray.append(bookmark);
      }
    }

    addBookmarkDecoration(bookmarkArray);
  }
}
#endif
void ZFlyEmBodyMergeProject::setBookmarkVisible(bool visible)
{
  m_isBookmarkVisible = visible;
}

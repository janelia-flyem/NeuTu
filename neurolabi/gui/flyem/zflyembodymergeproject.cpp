#include "zflyembodymergeproject.h"

#include <QtConcurrentRun>
#include <QApplication>
#include <QItemSelectionModel>
#include <QDesktopWidget>

#include "zintpoint.h"
#include "zstackdvidgrayscalefactory.h"
#include "dvid/zdvidreader.h"
#include "zstackframe.h"
#include "zstackdoc.h"
#include "flyem/zflyembodymergeframe.h"
#include "flyem/zflyembodymergedoc.h"
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
#include "zstackpresenter.h"
#include "dvid/zdvidbufferreader.h"
#include "dvid/zdvidurl.h"
#include "zflyemproofdoc.h"
#include "zstackmvc.h"
#include "dvid/zdvidsparsevolslice.h"
#include "dvid/zdvidlabelslice.h"
#include "zwidgetmessage.h"
#include "z3dgraphfactory.h"
#include "zstackdochelper.h"
#include "z3dgraphfilter.h"
#include "zflyemmisc.h"
#include "zprogresssignal.h"
#include "zdialogfactory.h"
#include "flyem/zflyembookmark.h"
#include "zsleeper.h"
#include "flyem/zflyembody3ddoc.h"
#include "zstackview.h"
#include "widgets/z3dtabwidget.h"

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

#if 1
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
  ZDvidTarget target = m_reader.getDvidTarget();
  target.setUuid(newUuid);
  m_reader.open(target);
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
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
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
      ZArray *array = m_reader.readLabels64(
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
                << m_docReader.getObjectGroup().getObjectList(ZStackObject::TYPE_SWC).size()
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
//          ZStackObject::TYPE_OBJECT3D_SCAN);
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

  connect(this, SIGNAL(selectionChanged(ZStackObjectSelector)),
          this, SLOT(update3DBodyView(ZStackObjectSelector)));
  connect(this, SIGNAL(selectionChanged()), this, SLOT(update3DBodyView()));
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
        getSelectedObjectList<ZObject3dScan>(ZStackObject::TYPE_OBJECT3D_SCAN);
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
}

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
}

void ZFlyEmBodyMergeProject::mergeBodyAnnotation(
    uint64_t targetId, const std::vector<uint64_t> &bodyId)
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    ZFlyEmBodyAnnotation annotation = reader.readBodyAnnotation(targetId);
    for (std::vector<uint64_t>::const_iterator iter = bodyId.begin();
         iter != bodyId.end(); ++iter) {
      if (*iter != targetId) {
        ZFlyEmBodyAnnotation subann = reader.readBodyAnnotation(*iter);
        annotation.mergeAnnotation(subann);
      }
    }

    if (annotation.getStatus().empty()) {
      annotation.setStatus("Not examined");
    }

    if (!annotation.isEmpty()) {
      ZDvidWriter writer;
      if (writer.open(getDvidTarget())) {
        writer.writeBodyAnntation(annotation);
      }
    }
  }
}

uint64_t ZFlyEmBodyMergeProject::getTargetId(
    uint64_t targetId, const std::vector<uint64_t> &bodyId,
    bool mergingToLargest)
{
  if (mergingToLargest) {
    int maxSize = m_reader.readBodyBlockCount(targetId);
    for (uint64_t id : bodyId) {
      int bodySize = m_reader.readBodyBlockCount(id);
      if (bodySize > maxSize) {
        bodySize = maxSize;
        targetId = id;
      }
    }
  }

  return targetId;
}

void ZFlyEmBodyMergeProject::uploadResultFunc(bool mergingToLargest)
{
  ZFlyEmBodyMerger *bodyMerger = getBodyMerger();
  if (bodyMerger != NULL) {
    getProgressSignal()->startProgress("Uploading merge result ...");

    if (m_writer.good()) {
      ZFlyEmBodyMerger::TLabelMap labelMap = bodyMerger->getFinalMap();

      ZWidgetMessage warnMsg;
      warnMsg.setType(neutube::MSG_WARNING);

      if (!labelMap.isEmpty()) {
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
        getProgressSignal()->advanceProgress(0.1);

        std::set<uint64_t> bodySet = getSelection(neutube::BODY_LABEL_ORIGINAL);

        std::set<uint64_t> newBodySet;
        foreach (uint64_t targetId, mergeMap.keys()) {
          const std::vector<uint64_t> &merged = mergeMap.value(targetId);
          uint64_t newTargetId = getTargetId(targetId, merged, mergingToLargest);
          newBodySet.insert(newTargetId);
          std::vector<uint64_t> newMerged;
          if (targetId != newTargetId) {
            newMerged.push_back(targetId);
            for (uint64_t id : merged) {
              if (id != newTargetId) {
                newMerged.push_back(id);
              }
            }
          } else {
            newMerged = merged;
          }

          m_writer.mergeBody(
                getDvidTarget().getBodyLabelName(), newTargetId, newMerged);
          mergeBodyAnnotation(newTargetId, newMerged);

          if (m_writer.getStatusCode() != 200) {
            emit messageGenerated(
                  ZWidgetMessage(
                    "Failed to upload merging results", neutube::MSG_ERROR));
          } else {
#if defined(_FLYEM_)
            std::vector<uint64_t> bodyArray = newMerged;
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
#endif
          }

          //Temporary fix for mesh update, which should be moved the remote service
          m_writer.deleteMesh(newTargetId);

          QList<ZDvidLabelSlice*> labelList =
              getDocument()->getDvidLabelSliceList();
          foreach (ZDvidLabelSlice *slice, labelList) {
            slice->setSelection(newBodySet, neutube::BODY_LABEL_ORIGINAL);
//            slice->mapSelection();
          }
          ZOUT(LTRACE(), 5) << "Label slice updated";
        }
        getProgressSignal()->advanceProgress(0.1);

        m_selectedOriginal.clear();
        for (std::set<uint64_t>::const_iterator iter = newBodySet.begin();
             iter != newBodySet.end(); ++iter) {
          m_selectedOriginal.insert(*iter);
        }

        if (getDocument<ZFlyEmProofDoc>() != NULL) {
          getDocument<ZFlyEmProofDoc>()->clearBodyForSplit();
          getDocument<ZFlyEmProofDoc>()->refreshDvidLabelBuffer(2000);
          ZOUT(LTRACE(), 5) << "Label buffer refreshed";
        }

        ZOUT(LTRACE(), 5) << "Merge uploaded.";

        emit mergeUploaded();

#if 0
        ZSleeper::msleep(2000);

        QList<ZDvidLabelSlice*> labelList =
            getDocument()->getDvidLabelSliceList();
        foreach (ZDvidLabelSlice *slice, labelList) {
          slice->refreshReaderBuffer();
        }
#endif

//        std::set<uint64_t> selectionSet =
//            getSelection(NeuTube::BODY_LABEL_MAPPED);

        for (std::set<uint64_t>::const_iterator iter = bodySet.begin();
             iter != bodySet.end(); ++iter) {
          emit checkingInBody(*iter, flyem::BODY_SPLIT_NONE);
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

void ZFlyEmBodyMergeProject::uploadResult(bool mergingToLargest)
{
//  QtConcurrent::run(this, &ZFlyEmBodyMergeProject::uploadResultFunc);
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
          ZStackDoc::OBJECT_MODIFIED_CACHE);
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
          ZStackDoc::OBJECT_MODIFIED_CACHE);


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
        selector.getSelectedList(ZStackObject::TYPE_OBJECT3D_SCAN);
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
          ZStackDoc::OBJECT_MODIFIED_CACHE);
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

    objList = selector.getDeselectedList(ZStackObject::TYPE_OBJECT3D_SCAN);
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
              ZStackObject::TYPE_SWC,
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

uint64_t ZFlyEmBodyMergeProject::getSelectedBodyId() const
{
  uint64_t bodyId = 0;
  if (m_dataFrame != NULL) {
    bodyId = m_dataFrame->getCompleteDocument()->getSelectedBodyId();
    /*
    const TStackObjectSet &objSet =
        m_dataFrame->document()->getSelected(ZStackObject::TYPE_OBJECT3D_SCAN);
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

#if 0
void ZFlyEmBodyMergeProject::setDocument(ZSharedPointer<ZStackDoc> doc)
{
  m_doc = doc;

  if (m_doc.get() != NULL) {
    connect(m_doc.get(), SIGNAL(objectSelectorChanged(ZStackObjectSelector)),
            this, SIGNAL(selectionChanged(ZStackObjectSelector)));
    connect(this, SIGNAL(selectionChanged(ZStackObjectSelector)),
            this, SLOT(update3DBodyView(ZStackObjectSelector)));
    connect(this, SIGNAL(selectionChanged()), this, SLOT(update3DBodyView()));
  }
}
#endif

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
    neutube::EBodyLabelType labelType) const
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

  const std::set<uint64_t> &selected = getSelection(neutube::BODY_LABEL_MAPPED);

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

void ZFlyEmBodyMergeProject::emitMessage(const QString msg, bool appending)
{
  ZWidgetMessage::ETarget target = ZWidgetMessage::TARGET_TEXT;
  if (appending) {
    target = ZWidgetMessage::TARGET_TEXT_APPENDING;
  }

  emit messageGenerated(
        ZWidgetMessage(msg, neutube::MSG_INFORMATION, target));
}

void ZFlyEmBodyMergeProject::emitError(const QString msg, bool appending)
{
  ZWidgetMessage::ETarget target = ZWidgetMessage::TARGET_TEXT;
  if (appending) {
    target = ZWidgetMessage::TARGET_TEXT_APPENDING;
  }

  emit messageGenerated(
        ZWidgetMessage(msg, neutube::MSG_ERROR, target));
}


uint64_t ZFlyEmBodyMergeProject::getMappedBodyId(uint64_t label) const
{
  if (getBodyMerger() != NULL) {
    return getBodyMerger()->getFinalLabel(label);
  }

  return label;
}

void ZFlyEmBodyMergeProject::setDvidTarget(const ZDvidTarget &target)
{
  if (m_reader.open(target)) {
    m_dvidInfo = m_reader.readGrayScaleInfo();
    m_writer.open(m_reader.getDvidTarget());
  }
}

void ZFlyEmBodyMergeProject::syncWithDvid()
{
  if (getDvidTarget().isValid()) {
    ZFlyEmBodyMerger *bodyMerger = getBodyMerger();
    if (bodyMerger != NULL) {
      QByteArray buffer = m_reader.readBuffer(
            ZDvidUrl(getDvidTarget()).getMergeOperationUrl(
              neutube::GetCurrentUserName()));
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
    doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
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

    getDocument()->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
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
      if (bookmark.getBookmarkType() != ZFlyEmBookmark::TYPE_FALSE_MERGE) {
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
      if (bookmark.getBookmarkType() == ZFlyEmBookmark::TYPE_FALSE_SPLIT) {
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

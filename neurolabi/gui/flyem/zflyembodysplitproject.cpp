//#define _NEUTU_USE_REF_KEY_
#include "zflyembodysplitproject.h"

#include <QProcess>
#include <QByteArray>
#include <QtConcurrentRun>
#include <QWidget>
#include <QUndoStack>
#include <QMutexLocker>

#include "zjsondef.h"

#include "mvc/zstackframe.h"
#include "mvc/zstackview.h"

#include "z3dwindow.h"
#include "zstackdoclabelstackfactory.h"
#include "zstackobject.h"
#include "zstackball.h"
#include "zsparsestack.h"
#include "zswctree.h"
#include "zwindowfactory.h"

#include "zstackskeletonizer.h"
#include "neutubeconfig.h"
#include "zswcgenerator.h"
#include "z3dswcfilter.h"
#include "zobject3dscan.h"
#include "zstroke2d.h"
#include "zstring.h"
#include "zflyemcoordinateconverter.h"
#include "zflyemneuron.h"

#include "zstackpatch.h"
#include "zstackobjectsource.h"
#include "neutubeconfig.h"
#include "zarray.h"

#include "zstackobjectsourcefactory.h"
#include "zflyemproofdoc.h"
#include "zwidgetmessage.h"
#include "zflyemmisc.h"
#include "mvc/zstackdochelper.h"
#include "zintcuboidobj.h"
#include "dialogs/zflyemsplituploadoptiondialog.h"
#include "zneutuservice.h"
#include "zglobal.h"
#include "zserviceconsumer.h"
#include "z3dvolumefilter.h"
#include "zstackdocaccessor.h"
#include "zswcfactory.h"
#include "neutuse/task.h"
#include "neutuse/taskfactory.h"
#include "zdialogfactory.h"
#include "zpunctum.h"
#include "zflyembookmarkarray.h"
#include "misc/miscutility.h"

#include "dvid/zdvidsparsestack.h"
#include "dvid/zdvidgrayslice.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdviddata.h"
#include "dvid/zdvidurl.h"

const char* ZFlyEmBodySplitProject::THREAD_RESULT_QUICK = "updateSplitQuickFunc";

ZFlyEmBodySplitProject::ZFlyEmBodySplitProject(QObject *parent) :
  QObject(parent), m_bodyId(0), m_dataFrame(NULL),
  m_quickResultWindow(NULL),
  m_minObjSize(0), m_keepingMainSeed(false), /*m_bookmarkArray(NULL),*/
  m_isBookmarkVisible(true), m_showingBodyMask(false)
{
  m_progressSignal = new ZProgressSignal(this);

  m_skelThre = 20;
  m_splitMode = neutu::EBodySplitMode::ONLINE;

//  connect(this, SIGNAL(bodyQuickViewReady()), this, SLOT(startBodyQuickView()));
//  connect(this, SIGNAL(result3dQuickViewReady()),
//          this, SLOT(startResultQuickView()));
//  connect(this, SIGNAL(rasingBodyQuickView()), this, SLOT(raiseBodyQuickView()));
//  connect(this, SIGNAL(rasingResultQuickView()), this, SLOT(raiseResultQuickView()));

  m_timer = new QTimer(this);
  m_timer->setInterval(200);
  connect(m_timer, &QTimer::timeout,
          this, &ZFlyEmBodySplitProject::updateSplitQuick);
  connect(this, &ZFlyEmBodySplitProject::splitListGenerated,
          this, &ZFlyEmBodySplitProject::uploadSplitList);
  connect(this, &ZFlyEmBodySplitProject::resultCommitted,
          this, &ZFlyEmBodySplitProject::resetStatusAfterUpload);
}

ZFlyEmBodySplitProject::~ZFlyEmBodySplitProject()
{
  clear();
}

void ZFlyEmBodySplitProject::clear(QWidget *widget)
{
  if (widget != NULL) {
    widget->hide();
    delete widget;
    widget = NULL;
  }
}

void ZFlyEmBodySplitProject::start()
{
  m_timer->start();
}

void ZFlyEmBodySplitProject::exit()
{
  quitResultUpdate();
  if (m_quickResultWindow != NULL) {
    m_quickResultWindow->close();
  }
  m_timer->stop();
}

void ZFlyEmBodySplitProject::clear()
{
  clearQuickResultWindow();

  if (m_dataFrame != NULL) {
    m_dataFrame->hide();
    delete m_dataFrame;
    m_dataFrame = NULL;
  }

  shallowClear();
}

void ZFlyEmBodySplitProject::shallowClearDataFrame()
{
  /*
  if (m_resultWindow != NULL) {
    m_resultWindow->hide();
    delete m_resultWindow;
    m_resultWindow = NULL;
  }
  */

  clear(m_quickResultWindow);

  shallowClear();
}

void ZFlyEmBodySplitProject::shallowClear()
{
//  m_resultWindow = NULL;
  m_quickResultWindow = NULL;
  m_dataFrame = NULL;

  m_bodyId = 0;

//  m_bookmarkDecoration.clear();
}

void ZFlyEmBodySplitProject::shallowClearResultWindow()
{
//  m_resultWindow = NULL;
}

void ZFlyEmBodySplitProject::shallowClearQuickResultWindow()
{
  m_quickResultWindow = NULL;
}

void ZFlyEmBodySplitProject::shallowClearQuickViewWindow()
{
}

/*
void ZFlyEmBodySplitProject::shallowClearBodyWindow()
{
  m_bodyWindow = NULL;
}
*/
void ZFlyEmBodySplitProject::setDvidTarget(const ZDvidTarget &target)
{
  LINFO() << "Setting dvid env in ZFlyEmBodySplitProject";
  m_reader.open(target);

#if 0
  if (m_reader.open(target)) {
    m_dvidInfo = m_reader.readLabelInfo();
#ifdef _DEBUG_
    std::vector<int> stackSize = m_dvidInfo.getStackSize();
    std::cout << "Dvid info: " << stackSize[0] << " " << stackSize[1] << " "
              << stackSize[2] << std::endl;
#endif
  }
#endif
}

void ZFlyEmBodySplitProject::setDvidInfo(const ZDvidInfo &info)
{
  m_dvidInfo = info;
}

void ZFlyEmBodySplitProject::setBodyId(uint64_t bodyId)
{
  m_bodyId = bodyId;
  invalidateSplitQuick();
}

void ZFlyEmBodySplitProject::showDataFrame() const
{
  if (m_dataFrame != NULL) {
    m_dataFrame->show();
    m_dataFrame->raise();
    m_dataFrame->activateWindow();
  }
}

ZProgressSignal* ZFlyEmBodySplitProject::getProgressSignal() const
{
  return m_progressSignal;
}

void ZFlyEmBodySplitProject::showDataFrame3d()
{
  if (getDocument() != NULL) {
    Z3DWindow *window = getDocument()->getParent3DWindow();

    if (window == NULL) {
      /*
      if (getMainWindow() != NULL) {
        getMainWindow()->startProgress("Opening 3D View ...", 0);
      }
      */

      ZWindowFactory factory;
      //factory.setParentWidget(parent);
      window = factory.make3DWindow(
            getSharedDocument(), Z3DView::EInitMode::EXCLUDE_MESH);
      window->setWindowTitle(getDocument()->getTitle());

      //getDocument()->registerUser(window);

      if (getDocument()->getParentFrame() != NULL) {
        connect(getDocument()->getParentFrame(), SIGNAL(closed(ZStackFrame*)),
                window, SLOT(close()));
      }

      /*
      if (getMainWindow() != NULL) {
        getMainWindow()->endProgress();
      }
      */
    }

    if (window != NULL) {
      window->show();
      window->raise();
    } else {
      emitError("3D functions are disabled");
//      emit messageGenerated("3D functions are disabled");
    }

  }
}

/*
void ZFlyEmBodySplitProject::raiseBodyQuickView()
{
  if (m_quickViewWindow != NULL) {
    m_quickViewWindow->raise();
  }
}
*/
void ZFlyEmBodySplitProject::raiseResultQuickView()
{
  if (m_quickResultWindow != NULL) {
    m_quickResultWindow->raise();
  }
}

void ZFlyEmBodySplitProject::showQuickView(Z3DWindow *window)
{
  if (window != NULL) {
    std::cout << "Showing quick view" << std::endl;
    window->show();
    std::cout << "Quick view ready" << std::endl;
    window->raise();
  }
}

void ZFlyEmBodySplitProject::clearQuickResultWindow()
{
  quitResultUpdate();
  if (m_quickResultWindow != NULL) {
    m_quickResultWindow->hide();
    delete m_quickResultWindow;
    m_quickResultWindow = NULL;
  }
}

void ZFlyEmBodySplitProject::startQuickView(Z3DWindow *window)
{
  if (window != NULL) {
    std::cout << "Starting quick view ..." << std::endl;
    window->setYZView();
    std::cout << "Estimating body bound box ..." << std::endl;
    const TStackObjectList &objList =
        window->getDocument()->getObjectList(ZStackObject::EType::SWC);

    ZCuboid boundBox;
    for (TStackObjectList::const_iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      ZSwcTree *tree = dynamic_cast<ZSwcTree*>(*iter);
      if (tree != NULL) {
        if (boundBox.isValid()) {
          boundBox.bind(tree->getBoundBox());
        } else {
          boundBox = tree->getBoundBox();
        }
      }
    }

    std::cout << "Zooming in" << std::endl;
    window->gotoPosition(boundBox);
//    m_quickViewWindow->setYZView();

    std::cout << "Showing quick view ..." << std::endl;
    showQuickView(window);
  }
}

void ZFlyEmBodySplitProject::startResultQuickView()
{
  startQuickView(m_quickResultWindow);
}


void ZFlyEmBodySplitProject::updateSplitQuick()
{
  if (!m_splitUpdated) {
    m_splitUpdated = true;
    const QString threadId = THREAD_RESULT_QUICK;
    if (!m_futureMap.isAlive(threadId)) {
      m_futureMap.removeDeadThread();
      m_cancelSplitQuick = false;
      QFuture<void> future =
          QtConcurrent::run(this, &ZFlyEmBodySplitProject::updateSplitQuickFunc);
      m_futureMap[threadId] = future;
    }
  }
}

void ZFlyEmBodySplitProject::waitResultQuickView()
{
  m_futureMap.waitForFinished(THREAD_RESULT_QUICK);
}

void ZFlyEmBodySplitProject::invalidateSplitQuick()
{
  m_cancelSplitQuick = true;
  m_splitUpdated = false;
}

void ZFlyEmBodySplitProject::updateSplitQuickFunc()
{
  loadResult3dQuick(m_quickResultDoc);
}

void ZFlyEmBodySplitProject::loadResult3dQuick(ZSharedPointer<ZStackDoc> doc)
{
  loadResult3dQuick(doc.get());
}

#if 1
void ZFlyEmBodySplitProject::loadResult3dQuick(ZStackDoc *doc)
{
  if (doc != NULL && getDocument() != NULL) {
    ZOUT(LINFO(), 3) << "Loading split results";

//    doc->beginObjectModifiedMode(ZStackDoc::EObjectModifiedMode::OBJECT_MODIFIED_CACHE);

    ZOUT(LINFO(), 3) << "Removing all SWCs";
//    doc->removeAllSwcTree();
    ZStackDocAccessor::RemoveAllSwcTree(doc, true);

    TStackObjectList objList =
        getDocument()->getObjectList(ZStackObjectRole::ROLE_SEGMENTATION);
    const int maxSwcNodeNumber = 100000;
    const int maxScale = 50;
    const int minScale = 1;
//    getProgressSignal()->advanceProgress(0.1);

//    double dp = 0.0;
//    if (!objList.isEmpty()) {
//      dp =  0.9 / objList.size();
//    }

    ZObject3dScan wholeBody;
    if (!objList.isEmpty()) {
      ZFlyEmProofDoc *proofDoc = getDocument<ZFlyEmProofDoc>();
      if (proofDoc != NULL) {
        ZDvidSparseStack *spStack = proofDoc->getBodyForSplit();
        wholeBody = *(spStack->getObjectMask());
      }
    }

    for (TStackObjectList::const_iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      ZObject3dScan *splitObj = dynamic_cast<ZObject3dScan*>(*iter);

      ZOUT(LINFO(), 3) << "Processing split object" << splitObj;
      if (splitObj != NULL) {
        if (splitObj->hasRole(ZStackObjectRole::ROLE_SEGMENTATION)) {
          if (!wholeBody.isEmpty()) {
            //For testing
//            ZObject3dScan testBody = wholeBody;
//            testBody.subtract(*splitObj);
            ////

            wholeBody.subtractSliently(*splitObj);

//            std::cout << "Subtract comparison: " << testBody.equalsLiterally(wholeBody)
//                      << std::endl;
          }

          ZOUT(LINFO(), 3) << "Converting split object";
          if (splitObj != NULL) {
            int ds = splitObj->getVoxelNumber() / maxSwcNodeNumber + 1;
            if (ds < minScale) {
              ds = minScale;
            }
            if (ds > maxScale) {
              ds = maxScale;
            }

            ZOUT(LINFO(), 3) << "Creating split SWC";
            ZSwcTree *tree = ZSwcGenerator::createSurfaceSwc(*splitObj, ds);
            if (tree != NULL) {
              tree->setAlpha(255);
              ZOUT(LINFO(), 3) << "Adding split SWC";
              ZStackDocAccessor::AddObject(doc, tree);
            }
          }
        }
      }

      if (m_cancelSplitQuick) {
        break;
      }
//      getProgressSignal()->advanceProgress(dp);
    }


    if (!wholeBody.isEmpty() && !m_cancelSplitQuick) {
      ZOUT(LINFO(), 3) << "Adding remain SWC";
      ZSwcTree *tree = ZSwcGenerator::createSurfaceSwc(wholeBody, 10);
      if (tree != NULL && !m_cancelSplitQuick) {
        tree->setColor(255, 255, 255);
        ZStackDocAccessor::AddObject(doc, tree);
      }
    }

    if (m_cancelSplitQuick) {
      ZStackDocAccessor::RemoveAllSwcTree(doc, true);
    }

    ZOUT(LINFO(), 3) << "Split object processed";
  }
}
#endif

void ZFlyEmBodySplitProject::updateResult3dQuick()
{
  emit messageGenerated(ZWidgetMessage("Updating 3D split view ..."));
//  result3dQuickFunc();
#if 1
  const QString threadId = "result3dQuickFunc";
  if (!m_futureMap.isAlive(threadId)) {
    m_futureMap.removeDeadThread();
    QFuture<void> future =
        QtConcurrent::run(this, &ZFlyEmBodySplitProject::result3dQuickFunc);
    m_futureMap[threadId] = future;
  }
#endif
//  updateResult3dQuickFunc();
//  QtConcurrent::run(this, &ZFlyEmBodySplitProject::updateResult3dQuickFunc);
}

void ZFlyEmBodySplitProject::quitResultUpdate()
{
  m_timer->stop();
  invalidateSplitQuick();
  m_futureMap.waitForFinished(THREAD_RESULT_QUICK);
  if (m_quickResultDoc) {
    m_quickResultDoc->removeAllSwcTree(true);
  }
  m_splitUpdated = true;
}

void ZFlyEmBodySplitProject::cancelResultUpdate()
{
  m_cancelSplitQuick = true;
  m_futureMap.waitForFinished(THREAD_RESULT_QUICK);
}

void ZFlyEmBodySplitProject::result3dQuickFunc()
{
  ZStackDoc *mainDoc = getDocument();

  if (mainDoc != NULL) {
    if (m_quickResultWindow != NULL) {
//      getProgressSignal()->startProgress("Showing result quick view ...");

      ZStackDoc *doc = m_quickResultWindow->getDocument();

//      getProgressSignal()->advanceProgress(0.1);

//      getProgressSignal()->startProgress(0.5);
      loadResult3dQuick(doc);
//      getProgressSignal()->endProgress();
      std::cout << "Result update done" << std::endl;
//      emit result3dQuickViewReady();
//      getProgressSignal()->endProgress();

//      emit rasingResultQuickView();
    }
  }
}

void ZFlyEmBodySplitProject::showResultQuickView()
{
  ZStackDoc *mainDoc = getDocument();

  if (mainDoc != NULL) {
    if (m_quickResultWindow == NULL) {
      ZWindowFactory windowFactory;
      windowFactory.setWindowTitle("Splitting Result");
      invalidateSplitQuick();

//      ZStackDoc *doc = new ZStackDoc;
      m_quickResultDoc = ZSharedPointer<ZStackDoc>(new ZStackDoc);
      m_quickResultDoc->setTag(neutu::Document::ETag::FLYEM_BODY_DISPLAY);
//      m_quickResultDoc->disconnectSwcNodeModelUpdate();
      m_quickResultWindow = windowFactory.make3DWindow(m_quickResultDoc);
      m_quickResultWindow->getSwcFilter()->setColorMode("Intrinsic");
      m_quickResultWindow->getSwcFilter()->setRenderingPrimitive("Sphere");
      m_quickResultWindow->setAttribute(Qt::WA_DeleteOnClose, false);
      m_quickResultWindow->setYZView();


      connect(m_quickResultWindow, SIGNAL(destroyed()),
              this, SLOT(shallowClearQuickResultWindow()));
      connect(m_quickResultWindow, SIGNAL(closed()), m_timer, SLOT(stop()));
//      connect(m_quickResultWindow, &Z3DWindow::closed,
//              this, &ZFlyEmBodySplitProject::resetQuickResultWindow);
//      connect(mainDoc, &ZStackDoc::labelFieldModified,
//              this, &ZFlyEmBodySplitProject::invalidateSplitQuick);
      connect(mainDoc, &ZStackDoc::segmentationUpdated,
              this, &ZFlyEmBodySplitProject::invalidateSplitQuick);

      if (m_dataFrame != NULL) {
        connect(m_quickResultWindow, SIGNAL(locating2DViewTriggered(int, int, int, int)),
                m_dataFrame, SLOT(setView(int, int, int, int)));
      }
      connect(m_quickResultWindow, SIGNAL(locating2DViewTriggered(int, int, int, int)),
              this, SIGNAL(locating2DViewTriggered(int, int, int, int)));

//      ZDvidReader reader;
      ZIntCuboid box(m_dvidInfo.getStartCoordinates(),
                     m_dvidInfo.getEndCoordinates());
      if (m_reader.isReady()) {
        Z3DGraph *boxGraph = flyem::MakeBoundBoxGraph(m_dvidInfo);
//        boxGraph->boundBox(&box);
        ZStackDocAccessor::AddObject(
              m_quickResultDoc.get(), boxGraph);
        ZStackDocAccessor::AddObject(
              m_quickResultDoc.get(),
              flyem::MakePlaneGraph(getDocument(), m_dvidInfo));
        m_quickResultWindow->setYZView();
//        ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
//        doc->addObject(ZFlyEmMisc::MakeBoundBoxGraph(m_dvidInfo), true);
//        doc->addObject(ZFlyEmMisc::MakePlaneGraph(getDocument(), m_dvidInfo), true);
      }

      if (!box.isEmpty()) {
        m_quickResultWindow->gotoPosition(
              ZCuboid(box.getFirstCorner().getX(),
                      box.getFirstCorner().getY(),
                      box.getFirstCorner().getZ(),
                      box.getLastCorner().getX(),
                      box.getLastCorner().getY(),
                      box.getLastCorner().getZ()));
      }
    } else {
      if (!m_quickResultWindow->isVisible()) {
        m_futureMap.waitForFinished(THREAD_RESULT_QUICK);
      }
    }
    showQuickView(m_quickResultWindow);
    if (!m_timer->isActive()) {
      m_timer->start();
    }
  }
}

void ZFlyEmBodySplitProject::resetQuickResultWindow()
{
  if (m_futureMap.hasThreadAlive()) {
    m_futureMap.waitForFinished();
  }
  m_quickResultDoc->removeAllSwcTree(true);
}

bool ZFlyEmBodySplitProject::hasDataFrame() const
{
  return m_dataFrame != NULL;
}

void ZFlyEmBodySplitProject::setDataFrame(ZStackFrame *frame)
{
  if (m_dataFrame != NULL) {
    clear();
  }

  m_dataFrame = frame;
  connect(m_dataFrame, SIGNAL(splitStarted()), this, SLOT(backupSeed()));

//  updateBookDecoration();
}


bool ZFlyEmBodySplitProject::hasBookmark() const
{
  if (getDocument() != NULL) {
    ZOUT(LTRACE(), 5) << "Checking bookmarks";
    return !getDocument()->getObjectList(
          ZStackObject::EType::FLYEM_BOOKMARK).isEmpty();
  }

  return false;
}

int ZFlyEmBodySplitProject::getBookmarkCount() const
{
  if (getDocument() != NULL) {
    ZOUT(LTRACE(), 5) << "Get bookmark count";
    return getDocument()->getObjectList(
          ZStackObject::EType::FLYEM_BOOKMARK).size();
  }

  return 0;
}


void ZFlyEmBodySplitProject::locateBookmark(const ZFlyEmBookmark &bookmark)
{
  if (m_dataFrame != NULL) {
    m_dataFrame->viewRoi(bookmark.getLocation().getX(),
                         bookmark.getLocation().getY(),
                         bookmark.getLocation().getZ(), 5);
  }
}

void ZFlyEmBodySplitProject::addBookmarkDecoration(
    const ZFlyEmBookmarkArray &bookmarkArray)
{
  if (getDocument() != NULL) {
    QVector<ZPunctum*> punctumArray = bookmarkArray.toPunctumArray(
          m_isBookmarkVisible);
    getDocument()->addObjectFast(punctumArray.begin(), punctumArray.end());
  }
}

void ZFlyEmBodySplitProject::setBookmarkVisible(bool visible)
{
  m_isBookmarkVisible = visible;
}

std::set<int> ZFlyEmBodySplitProject::getBookmarkBodySet() const
{
  std::set<int> bodySet;
  ZStackDoc *doc = getDocument();
  if (doc != NULL) {
    ZOUT(LTRACE(), 5) << "Get bookmark body set";
    const TStackObjectList &objList =
        doc->getObjectList(ZStackObject::EType::FLYEM_BOOKMARK);
    for (TStackObjectList::const_iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      const ZFlyEmBookmark *bookmark = dynamic_cast<ZFlyEmBookmark*>(*iter);
      bodySet.insert(bookmark->getBodyId());
    }
  }

  return bodySet;
}


void ZFlyEmBodySplitProject::exportSplits()
{
  //ZObject3dScan body = *(getDataFrame()->document()->getSparseStack()->getObjectMask());

}

void ZFlyEmBodySplitProject::chopBodyX(int x, ZFlyEmSplitUploadOptionDialog *dlg)
{
  chopBody(x, neutu::EAxis::X, dlg);
}

void ZFlyEmBodySplitProject::chopBodyY(int y, ZFlyEmSplitUploadOptionDialog *dlg)
{
  chopBody(y, neutu::EAxis::Y, dlg);
}


void ZFlyEmBodySplitProject::chopBody(
    int v, neutu::EAxis axis, ZFlyEmSplitUploadOptionDialog *dlg)
{
#ifdef _FLYEM_
  ZFlyEmProofDoc* doc = getDocument<ZFlyEmProofDoc>();
  if (doc != NULL) {
    ZDvidWriter writer;
    if (writer.open(getDvidTarget())) {
      getProgressSignal()->startProgress("Slicing body");
      emitMessage("Uploading results ...");

      ZObject3dScan *wholeBody = doc->getBodyForSplit()->getObjectMask();

      getProgressSignal()->advanceProgress(0.1);
      if (wholeBody != NULL) {
        uint64_t newBodyId = 0;
        ZObject3dScan remain;
        ZObject3dScan subobj;

        wholeBody->chop(v, axis, &remain, &subobj);
//        wholeBody->chopZ(z, &remain, &subobj);
        size_t subobjVoxelNumber = subobj.getVoxelNumber();
        size_t remainVoxelNumber = remain.getVoxelNumber();
        size_t voxelNumber = 0;

        if (subobjVoxelNumber > 0 && remainVoxelNumber > 0) {
          //Keep the larger part
          if (subobjVoxelNumber <= remainVoxelNumber) {
            newBodyId = writer.writePartition(*wholeBody, subobj, getBodyId());
            *wholeBody = remain;
            voxelNumber = subobjVoxelNumber;
          } else {
            newBodyId = writer.writePartition(*wholeBody, remain, getBodyId());
            *wholeBody = subobj;
            voxelNumber = remainVoxelNumber;
          }


          getProgressSignal()->advanceProgress(0.1);

          std::vector<uint64_t> updateBodyArray;

          if (newBodyId > 0) {
            if (dlg != NULL) {
              ZFlyEmBodyAnnotation annot = dlg->getAnnotation(
                    getBodyId(), newBodyId);
              if (!annot.isEmpty()) {
                writer.writeBodyAnntation(annot);
              }
            }

            QString msg = QString("Cropped object uploaded as %1 (%2 voxels).").
                arg(newBodyId).arg(voxelNumber);
            if (voxelNumber >= m_skelThre) {
              updateBodyArray.push_back(newBodyId);
            }
            emitMessage(msg);

            std::vector<uint64_t> bodyArray;
            bodyArray.push_back(getBodyId());
            bodyArray.push_back(newBodyId);
            updateBodyDep(bodyArray);

//            GET_FLYEM_CONFIG.getNeutuService().requestBodyUpdate(
//                  getDvidTarget(), getBodyId(), ZNeutuService::UPDATE_ALL);
//            GET_FLYEM_CONFIG.getNeutuService().requestBodyUpdate(
//                  getDvidTarget(), newBodyId, ZNeutuService::UPDATE_ALL);

            QString bodyMessage = QString("Body %1 splitted: ").arg(getBodyId());
            bodyMessage += "<font color=#007700>";
            bodyMessage.append(QString("%1 ").arg(newBodyId));
            bodyMessage += "</font>";
            emitMessage(bodyMessage);

            getProgressSignal()->advanceProgress(0.1);

            updateSplitDocument();
            emit resultCommitted();
          } else {
            emitError("Warning: Something wrong happened during uploading! "
                      "Please contact the developer as soon as possible.");
          }
        }
      }

      getProgressSignal()->endProgress();

      emitMessage("Done.");
    }
  }
#endif
}

void ZFlyEmBodySplitProject::chopBodyZ(int z, ZFlyEmSplitUploadOptionDialog *dlg)
{
#ifdef _FLYEM_
  ZFlyEmProofDoc* doc = getDocument<ZFlyEmProofDoc>();
  if (doc != NULL) {
    ZDvidWriter writer;
    if (writer.open(getDvidTarget())) {
      getProgressSignal()->startProgress("Slicing body");
      emitMessage("Uploading results ...");

      ZObject3dScan *wholeBody = doc->getBodyForSplit()->getObjectMask();

      getProgressSignal()->advanceProgress(0.1);
      if (wholeBody != NULL) {
        uint64_t newBodyId = 0;
        ZObject3dScan remain;
        ZObject3dScan subobj;

        wholeBody->chopZ(z, &remain, &subobj);
        size_t subobjVoxelNumber = subobj.getVoxelNumber();
        size_t remainVoxelNumber = remain.getVoxelNumber();
        size_t voxelNumber = 0;

        if (subobjVoxelNumber > 0 && remainVoxelNumber > 0) {
          //Keep the larger part
          if (subobjVoxelNumber <= remainVoxelNumber) {
            newBodyId = writer.writePartition(*wholeBody, subobj, getBodyId());
            *wholeBody = remain;
            voxelNumber = subobjVoxelNumber;
          } else {
            newBodyId = writer.writePartition(*wholeBody, remain, getBodyId());
            *wholeBody = subobj;
            voxelNumber = remainVoxelNumber;
          }


          getProgressSignal()->advanceProgress(0.1);

          std::vector<uint64_t> updateBodyArray;

          if (newBodyId > 0) {
            if (dlg != NULL) {
              ZFlyEmBodyAnnotation annot = dlg->getAnnotation(
                    getBodyId(), newBodyId);
              if (!annot.isEmpty()) {
                writer.writeBodyAnntation(annot);
              }
            }

            QString msg = QString("Cropped object uploaded as %1 (%2 voxels).").
                arg(newBodyId).arg(voxelNumber);
            if (voxelNumber >= m_skelThre) {
              updateBodyArray.push_back(newBodyId);
            }
            emitMessage(msg);

            updateBodyDep(getBodyId(), newBodyId);
            /*
            GET_FLYEM_CONFIG.getNeutuService().requestBodyUpdate(
                  getDvidTarget(), getBodyId(), ZNeutuService::UPDATE_ALL);
            GET_FLYEM_CONFIG.getNeutuService().requestBodyUpdate(
                  getDvidTarget(), newBodyId, ZNeutuService::UPDATE_ALL);
                  */

            QString bodyMessage = QString("Body %1 splitted: ").arg(getBodyId());
            bodyMessage += "<font color=#007700>";
            bodyMessage.append(QString("%1 ").arg(newBodyId));
            bodyMessage += "</font>";
            emitMessage(bodyMessage);

            getProgressSignal()->advanceProgress(0.1);

            updateSplitDocument();
            emit resultCommitted();
          } else {
            emitError("Warning: Something wrong happened during uploading! "
                      "Please contact the developer as soon as possible.");
          }
        }
      }

      getProgressSignal()->endProgress();

      emitMessage("Done.");
    }
  }
#endif
}

void ZFlyEmBodySplitProject::cropBody(ZFlyEmSplitUploadOptionDialog *dlg)
{
#ifdef _FLYEM_
  ZFlyEmProofDoc* doc = getDocument<ZFlyEmProofDoc>();
  if (doc != NULL) {
    ZDvidWriter writer;
    if (writer.open(getDvidTarget())) {
      getProgressSignal()->startProgress("Cropping body");
      emitMessage("Uploading results ...");

      ZObject3dScan *wholeBody = doc->getBodyForSplit()->getObjectMask();

      getProgressSignal()->advanceProgress(0.1);

      ZIntCuboidObj *box = doc->getSplitRoi();
      if (wholeBody != NULL) {
        uint64_t newBodyId = 0;
        ZObject3dScan remain;
        ZObject3dScan subobj;
        wholeBody->subobject(box->getCuboid(), &remain, &subobj);
        if (!subobj.isEmpty()) {
          newBodyId = writer.writePartition(*wholeBody, subobj, getBodyId());
        }

        getProgressSignal()->advanceProgress(0.1);

        std::vector<uint64_t> updateBodyArray;

        if (newBodyId > 0) {
          if (dlg != NULL) {
            ZFlyEmBodyAnnotation annot = dlg->getAnnotation(
                  getBodyId(), newBodyId);
            if (!annot.isEmpty()) {
              writer.writeBodyAnntation(annot);
            }
          }

          *wholeBody = remain;
          size_t voxelNumber = subobj.getVoxelNumber();
          QString msg = QString("Cropped object uploaded as %1 (%2 voxels).").
              arg(newBodyId).arg(voxelNumber);
          if (voxelNumber >= m_skelThre) {
            updateBodyArray.push_back(newBodyId);
          }
          emitMessage(msg);

          updateBodyDep(getBodyId(), newBodyId);

          /*
          GET_FLYEM_CONFIG.getNeutuService().requestBodyUpdate(
                getDvidTarget(), getBodyId(), ZNeutuService::UPDATE_ALL);
          GET_FLYEM_CONFIG.getNeutuService().requestBodyUpdate(
                getDvidTarget(), newBodyId, ZNeutuService::UPDATE_ALL);
*/

          QString bodyMessage = QString("Body %1 splitted: ").arg(getBodyId());
          bodyMessage += "<font color=#007700>";
          bodyMessage.append(QString("%1 ").arg(newBodyId));
          bodyMessage += "</font>";
          emitMessage(bodyMessage);

          getProgressSignal()->advanceProgress(0.1);

          updateSplitDocument();
          emit resultCommitted();
        } else {
          emitError("Warning: Something wrong happened during uploading! "
                    "Please contact the developer as soon as possible.");
        }
      }

      getProgressSignal()->endProgress();

      emitMessage("Done.");
    }
  }
#endif
}

void ZFlyEmBodySplitProject::decomposeBody(ZFlyEmSplitUploadOptionDialog *dlg)
{
  getProgressSignal()->startProgress("Decomposing body");
  emitMessage("Uploading results ...");

  ZObject3dScan *wholeBody = NULL;
  if (getDocument<ZFlyEmProofDoc>() != NULL) {
    ZFlyEmProofDoc* doc = getDocument<ZFlyEmProofDoc>();
    wholeBody = doc->getBodyForSplit()->getObjectMask();
  } else {
    wholeBody = getDocument()->getSparseStackMask();
  }

  getProgressSignal()->advanceProgress(0.1);
  emitMessage(QString("Identifying isolated objects ..."));
  std::vector<ZObject3dScan> objArray =
      wholeBody->getConnectedComponent(ZObject3dScan::ACTION_NONE);
  getProgressSignal()->advanceProgress(0.2);

  QList<uint64_t> newBodyIdList;

  ZDvidWriter writer;
  writer.open(getDvidTarget());

  std::vector<uint64_t> updateBodyArray;

  if (objArray.size() > 1) {
    double dp = 0.5 / objArray.size();
    size_t maxIndex = 0;
    size_t maxSize = 0;
    size_t index = 0;
    for (std::vector<ZObject3dScan>::const_iterator iter = objArray.begin();
         iter != objArray.end(); ++iter, ++index) {
      const ZObject3dScan &obj = *iter;
      if (obj.getVoxelNumber() > maxSize) {
        maxSize = obj.getVoxelNumber();
        maxIndex = index;
      }
    }

    index = 0;
    for (std::vector<ZObject3dScan>::iterator iter = objArray.begin();
         iter != objArray.end(); ++iter, ++index) {
      if (index != maxIndex) {
        ZObject3dScan &obj = *iter;
        obj.canonize();
        wholeBody->subtractSliently(obj);
        uint64_t newBodyId = writer.writePartition(*wholeBody, obj, getBodyId());
        QString msg;
        if (newBodyId > 0) {
          size_t voxelNumber = obj.getVoxelNumber();
          msg = QString("Isolated object uploaded as %1 (%2 voxels) .").
              arg(newBodyId).arg(voxelNumber);
          if (voxelNumber >= m_skelThre) {
            updateBodyArray.push_back(newBodyId);
          }
          newBodyIdList.append(newBodyId);

          if (dlg != NULL) {
            ZFlyEmBodyAnnotation annot = dlg->getAnnotation(
                  getBodyId(), newBodyId);
            if (!annot.isEmpty()) {
              writer.writeBodyAnntation(annot);
            }
          }

          emitMessage(msg);
        } else {
          emitError("Warning: Something wrong happened during uploading! "
                    "Please contact the developer as soon as possible.");
        }
      }

      getProgressSignal()->advanceProgress(dp);
    }
  } else {
    emitMessage("No isolation found.");
  }

#if defined(_FLYEM_)
  if (!newBodyIdList.isEmpty()) {
    updateBodyDep(wholeBody->getLabel());
    updateBodyDep(updateBodyArray);
    /*
    GET_FLYEM_CONFIG.getNeutuService().requestBodyUpdate(
          getDvidTarget(), wholeBody->getLabel(), ZNeutuService::UPDATE_ALL);
    GET_FLYEM_CONFIG.getNeutuService().requestBodyUpdate(
          getDvidTarget(), updateBodyArray, ZNeutuService::UPDATE_ALL);
*/
    QString bodyMessage = QString("Body %1 splitted: ").arg(wholeBody->getLabel());
    bodyMessage += "<font color=#007700>";
    foreach (uint64_t bodyId, newBodyIdList) {
      bodyMessage.append(QString("%1 ").arg(bodyId));
    }
    bodyMessage += "</font>";
    emitMessage(bodyMessage);
  }
#endif

  updateSplitDocument();

  getProgressSignal()->endProgress();

  emitMessage("Done.");
  emit resultCommitted();
}

void ZFlyEmBodySplitProject::resetStatusAfterUpload()
{
  deleteSavedSeed();
  getDocument()->undoStack()->clear();
  if (keepingMainSeed()) {
    removeAllSideSeed();
  } else {
    removeAllSeed();
  }

  getDocument()->setSegmentationReady(false);
  getProgressSignal()->endProgress();
}

void ZFlyEmBodySplitProject::commitResult()
{
  getProgressSignal()->startProgress("Saving splits");

//  getProgressSignal()->startProgress(0.8);

  m_cancelSplitQuick = true;
  m_splitUpdated = true;
  m_futureMap.waitForFinished();

  if (getDocument()->getLabelField() != NULL) {
    commitResultFunc(
          getDocument()->getSparseStackMask(),
          getDocument()->getLabelField(),
          getMinObjSize());
  } else {
    QList<ZStackObject*> objList =
        getDocument()->getObjectList(ZStackObjectRole::ROLE_SEGMENTATION);
    std::vector<ZObject3dScan*> objArray;
    foreach (ZStackObject *obj, objList) {
      ZObject3dScan *tmpObj = dynamic_cast<ZObject3dScan*>(obj);
      if (tmpObj != NULL) {
        objArray.push_back(tmpObj);
      }
    }
    commitResultFunc(
          getDocument()->getSparseStackMask(), objArray,
          getMinObjSize(), getDocument()->hadSegmentationDownsampled());
  }
//  getProgressSignal()->endProgress();



//  getProgressSignal()->endProgress();

  /*
  QtConcurrent::run(this, &ZFlyEmBodySplitProject::commitResultFunc,
                    getDataFrame()->document()->getSparseStack()->getObjectMask(),
                    getDataFrame()->document()->getLabelField(),
                    getDataFrame()->document()->getSparseStack()->getDownsampleInterval());
                    */
}

static void prepareBodyUpload(const ZObject3dScan &obj,
                              QList<ZObject3dScan> &objList,
                              QList<uint64_t> &oldBodyIdList, uint64_t label)
{
  objList.append(obj);
  oldBodyIdList.append(label);
}

void ZFlyEmBodySplitProject::updateSplitDocument()
{
  if (getDocument<ZFlyEmProofDoc>() != NULL) {
    getDocument<ZFlyEmProofDoc>()->deprecateSplitSource();
  }
}

void ZFlyEmBodySplitProject::commitCoarseSplit(const ZObject3dScan &splitPart)
{
  ZDvidWriter writer;
  if (writer.open(getDvidTarget())) {
    emitMessage("Uploading crop result ...");
    uint64_t bodyId = writer.writeCoarseSplit(
          splitPart, getBodyId());
    if (bodyId == 0) {
      emit messageGenerated(
            ZWidgetMessage(QString("Split %1 failed.").
                           arg(getBodyId()),
                           neutu::EMessageType::ERROR));
    } else {
      updateBodyDep(getBodyId(), bodyId);
#if defined(_FLYEM_2)
      GET_FLYEM_CONFIG.getNeutuService().requestBodyUpdate(
            getDvidTarget(), getBodyId(), ZNeutuService::UPDATE_ALL);
      GET_FLYEM_CONFIG.getNeutuService().requestBodyUpdate(
            getDvidTarget(), bodyId, ZNeutuService::UPDATE_ALL);
#endif
      updateSplitDocument();
      emitMessage(QString("Done. The cropped part has bodyId %1").arg(bodyId));
      emit resultCommitted();
    }
  }
}

void ZFlyEmBodySplitProject::processSmallBodyGroup(
    ZObject3dScan *body, size_t minObjSize, ZObject3dScan *smallBodyGroup)
{
  if (minObjSize > 0 && body != NULL) { //Isolated objects from the original body
    emitMessage(QString("Identifying isolated objects ..."));
    std::vector<ZObject3dScan> objArray =
        body->getConnectedComponent(ZObject3dScan::ACTION_NONE);
    if (objArray.size() > 1) {
      body->clear();
      for (std::vector<ZObject3dScan>::const_iterator iter = objArray.begin();
           iter != objArray.end(); ++iter) {
        const ZObject3dScan &obj = *iter;
        if (obj.getVoxelNumber() < minObjSize) {
          smallBodyGroup->concat(obj);
        } else {
          body->concat(obj);
        }
      }
    }
    LINFO() << "#Small body group voxels:" << smallBodyGroup->getVoxelNumber();
  }
}

void ZFlyEmBodySplitProject::processIsolation(
    ZObject3dScan &currentBody, ZObject3dScan *body,
    QList<ZObject3dScan> &splitList, QList<uint64_t> &oldBodyIdList,
    const ZObject3dScan *obj, size_t minIsolationSize)
{
  std::vector<ZObject3dScan> objArray =
      currentBody.getConnectedComponent(ZObject3dScan::ACTION_NONE);
  if (objArray.empty()) {
    emitError("Warning: Empty split detected after connect component analysis.");
  }
  for (std::vector<ZObject3dScan>::iterator iter = objArray.begin();
       iter != objArray.end(); ++iter) {
    ZObject3dScan &subobj = *iter;
    bool isAdopted = false;
    if (subobj.getVoxelNumber() < minIsolationSize &&
        currentBody.getVoxelNumber() / subobj.getVoxelNumber() > 10) {
      if (body->isAdjacentTo(subobj)) {
        body->concat(subobj);
        isAdopted = true;
      }
    }

    if (!isAdopted) {
      prepareBodyUpload(
            subobj, splitList, oldBodyIdList, obj->getLabel());
    }
  }
}

ZDvidReader& ZFlyEmBodySplitProject::getCommitReader()
{
  if (!m_commitReader.good()) {
    m_commitReader.openRaw(m_reader.getDvidTarget());
  }

  return m_commitReader;
}

ZDvidWriter& ZFlyEmBodySplitProject::getCommitWriter()
{
  if (!m_commitWriter.good()) {
    m_commitWriter.openRaw(m_reader.getDvidTarget());
  }

  return m_commitWriter;
}

ZDvidWriter& ZFlyEmBodySplitProject::getMainWriter()
{
  if (!m_writer.good()) {
    m_writer.openRaw(m_reader.getDvidTarget());
  }

  return m_writer;
}

void ZFlyEmBodySplitProject::updateBodyDep(uint64_t bodyId1, uint64_t bodyId2)
{
  std::vector<uint64_t> bodyArray;
  bodyArray.push_back(bodyId1);
  bodyArray.push_back(bodyId2);
  updateBodyDep(bodyArray);
}

void ZFlyEmBodySplitProject::updateBodyDep(uint64_t bodyId)
{
  std::vector<uint64_t> bodyArray;
  bodyArray.push_back(bodyId);
  updateBodyDep(bodyArray);
}

void ZFlyEmBodySplitProject::updateBodyDep(
    const std::vector<uint64_t> &bodyArray)
{
#if defined(_FLYEM_)
    if (GET_FLYEM_CONFIG.getNeutuseWriter().ready()) { //Use new server
      for (uint64_t bodyId : bodyArray) {
        neutuse::Task task = neutuse::TaskFactory::MakeDvidTask(
              "skeletonize", getDvidTarget(), bodyId, true);
        task.setPriority(5);
        GET_FLYEM_CONFIG.getNeutuseWriter().uploadTask(task);
      }
    } else if (GET_FLYEM_CONFIG.getNeutuService().isNormal()) {
      GET_FLYEM_CONFIG.getNeutuService().requestBodyUpdate(
            getDvidTarget(), bodyArray, ZNeutuService::UPDATE_ALL);
    }
#endif
}

void ZFlyEmBodySplitProject::commitResultFunc(
    ZObject3dScan *wholeBody, const std::vector<ZObject3dScan *> &objArray,
    size_t minObjSize, bool checkingIslotation)
{
  getProgressSignal()->startProgress("Uploading splitted bodies");

  getProgressSignal()->startProgress(0.8);

  emitMessage("Uploading results ...");


  ZObject3dScan body = *wholeBody;


//  ZOUT(LINFO(), 3) << "Label field ds: " << dsIntv.toString();

  getProgressSignal()->advanceProgress(0.1);

  ZObject3dScan smallBodyGroup;
  processSmallBodyGroup(&body, minObjSize, &smallBodyGroup);

  getProgressSignal()->advanceProgress(0.1);

#ifdef _DEBUG_2
    body.save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

//  int maxNum = 1;
//  QStringList filePathList;
  m_splitList.clear();
  m_oldBodyIdList.clear();

//  QList<ZObject3dScan> splitList;
//  QList<uint64_t> oldBodyIdList;

  ZObject3dScan mainBody;

  emitMessage(QString("Processing splits ..."));
  if (!objArray.empty()) { //Process splits
//    std::vector<ZObject3dScan*> objArray =
//        ZObject3dScan::extractAllObject(*labelField);
    emitMessage(QString("%1 labels extracted.").arg(objArray.size()));
    QString sizeMessage = "Object sizes: ";
    for (ZObject3dScan *obj : objArray) {
      sizeMessage +=
          QString("%1: %2; ").arg(obj->getLabel()).arg(obj->getVoxelNumber());
    }
    emitMessage(sizeMessage);

    getProgressSignal()->advanceProgress(0.1);

    double dp = 0.2;

    if (!objArray.empty()) {
      dp = 0.2 / objArray.size();
    }

//    std::vector<ZObject3dScan> mainObjectArray;
//    std::vector<ZObject3dScan> minorObjectArray;
    size_t minIsolationSize = 50;


    for (std::vector<ZObject3dScan*>::const_iterator iter = objArray.begin();
         iter != objArray.end(); ++iter) {
      const ZObject3dScan *obj = *iter;
//      obj->upSample(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());??
      if (obj->getLabel() > 1) {
//        obj->upSample(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());

        /* Subtract a split body.
         * currentBody is the one to split; body becomes the remained part */
        ZObject3dScan currentBody = body.subtract(*obj);

        if (currentBody.isEmpty()) {
          emitError("Warning: Empty split detected.");
        } else {
          if (checkingIslotation) { //Check isolations caused by downsampling
            processIsolation(
                  currentBody, &body, m_splitList, m_oldBodyIdList, obj,
                  minIsolationSize);
          } else {
            prepareBodyUpload(
                  currentBody, m_splitList, m_oldBodyIdList, obj->getLabel());
//            prepareBodyUpload(currentBody, filePathList, oldBodyIdList, maxNum,
//                              getBodyId(), obj->getLabel());
          }
        }
      } else {
        mainBody.concat(*obj);
      }
//      delete obj;

      getProgressSignal()->advanceProgress(dp);
    }
  }

  if (!body.isEmpty() && m_runningCca /*&& minObjSize > 0*/) { //Check isolated objects after split
    std::vector<ZObject3dScan> objArray =
        body.getConnectedComponent(ZObject3dScan::ACTION_NONE);

#ifdef _DEBUG_2
    body.save(GET_TEST_DATA_DIR + "/test2.sobj");
#endif

    double dp = 0.2;

    if (!objArray.empty()) {
      dp = 0.2 / objArray.size();
    }

//    mainBody.upSample(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());
    for (std::vector<ZObject3dScan>::iterator iter = objArray.begin();
         iter != objArray.end(); ++iter) {
      ZObject3dScan &obj = *iter;

      //Check single connect for bound box split
      //Any component only connected to one split part?
      int count = 0;
      int splitIndex = 0;

      if (!obj.isAdjacentTo(mainBody)) {
        for (int index = 0; index < m_splitList.size(); ++index) {
          if (obj.isAdjacentTo(m_splitList[index])) {
            ++count;
            if (count > 1) {
              break;
            }
            splitIndex = index;
          }
        }
      }

      if (count == 1) {
        ZObject3dScan &split = m_splitList[splitIndex];
        split.concat(obj);
      } else {
        if (obj.getVoxelNumber() < minObjSize) {
          smallBodyGroup.concat(obj);
        }
      }

      getProgressSignal()->advanceProgress(dp);
    }
  }

  if (!smallBodyGroup.isEmpty()) {
    prepareBodyUpload(
          smallBodyGroup, m_splitList, m_oldBodyIdList, 0);
    //prepareBodyUpload(smallBodyGroup, filePathList, oldBodyIdList, maxNum,
    //                  getBodyId(), 0);
  }

//  ZDvidReader &reader = getCommitReader();
//  reader.open(m_dvidTarget);
//  int bodyId = reader.readMaxBodyId();

  getProgressSignal()->endProgress();

  emit splitListGenerated();
//  uploadSplitListFunc();

#if 0
  QList<uint64_t> newBodyIdList;

  ZDvidWriter &writer = getCommitWriter();
//  writer.open(getDvidTarget());

  size_t skelThre = 20;
  int bodyIndex = 0;
  std::vector<uint64_t> updateBodyArray;

  foreach (const ZObject3dScan &obj, splitList) {
    wholeBody->subtractSliently(obj);

    uint64_t newBodyId = writer.writePartition(*wholeBody, obj, getBodyId());
//    uint64_t newBodyId = writer.writeSplit(obj, getBodyId(), 0);
    ++bodyIndex;

    uint64_t oldBodyId = oldBodyIdList[bodyIndex - 1];
    QString msg;
    if (newBodyId > 0) {
      size_t voxelNumber = obj.getVoxelNumber();
      if (oldBodyId > 0) {
        msg = QString("Label %1 uploaded as %2 (%3 voxels).").
            arg(oldBodyId).arg(newBodyId).arg(voxelNumber);
      } else {
        msg = QString("Isolated object uploaded as %1 (%2 voxels) .").
            arg(newBodyId).arg(voxelNumber);
      }
      newBodyIdList.append(newBodyId);

      if (voxelNumber >= skelThre) {
        updateBodyArray.push_back(newBodyId);
      }

      ZFlyEmBodyAnnotation annot;
      annot.setBodyId(newBodyId);
//      annot.setStatus("Not examined");
      writer.writeBodyAnntation(annot);

      emitMessage(msg);
    } else {
      emitError("Warning: Something wrong happened during uploading! "
                "Please contact the developer as soon as possible.");
    }


    getProgressSignal()->advanceProgress(dp);
//    emit progressAdvanced(dp);
  }

  if (!newBodyIdList.isEmpty()) {
    QString bodyMessage = QString("Body %1 splitted: ").arg(wholeBody->getLabel());
    std::vector<uint64_t> bodyArray = updateBodyArray;
    bodyArray.push_back(wholeBody->getLabel());
    updateBodyDep(bodyArray);

#if defined(_FLYEM_2)
    if (GET_FLYEM_CONFIG.getNeutuseWriter().ready()) { //Use new server
      std::vector<uint64_t> bodyArray = updateBodyArray;
      bodyArray.push_back(wholeBody->getLabel());
      for (uint64_t bodyId : bodyArray) {
        neutuse::Task task = neutuse::TaskFactory::MakeDvidTask(
              "skeletonize", getDvidTarget(), bodyId, true);
        GET_FLYEM_CONFIG.getNeutuseWriter().uploadTask(task);
      }
    } else if (GET_FLYEM_CONFIG.getNeutuService().isNormal()) {
      GET_FLYEM_CONFIG.getNeutuService().requestBodyUpdate(
            getDvidTarget(), wholeBody->getLabel(), ZNeutuService::UPDATE_ALL);
      GET_FLYEM_CONFIG.getNeutuService().requestBodyUpdate(
            getDvidTarget(), updateBodyArray, ZNeutuService::UPDATE_ALL);
    }
#endif

    bodyMessage += "<font color=#007700>";
    foreach (uint64_t bodyId, newBodyIdList) {
      bodyMessage.append(QString("%1 ").arg(bodyId));
    }
    bodyMessage += "</font>";
    emitMessage(bodyMessage);
  }

  getProgressSignal()->endProgress();
  //writer.writeMaxBodyId(bodyId);

  updateSplitDocument();

//  emit progressDone();
  emitMessage("Done.");
  emit resultCommitted();
#endif
}

void ZFlyEmBodySplitProject::uploadSplitList()
{
  if (splitSizeChecked()) {
    QtConcurrent::run(this, &ZFlyEmBodySplitProject::uploadSplitListFunc);
  } else {

    getProgressSignal()->endProgress();
  }
}

QWidget* ZFlyEmBodySplitProject::getParentWidget() const
{
  return qobject_cast<QWidget*>(parent());
}

bool ZFlyEmBodySplitProject::splitSizeChecked() const
{
  ZObject3dScan *wholeBody = getDocument()->getSparseStackMask();
  size_t mainBodySize = wholeBody->getVoxelNumber();
  size_t maxSplitSize = 0;
//  std::vector<size_t> splitSizeList(m_splitList.size(), 0);
  for (int i = 0; i < m_splitList.size(); ++i) {
    const ZObject3dScan &obj = m_splitList[i];
    size_t splitSize = obj.getVoxelNumber();
    if (maxSplitSize < splitSize) {
      maxSplitSize = splitSize;
    }
//    splitSizeList[i] = obj.getVoxelNumber();
    mainBodySize -= splitSize;
  }

  //80%
  if (mainBodySize < (maxSplitSize / 5) * 4) {
    return ZDialogFactory::Ask(
          "Uploading Split",
          "It looks like you're splitting off a big part of the body. "
          "Do you want to continue?", getParentWidget());
  }

  return true;
}

void ZFlyEmBodySplitProject::uploadSplitListFunc()
{
  if (m_splitList.empty()) {
    emitError("Warning: No splits generated for upload! "
              "Please contact the developer as soon as possible.");
    getProgressSignal()->endProgress();
  } else {
    double dp = 0.2 / m_splitList.size();

    QList<uint64_t> newBodyIdList;

    ZDvidWriter &writer = getCommitWriter();
    //  writer.open(getDvidTarget());

    size_t skelThre = 20;
    int bodyIndex = 0;


    std::vector<uint64_t> updateBodyArray;
    ZObject3dScan *wholeBody = getDocument()->getSparseStackMask();

    foreach (const ZObject3dScan &obj, m_splitList) {
      wholeBody->subtractSliently(obj);

      uint64_t newBodyId = writer.writePartition(*wholeBody, obj, getBodyId());
      //    uint64_t newBodyId = writer.writeSplit(obj, getBodyId(), 0);
      ++bodyIndex;

      uint64_t oldBodyId = m_oldBodyIdList[bodyIndex - 1];
      QString msg;
      if (newBodyId > 0) {
        size_t voxelNumber = obj.getVoxelNumber();
        if (oldBodyId > 0) {
          msg = QString("Label %1 uploaded as %2 (%3 voxels).").
              arg(oldBodyId).arg(newBodyId).arg(voxelNumber);
        } else {
          msg = QString("Isolated object uploaded as %1 (%2 voxels) .").
              arg(newBodyId).arg(voxelNumber);
        }
        newBodyIdList.append(newBodyId);

        if (voxelNumber >= skelThre) {
          updateBodyArray.push_back(newBodyId);
        }

        ZFlyEmBodyAnnotation annot;
        annot.setBodyId(newBodyId);
        //      annot.setStatus("Not examined");
        writer.writeBodyAnntation(annot);

        emitMessage(msg);
      } else {
        emitError("Warning: Something wrong happened during uploading! "
                  "Please contact the developer as soon as possible.");
      }


      getProgressSignal()->advanceProgress(dp);
      //    emit progressAdvanced(dp);
    }

    if (!newBodyIdList.isEmpty()) {
      QString bodyMessage = QString("Body %1 splitted: ").arg(wholeBody->getLabel());
      std::vector<uint64_t> bodyArray = updateBodyArray;
      bodyArray.push_back(wholeBody->getLabel());
      updateBodyDep(bodyArray);

      bodyMessage += "<font color=#007700>";
      foreach (uint64_t bodyId, newBodyIdList) {
        bodyMessage.append(QString("%1 ").arg(bodyId));
      }
      bodyMessage += "</font>";
      emitMessage(bodyMessage);
    }
    //writer.writeMaxBodyId(bodyId);

    updateSplitDocument();
    emit resultCommitted();
  }

//  getProgressSignal()->endProgress();
//  emit progressDone();
  emitMessage("Done.");
}

void ZFlyEmBodySplitProject::commitResultFunc(
    ZObject3dScan *wholeBody, const ZStack *labelField,
    size_t minObjSize)
{
  getProgressSignal()->startProgress("Uploading splitted bodies");

  emitMessage("Uploading results ...");

//  const ZObject3dScan *wholeBody =
//      getDataFrame()->document()->getSparseStack()->getObjectMask();

  ZObject3dScan body = *wholeBody;

  ZIntPoint dsIntv = labelField->getDsIntv();

  ZOUT(LINFO(), 3) << "Label field ds: " << dsIntv.toString();

//  size_t minObjSize = 20;

  getProgressSignal()->advanceProgress(0.1);

  ZObject3dScan smallBodyGroup;

  if (minObjSize > 0) { //Isolated objects from the original body
    emitMessage(QString("Identifying isolated objects ..."));
    std::vector<ZObject3dScan> objArray =
        body.getConnectedComponent(ZObject3dScan::ACTION_NONE);
    if (objArray.size() > 1) {
      body.clear();
      for (std::vector<ZObject3dScan>::const_iterator iter = objArray.begin();
           iter != objArray.end(); ++iter) {
        const ZObject3dScan &obj = *iter;
        if (obj.getVoxelNumber() < minObjSize) {
          smallBodyGroup.concat(obj);
        } else {
          body.concat(obj);
        }
      }
    }
    LINFO() << "#Small body group voxels:" << smallBodyGroup.getVoxelNumber();
  }

  getProgressSignal()->advanceProgress(0.1);

#ifdef _DEBUG_2
    body.save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

//  int maxNum = 1;
  QStringList filePathList;
  QList<ZObject3dScan> splitList;
  QList<uint64_t> oldBodyIdList;

  ZObject3dScan mainBody;

  emitMessage(QString("Processing splits ..."));
  if (labelField != NULL) { //Process splits
    std::vector<ZObject3dScan*> objArray =
        ZObject3dScan::extractAllObject(*labelField);
    emitMessage(QString("%1 labels extracted.").arg(objArray.size()));
    QString sizeMessage = "Object sizes: ";
    for (std::vector<ZObject3dScan*>::const_iterator iter = objArray.begin();
         iter != objArray.end(); ++iter) {
      const ZObject3dScan *obj = *iter;
      sizeMessage +=
          QString("%1: %2; ").arg(obj->getLabel()).arg(obj->getVoxelNumber());
    }
    emitMessage(sizeMessage);
#ifdef _DEBUG_2
    stack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

//    emit progressAdvanced(0.1);
    getProgressSignal()->advanceProgress(0.1);

    double dp = 0.2;

    if (!objArray.empty()) {
      dp = 0.2 / objArray.size();
    }

//    std::vector<ZObject3dScan> mainObjectArray;
//    std::vector<ZObject3dScan> minorObjectArray;
    size_t minIsolationSize = 50;


    for (std::vector<ZObject3dScan*>::iterator iter = objArray.begin();
         iter != objArray.end(); ++iter) {
      ZObject3dScan *obj = *iter;
//      obj->upSample(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());??
      if (obj->getLabel() > 1) {
        obj->upSample(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());

        /* Subtract a split body.
         * currentBody is the one to split; body becomes the remained part */
        ZObject3dScan currentBody = body.subtract(*obj);

        if (currentBody.isEmpty()) {
          emitError("Warning: Empty split detected.");
        } else {
          if (!dsIntv.isZero()) { //Check isolations caused by downsampling
            std::vector<ZObject3dScan> objArray =
                currentBody.getConnectedComponent(ZObject3dScan::ACTION_NONE);
            if (objArray.empty()) {
              emitError("Warning: Empty split detected after connect component analysis.");
            }
            for (std::vector<ZObject3dScan>::iterator iter = objArray.begin();
                 iter != objArray.end(); ++iter) {
              ZObject3dScan &subobj = *iter;
              bool isAdopted = false;
              if (subobj.getVoxelNumber() < minIsolationSize &&
                  currentBody.getVoxelNumber() / subobj.getVoxelNumber() > 10) {
                if (body.isAdjacentTo(subobj)) {
                  body.concat(subobj);
                  isAdopted = true;
                }
              }

              if (!isAdopted) {
                prepareBodyUpload(
                      subobj, splitList, oldBodyIdList, obj->getLabel());
//                prepareBodyUpload(subobj, filePathList, oldBodyIdList, maxNum,
//                                  getBodyId(), obj->getLabel());
              }
            }
          } else {
            prepareBodyUpload(
                  currentBody, splitList, oldBodyIdList, obj->getLabel());
//            prepareBodyUpload(currentBody, filePathList, oldBodyIdList, maxNum,
//                              getBodyId(), obj->getLabel());
          }
        }
      } else {
        mainBody.concat(*obj);
      }
      delete obj;

      getProgressSignal()->advanceProgress(dp);
//      emit progressAdvanced(dp);
    }
  }

  if (!body.isEmpty() && m_runningCca /*&& minObjSize > 0*/) { //Check isolated objects after split
    std::vector<ZObject3dScan> objArray =
        body.getConnectedComponent(ZObject3dScan::ACTION_NONE);

#ifdef _DEBUG_2
    body.save(GET_TEST_DATA_DIR + "/test2.sobj");
#endif

    double dp = 0.2;

    if (!objArray.empty()) {
      dp = 0.2 / objArray.size();
    }

    mainBody.upSample(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());
    for (std::vector<ZObject3dScan>::iterator iter = objArray.begin();
         iter != objArray.end(); ++iter) {
      ZObject3dScan &obj = *iter;

      //Check single connect for bound box split
      //Any component only connected to one split part?
      int count = 0;
      int splitIndex = 0;

      if (!obj.isAdjacentTo(mainBody)) {
        for (int index = 0; index < splitList.size(); ++index) {
          if (obj.isAdjacentTo(splitList[index])) {
            ++count;
            if (count > 1) {
              break;
            }
            splitIndex = index;
          }
        }
      }

      if (count == 1) {
        ZObject3dScan &split = splitList[splitIndex];
        split.concat(obj);
      } else {
        if (obj.getVoxelNumber() < minObjSize) {
          smallBodyGroup.concat(obj);
        }
      }

      getProgressSignal()->advanceProgress(dp);
    }
  }

  if (!smallBodyGroup.isEmpty()) {
    prepareBodyUpload(
          smallBodyGroup, splitList, oldBodyIdList, 0);
    //prepareBodyUpload(smallBodyGroup, filePathList, oldBodyIdList, maxNum,
    //                  getBodyId(), 0);
  }

//  ZDvidReader reader;
//  reader.open(m_dvidTarget);
//  int bodyId = reader.readMaxBodyId();

  int bodyIndex = 0;

  double dp = 0.2;

  if (!splitList.empty()) {
    dp = 0.2 / filePathList.size();
  } else {
    emitError("Warning: No splits generated for upload! "
              "Please contact the developer as soon as possible.");
  }

  /*
  if (!filePathList.empty()) {
    dp = 0.2 / filePathList.size();
  } else {
    emitError("Warning: No splits generated for upload! "
              "Please contact the developer as soon as possible.");
  }
  */

  QList<uint64_t> newBodyIdList;

  ZDvidWriter &writer = getCommitWriter();
//  writer.open(getDvidTarget());

  size_t skelThre = 20;

  std::vector<uint64_t> updateBodyArray;

//  foreach (QString objFile, filePathList) {
  foreach (const ZObject3dScan &obj, splitList) {
//    const ZObject3dScan &obj = splitList
//    ZObject3dScan obj;
//    obj.load(objFile.toStdString());
    /*
    uint64_t newBodyId = writer.writeSplitMultires(*wholeBody, obj, getBodyId());
    ++bodyIndex;
    */

    /*
    uint64_t newBodyId = writer.writeSplit(
          getDvidTarget().getBodyLabelName(), obj, getBodyId(), ++bodyIndex);
          */

    wholeBody->subtractSliently(obj);

//    uint64_t newBodyId = writer.writePartition(*wholeBody, obj, getBodyId());

    uint64_t newBodyId = writer.writeSplit(obj, getBodyId(), 0);

    ++bodyIndex;

    uint64_t oldBodyId = oldBodyIdList[bodyIndex - 1];
    QString msg;
    if (newBodyId > 0) {
      size_t voxelNumber = obj.getVoxelNumber();
      if (oldBodyId > 0) {
        msg = QString("Label %1 uploaded as %2 (%3 voxels).").
            arg(oldBodyId).arg(newBodyId).arg(voxelNumber);
      } else {
        msg = QString("Isolated object uploaded as %1 (%2 voxels) .").
            arg(newBodyId).arg(voxelNumber);
      }
      newBodyIdList.append(newBodyId);

      if (voxelNumber >= skelThre) {
        updateBodyArray.push_back(newBodyId);
      }

      ZFlyEmBodyAnnotation annot;
      annot.setBodyId(newBodyId);
//      annot.setStatus("Not examined");
      writer.writeBodyAnntation(annot);

      emitMessage(msg);
    } else {
      emitError("Warning: Something wrong happened during uploading! "
                "Please contact the developer as soon as possible.");
    }


    getProgressSignal()->advanceProgress(dp);
//    emit progressAdvanced(dp);
  }

  if (!newBodyIdList.isEmpty()) {
    QString bodyMessage = QString("Body %1 splitted: ").arg(wholeBody->getLabel());

    updateBodyDep(wholeBody->getLabel());
    updateBodyDep(updateBodyArray);

#if defined(_FLYEM_2)
    GET_FLYEM_CONFIG.getNeutuService().requestBodyUpdate(
          getDvidTarget(), wholeBody->getLabel(), ZNeutuService::UPDATE_ALL);
    GET_FLYEM_CONFIG.getNeutuService().requestBodyUpdate(
          getDvidTarget(), updateBodyArray, ZNeutuService::UPDATE_ALL);
#endif

    bodyMessage += "<font color=#007700>";
    foreach (uint64_t bodyId, newBodyIdList) {
      bodyMessage.append(QString("%1 ").arg(bodyId));
    }
    bodyMessage += "</font>";
    emitMessage(bodyMessage);
  }

  getProgressSignal()->endProgress();
  //writer.writeMaxBodyId(bodyId);

  updateSplitDocument();

//  emit progressDone();
  emitMessage("Done.");
  emit resultCommitted();
}

int ZFlyEmBodySplitProject::selectAllSeed()
{
  int nSelected = 0;
  if (getDocument() != NULL) {
    QList<ZDocPlayer*> playerList =
        getDocument()->getPlayerList(ZStackObjectRole::ROLE_SEED);
//    getDocument()->deselectAllObject();
    foreach (const ZDocPlayer *player, playerList) {
      getDocument()->setSelected(player->getData(), true);
      ++nSelected;
    }
    if (m_dataFrame != NULL) {
      m_dataFrame->view()->paintObject();
    }
  }

  return nSelected;
}

int ZFlyEmBodySplitProject::selectSeed(int label)
{
  int nSelected = 0;
  if (getDocument() != NULL) {
    QList<ZDocPlayer*> playerList =
        getDocument()->getPlayerList(ZStackObjectRole::ROLE_SEED);
    getDocument()->deselectAllObject();
    foreach (const ZDocPlayer *player, playerList) {
      if (player->getLabel() == label) {
       getDocument()->setSelected(player->getData(), true);
       ++nSelected;
      }
    }
    if (m_dataFrame != NULL) {
      m_dataFrame->view()->paintObject();
    }
  }

  return nSelected;
}

ZJsonArray ZFlyEmBodySplitProject::getRoiJson() const
{
  ZJsonArray roiJson;

  ZFlyEmProofDoc *proofDoc = getDocument<ZFlyEmProofDoc>();
  if (proofDoc != NULL) {
    ZIntCuboidObj *roi = proofDoc->getSplitRoi();
    if (roi != NULL) {
      roiJson = roi->getCuboid().toJsonArray();
    }
  }

  return roiJson;
}

void ZFlyEmBodySplitProject::backupSeed()
{
  if (getBodyId() == 0) {
    return;
  }

  ZOUT(LINFO(), 3) << "Backup seeds";

  ZDvidReader &reader = getMainReader();
  if (reader.good()) {
    QList<ZDocPlayer*> playerList;
    if (getDocument() != NULL) {
      playerList = getDocument()->getPlayerList(ZStackObjectRole::ROLE_SEED);
    }
    ZJsonArray jsonArray;
    foreach (const ZDocPlayer *player, playerList) {
      ZJsonObject jsonObj = player->toJsonObject();
      if (!jsonObj.isEmpty()) {
        jsonArray.append(jsonObj);
      }
    }

    ZDvidWriter &writer = getCommitWriter();
    if (writer.good()) {
      ZJsonObject rootObj;

      ZFlyEmProofDoc *proofDoc = getDocument<ZFlyEmProofDoc>();
      if (proofDoc != NULL) {
        ZIntCuboidObj *roi = proofDoc->getSplitRoi();
        if (roi != NULL) {
          ZJsonArray roiJson = roi->getCuboid().toJsonArray();
          rootObj.setEntry("roi", roiJson);
        }
      }

      if (!jsonArray.isEmpty()) {
        rootObj.setEntry("seeds", jsonArray);
      }

      if (!rootObj.isEmpty()) {
        writer.writeJson(getSplitLabelName(), getBackupSeedKey(getBodyId()),
                         rootObj);
      }
    }
  }
}

void ZFlyEmBodySplitProject::deleteSavedSeed()
{
  ZDvidReader &reader = getMainReader();
  if (reader.good()) {
    if (!reader.hasData(getSplitLabelName())) {
      emitError(
            ("Failed to delete seed: " + getSplitLabelName() +
            " has not been created on the server.").c_str());

      return;
    }
  }

  ZDvidWriter &writer = getMainWriter();
  if (writer.good()) {
    writer.deleteKey(getSplitLabelName(), getSeedKey(getBodyId()));
    emit messageGenerated(QString("All seeds of %1 have been deleted").
                          arg(getBodyId()));
  }
}

void ZFlyEmBodySplitProject::swapMainSeedLabel(int label)
{
  if (getDocument() != NULL && label != 1) {
    QList<ZDocPlayer*> playerList =
        getDocument()->getPlayerList(ZStackObjectRole::ROLE_SEED);

    //Check if the label exists
    QSet<ZDocPlayer*> newSeedSet;
    QSet<ZDocPlayer*> oldSeedSet;
    foreach (ZDocPlayer *player, playerList) {
      if (player->getLabel() == label) {
        newSeedSet.insert(player);
      } else if (player->getLabel() == 1) {
        oldSeedSet.insert(player);
      }
    }

    for (QSet<ZDocPlayer*>::iterator iter = newSeedSet.begin();
         iter != newSeedSet.end(); ++iter) {
      ZDocPlayer *seed = *iter;
      seed->setLabel(1);
    }

    for (QSet<ZDocPlayer*>::iterator iter = oldSeedSet.begin();
         iter != oldSeedSet.end(); ++iter) {
      ZDocPlayer *seed = *iter;
      seed->setLabel(label);
    }

    ZOUT(LTRACE(), 5) << "Swap seed label";
    TStackObjectList objList =
        getDocument()->getObjectList(ZStackObject::EType::OBJECT3D_SCAN);

    for (TStackObjectList::const_iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      ZObject3dScan *splitObj = dynamic_cast<ZObject3dScan*>(*iter);
      if (splitObj != NULL) {
        if (splitObj->hasRole(ZStackObjectRole::ROLE_TMP_RESULT)) {
          if ((int) splitObj->getLabel() == label) {
            splitObj->setLabel(1);
            splitObj->setColor(ZStroke2d::GetLabelColor(1));
          } else if (splitObj->getLabel() == 1) {
            splitObj->setLabel(label);
            splitObj->setColor(ZStroke2d::GetLabelColor(label));
          }
        }
      }
    }

    ZStack *labelField = getDocument()->getLabelField();
    if (labelField != NULL) {
      labelField->swapValue(1, label);
    }

    if (m_dataFrame != NULL) {
      m_dataFrame->view()->paintObject();
    }
  }
}

ZIntCuboid ZFlyEmBodySplitProject::getSeedBoundBox() const
{
  ZIntCuboid box;

  QList<ZDocPlayer*> playerList =
      getDocument()->getPlayerList(ZStackObjectRole::ROLE_SEED);
  foreach (const ZDocPlayer *player, playerList) {
    ZIntCuboid seedBox = player->getBoundBox();
    if (!seedBox.isEmpty()) {
      box.join(seedBox);
    }
  }

  return box;
}

ZJsonArray ZFlyEmBodySplitProject::getSeedJson() const
{
  QList<ZDocPlayer*> playerList =
      getDocument()->getPlayerList(ZStackObjectRole::ROLE_SEED);
  ZJsonArray jsonArray;
  foreach (const ZDocPlayer *player, playerList) {
    ZJsonObject jsonObj = player->toSeedJson();
    if (!jsonObj.isEmpty()) {
      jsonArray.append(jsonObj);
    }
  }

  return jsonArray;
}

void ZFlyEmBodySplitProject::saveSeed(bool emphasizingMessage)
{
  ZDvidReader &reader = getMainReader();
  if (reader.good()) {
    if (!reader.hasData(getSplitLabelName())) {
      emitError(
            ("Failed to save seed: " + getSplitLabelName() +
            " has not been created on the server.").c_str());

      return;
    }
  }

  QList<ZDocPlayer*> playerList =
      getDocument()->getPlayerList(ZStackObjectRole::ROLE_SEED);
  ZJsonArray jsonArray;
  foreach (const ZDocPlayer *player, playerList) {
    ZJsonObject jsonObj = player->toJsonObject();
    if (!jsonObj.isEmpty()) {
      jsonArray.append(jsonObj);
    }
  }


  ZDvidWriter &writer = getMainWriter();
  if (writer.good()) {
    if (jsonArray.isEmpty()) {
      writer.deleteKey(getSplitLabelName(), getSeedKey(getBodyId()));
      if (emphasizingMessage) {
        emitPopoupMessage("All seeds deleted");
      } else {
        emitMessage("All seeds deleted");
      }
    } else {
      ZJsonObject rootObj;
      rootObj.setEntry("seeds", jsonArray);
      writer.writeJson(getSplitLabelName(), getSeedKey(getBodyId()), rootObj);
      if (emphasizingMessage) {
        emitPopoupMessage("All seeds have been saved");
      }
      emitMessage(ZWidgetMessage::appendTime("All seeds saved"));
    }
  }
}

std::string ZFlyEmBodySplitProject::saveTask(uint64_t bodyId) const
{
  std::string location;

  if (bodyId > 0) {
    ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriterFromUrl(
          GET_FLYEM_CONFIG.getTaskServer());
    if (writer != NULL) {
      ZJsonArray seedJson = getSeedJson();
      if (!seedJson.isEmpty()) {
        ZJsonArray roiJson = getRoiJson();
        if (roiJson.isEmpty()) {
          ZIntCuboid range = misc::EstimateSplitRoi(getSeedBoundBox());
          if (!range.isEmpty()) {
            roiJson = range.toJsonArray();
          }
        }

        ZJsonObject task = flyem::MakeSplitTask(
              getDvidTarget(), bodyId, seedJson, roiJson);

        location = writer->writeServiceTask("split", task);
        ZJsonObject taskJson;
        taskJson.setEntry(neutu::json::REF_KEY, location);
  //      QUrl url(bodyUrl.c_str());
        ZDvidUrl dvidUrl(getDvidTarget());
        QString taskKey = dvidUrl.getSplitTaskKey(bodyId).c_str();
  //      QString("task__") + QUrl::toPercentEncoding(bodyUrl.c_str());
        writer->writeSplitTask(taskKey, taskJson);

        location = taskKey.toStdString();
      }
#if 0
      ZJsonObject task;
      ZDvidUrl dvidUrl(getDvidTarget());
      std::string bodyUrl = dvidUrl.getSparsevolUrl(bodyId);
      task.setEntry("signal", bodyUrl);
      ZJsonArray seedJson = getSeedJson();
      task.setEntry("seeds", seedJson);
      ZJsonArray roiJson = getRoiJson();
#if 0
      if (roiJson.isEmpty()) {
        ZIntCuboid range = ZFlyEmMisc::EstimateSplitRoi(getSeedBoundBox());
        if (!range.isEmpty()) {
          roiJson = range.toJsonArray();
        }
      }
#endif
      if (!roiJson.isEmpty()) {
        task.setEntry("range", roiJson);
      }

      ZJsonObject signalInfo;
      signalInfo.setEntry(
            ZDvidTarget::m_grayScaleNameKey, getDvidTarget().getGrayScaleName());
      ZJsonObject sourceConfig = getDvidTarget().getSourceConfig();
      if (!sourceConfig.isEmpty()) {
        signalInfo.setEntry(ZDvidTarget::m_sourceConfigKey, sourceConfig);
      }
      task.setEntry("signal info", signalInfo);
#endif

    }
  }

  return location;
}

void ZFlyEmBodySplitProject::updateBodyId()
{
  m_bodyId = 0;
  ZFlyEmProofDoc *doc = getDocument<ZFlyEmProofDoc>();
  if (doc != NULL) {
    std::set<uint64_t> bodySet =
        doc->getSelectedBodySet(neutu::ELabelSource::ORIGINAL);
    if (bodySet.size() == 1) {
      m_bodyId = *(bodySet.begin());
    }
  }
}

uint64_t ZFlyEmBodySplitProject::getBodyId() const
{
#ifdef _NEU3_
  const_cast<ZFlyEmBodySplitProject&>(*this).updateBodyId();
#endif

  return m_bodyId;
}

std::string ZFlyEmBodySplitProject::saveTask() const
{
  return saveTask(getBodyId());
}

void ZFlyEmBodySplitProject::recoverSeed()
{
  downloadSeed(getBackupSeedKey(getBodyId()));
}

void ZFlyEmBodySplitProject::loadSeed(const ZJsonObject &obj)
{
  if (obj.hasKey("seeds")) {
    ZLabelColorTable colorTable;

    ZJsonArray jsonArray(obj["seeds"], ZJsonValue::SET_INCREASE_REF_COUNT);
    for (size_t i = 0; i < jsonArray.size(); ++i) {
      ZJsonObject seedJson(
            jsonArray.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
      if (seedJson.hasKey("stroke")) {
        ZStroke2d *stroke = new ZStroke2d;
        stroke->loadJsonObject(seedJson);
        if (!stroke->isEmpty()) {
          stroke->setRole(ZStackObjectRole::ROLE_SEED |
                          ZStackObjectRole::ROLE_3DGRAPH_DECORATOR);
          stroke->setPenetrating(false);
          getDocument()->addObject(stroke);
        } else {
          delete stroke;
        }
      } else if (seedJson.hasKey("obj3d")) {
        ZObject3d *obj3d = new ZObject3d;
        obj3d->loadJsonObject(seedJson);
        obj3d->setColor(colorTable.getColor(obj3d->getLabel()));

        if (!obj3d->isEmpty()) {
          obj3d->setRole(ZStackObjectRole::ROLE_SEED |
                         ZStackObjectRole::ROLE_3DGRAPH_DECORATOR);
          getDocument()->addObject(obj3d);
        } else {
          delete obj3d;
        }
      }
    }

    if (obj.hasKey("roi")) {
      ZIntCuboid box;
      box.loadJson(ZJsonArray(obj.value("roi")));

      ZIntCuboidObj *roiObj = new ZIntCuboidObj;
      roiObj->setCuboid(box);
      roiObj->setColor(QColor(255, 255, 255));
      roiObj->setSource(ZStackObjectSourceFactory::MakeFlyEmSplitRoiSource());
      getDocument()->addObject(roiObj);
    }
  }
}

void ZFlyEmBodySplitProject::importSeed(const QString &fileName)
{
  if (!fileName.isEmpty()) {
    ZJsonObject obj;
    obj.load(fileName.toStdString());
    if (obj.hasKey("seeds")) {
#ifdef _DEBUG_
      std::cout << getDocument()->getPlayerList(
                     ZStackObjectRole::ROLE_SEED).size() << " seeds" <<  std::endl;
#endif

      getDocument()->undoStack()->clear();
      removeAllSeed();
      loadSeed(obj);
    } else {
      emit messageGenerated(
            ZWidgetMessage(
              QString("Invalid seed file: %1. Seeds remain unchanged").
              arg(fileName),
              neutu::EMessageType::ERROR));
    }
  }
}

void ZFlyEmBodySplitProject::exportSeed(const QString &fileName)
{
  if (!fileName.isEmpty()) {
    QList<ZDocPlayer*> playerList =
        getDocument()->getPlayerList(ZStackObjectRole::ROLE_SEED);
    ZJsonArray jsonArray;
    foreach (const ZDocPlayer *player, playerList) {
      ZJsonObject jsonObj = player->toJsonObject();
      if (!jsonObj.isEmpty()) {
        jsonArray.append(jsonObj);
      }
    }

    ZDvidWriter writer;
    if (writer.open(getDvidTarget())) {
      if (!jsonArray.isEmpty()) {
        ZJsonObject rootObj;
        rootObj.setEntry("seeds", jsonArray);
        rootObj.dump(fileName.toStdString());
      }
    }
  }
}

void ZFlyEmBodySplitProject::downloadSeed()
{
  downloadSeed(getSeedKey(getBodyId()));
}

void ZFlyEmBodySplitProject::removeAllSeed()
{
  getDocument()->removeObject(ZStackObjectRole::ROLE_SEED, true);
}

void ZFlyEmBodySplitProject::removeAllSideSeed()
{
  std::set<ZStackObject*> removeSet;
  QList<ZDocPlayer*> &playerList =
      getDocument()->getPlayerList().getPlayerList();
  {
    QMutexLocker locker(getDocument()->getPlayerList().getMutex());
    for (QList<ZDocPlayer*>::iterator iter = playerList.begin();
         iter != playerList.end(); /*++iter*/) {
      ZDocPlayer *player = *iter;
      if (player->hasRole(ZStackObjectRole::ROLE_SEED) && player->getLabel() > 1) {
        removeSet.insert(player->getData());
        iter = playerList.erase(iter);
        delete player;
      } else {
        ++iter;
      }
    }
  }

  getDocument()->beginObjectModifiedMode(ZStackDoc::EObjectModifiedMode::CACHE);
  for (std::set<ZStackObject*>::iterator iter = removeSet.begin();
       iter != removeSet.end(); ++iter) {
    getDocument()->removeObject(*iter);
  }
  getDocument()->endObjectModifiedMode();
  getDocument()->processObjectModified();

  /*
  getDocument()->getObjectGroup().removeObject(
        removeSet.begin(), removeSet.end(), true);

  if (!removeSet.empty()) {
    getDocument()->notifyObjectModified();
    getDocument()->notifyPlayerChanged(ZStackObjectRole::ROLE_SEED);
  }
  */
}

void ZFlyEmBodySplitProject::downloadSeed(const std::string &seedKey)
{
  ZDvidReader &reader = getMainReader();
  if (reader.good()) {
    getDocument()->undoStack()->clear();
    removeAllSeed();
    QByteArray seedData = reader.readKeyValue(
          getSplitLabelName().c_str(), seedKey.c_str());
    if (!seedData.isEmpty()) {
      ZJsonObject obj;
      obj.decode(seedData.constData());

      loadSeed(obj);
    }
  }
}

ZFlyEmNeuron ZFlyEmBodySplitProject::getFlyEmNeuron() const
{
  ZFlyEmNeuron neuron;
  neuron.setId(getBodyId());
  neuron.setPath(getDvidTarget());

  return neuron;
}

void ZFlyEmBodySplitProject::viewPreviousSlice()
{
  if (getDataFrame() != NULL) {
    getDataFrame()->view()->blockRedraw(true);
    getDataFrame()->view()->stepSlice(-1);
    getDataFrame()->view()->blockRedraw(false);
    viewFullGrayscale();
    updateBodyMask();
  }
}

void ZFlyEmBodySplitProject::viewNextSlice()
{
  if (getDataFrame() != NULL) {
    getDataFrame()->view()->blockRedraw(true);
    getDataFrame()->view()->stepSlice(1);
    getDataFrame()->view()->blockRedraw(false);
    viewFullGrayscale();
    updateBodyMask();
  }
}

void ZFlyEmBodySplitProject::viewFullGrayscale(bool viewing)
{
  ZStackFrame *frame = getDataFrame();
  if (frame != NULL) {
    if (viewing) {
      viewFullGrayscale();
    } else {
      ZStackObject *obj =
          frame->document()->getObjectGroup().findFirstSameSource(
            ZStackObject::EType::DVID_GRAY_SLICE,
            ZStackObjectSourceFactory::MakeDvidGraySliceSource(neutu::EAxis::Z));
      if (obj != NULL) {
        obj->setVisible(false);
        frame->updateView();
      }
    }
  }
}

void ZFlyEmBodySplitProject::viewFullGrayscale()
{
  ZDvidReader &reader = getMainReader();
  if (reader.good()) {
    ZStackFrame *frame = getDataFrame();
    if (frame != NULL) {
      int currentSlice = frame->view()->sliceIndex();

      QRect rect = frame->view()->getViewPort(neutu::ECoordinateSystem::STACK);
      ZRect2d rectRoi;
      rectRoi.set(rect.x(), rect.y(), rect.width(), rect.height());
//      ZRect2d rectRoi = frame->document()->getRect2dRoi();
      ZIntPoint offset = frame->document()->getStackOffset();
//      if (!rectRoi.isValid()) {
//        int width = frame->document()->getStackWidth();
//        int height = frame->document()->getStackHeight();
//        rectRoi.set(offset.getX(), offset.getY(), width, height);
//      }

      int z = currentSlice + offset.getZ();
      ZDvidGraySlice *graySlice = dynamic_cast<ZDvidGraySlice*>(
            frame->document()->getObjectGroup().findFirstSameSource(
              ZStackObject::EType::DVID_GRAY_SLICE,
              ZStackObjectSourceFactory::MakeDvidGraySliceSource(neutu::EAxis::Z)));

      if (graySlice == NULL) {
        graySlice = new ZDvidGraySlice();
        graySlice->setDvidTarget(getDvidTarget());
        graySlice->setSource(
              ZStackObjectSourceFactory::MakeDvidGraySliceSource(neutu::EAxis::Z));
        frame->document()->addObject(graySlice, false);
      }

      graySlice->setVisible(true);

      graySlice->setBoundBox(rectRoi);
      graySlice->update(z);

      frame->updateView();


#if 0
      ZStack *stack = reader.readGrayScale(
            rectRoi.getX0(), rectRoi.getY0(), z,
            rectRoi.getWidth(), rectRoi.getHeight(), 1);
      ZStackPatch *patch = new ZStackPatch(stack);
      patch->setZOrder(-1);
      patch->setSource(ZStackObjectSource::getSource(
                         ZStackObjectSource::ID_BODY_GRAYSCALE_PATCH));
      patch->setTarget(ZStackObject::STACK_CANVAS);
      frame->document()->addStackPatch(patch);
#endif
    }
  }
}

/*
void ZFlyEmBodySplitProject::downloadBodyMask()
{
  ZFlyEmProofDoc *doc = getDocument<ZFlyEmProofDoc>();
  if (doc != NULL) {
    getDocument<ZFlyEmProofDoc>()->downloadBodyMask();
  }
}
*/
void ZFlyEmBodySplitProject::updateBodyMask()
{
  ZStackFrame *frame = getDataFrame();
  if (frame != NULL) {
    frame->document()->removeObject(ZStackObjectRole::ROLE_MASK, true);
    if (showingBodyMask()) {
      ZDvidReader &reader = getMainReader();
      if (reader.good()) {
        int currentSlice = frame->view()->sliceIndex();

        ZRect2d rectRoi = frame->document()->getRect2dRoi();
        ZIntPoint offset = frame->document()->getStackOffset();
        if (!rectRoi.isValid()) {
          int width = frame->document()->getStackWidth();
          int height = frame->document()->getStackHeight();
          rectRoi.set(offset.getX(), offset.getY(), width, height);
        }

        int z = currentSlice + offset.getZ();
        ZArray *array = reader.readLabels64(
              rectRoi.getX0(), rectRoi.getY0(), z,
              rectRoi.getWidth(), rectRoi.getHeight(), 1);
        /*
        ZStack *stack = reader.readBodyLabel(
              rectRoi.getX0(), rectRoi.getY0(), z,
              rectRoi.getWidth(), rectRoi.getHeight(), 1);
              */
        if (array != NULL) {
          /*
          std::map<int, ZObject3dScan*> *bodySet =
              ZObject3dScan::extractAllObject(
                (uint64_t*) stack->array8(), stack->width(), stack->height(), 1,
                stack->getOffset().getZ(), 1, NULL);
                */

          std::map<uint64_t, ZObject3dScan*> *bodySet =
              ZObject3dScan::extractAllObject(
                array->getDataPointer<uint64_t>(), array->getDim(0),
                array->getDim(1), 1,
                array->getStartCoordinate(2), 1, NULL);

          frame->document()->blockSignals(true);
          for (std::map<uint64_t, ZObject3dScan*>::const_iterator iter = bodySet->begin();
               iter != bodySet->end(); ++iter) {
            uint64_t label = iter->first;
            ZObject3dScan *obj = iter->second;
            if (label > 0) {
              obj->translate(
                    array->getStartCoordinate(0),
                    array->getStartCoordinate(1), 0);
              obj->setRole(ZStackObjectRole::ROLE_MASK);
              frame->document()->addObject(obj, false);
            } else {
              delete obj;
            }
          }
          frame->document()->blockSignals(false);
          frame->document()->notifyObject3dScanModified();
          frame->document()->notifyPlayerChanged(ZStackObjectRole::ROLE_MASK);
        }

        delete array;
      }
    }
  }
}

std::string ZFlyEmBodySplitProject::getSplitStatusName() const
{
  return ZDvidData::GetName(
        ZDvidData::ERole::SPLIT_STATUS, ZDvidData::ERole::BODY_LABEL,
        getDvidTarget().getBodyLabelName());
}

std::string ZFlyEmBodySplitProject::getSplitLabelName() const
{
  return ZDvidData::GetName(ZDvidData::ERole::SPLIT_LABEL,
                            ZDvidData::ERole::BODY_LABEL,
                            getDvidTarget().getBodyLabelName());
}

std::string ZFlyEmBodySplitProject::getSeedKey(uint64_t bodyId) const
{
  return getDvidTarget().getBodyLabelName() + "_seed_" +
      ZString::num2str(bodyId);
}

std::string ZFlyEmBodySplitProject::getBackupSeedKey(uint64_t bodyId) const
{
  return getDvidTarget().getBodyLabelName() + "_backup_seed_" +
      ZString::num2str(bodyId);
}

void ZFlyEmBodySplitProject::runLocalSplit()
{
  quitResultUpdate();
  if (getDocument() != NULL) {
    backupSeed();
    ZFlyEmProofDoc *proofdoc = getDocument<ZFlyEmProofDoc>();
    if (proofdoc != NULL) {
      proofdoc->runLocalSplit(getSplitMode());
    } else {
      getDocument()->runLocalSeededWatershed();
    }
  }
  if (m_quickResultWindow != NULL) {
    m_timer->start();
  }
}

void ZFlyEmBodySplitProject::runSplit()
{
  quitResultUpdate();
  if (getDocument() != NULL) {
    backupSeed();
    ZFlyEmProofDoc *proofdoc = getDocument<ZFlyEmProofDoc>();
    if (proofdoc != NULL) {
      proofdoc->runSplit(getSplitMode());
    } else {
      getDocument()->runSeededWatershed();
    }
  }
  if (m_quickResultWindow != NULL) {
    m_timer->start();
  }
}

void ZFlyEmBodySplitProject::runFullSplit()
{
  quitResultUpdate();
  if (getDocument() != NULL) {
    backupSeed();
    ZFlyEmProofDoc *proofdoc = getDocument<ZFlyEmProofDoc>();
    if (proofdoc != NULL) {
      proofdoc->runFullSplit(getSplitMode());
    } else {
      getDocument()->runSeededWatershed();
    }
  }
  if (m_quickResultWindow != NULL) {
    m_timer->start();
  }
}

void ZFlyEmBodySplitProject::setSeedProcessed(uint64_t bodyId)
{
  ZDvidWriter writer;
  if (writer.open(getDvidTarget())) {
    ZJsonObject statusJson;
    statusJson.setEntry("processed", true);
    writer.writeJson(getSplitStatusName(),
                     getSeedKey(bodyId), statusJson);
  }
}

bool ZFlyEmBodySplitProject::isSeedProcessed(uint64_t bodyId) const
{
  bool isProcessed = false;

  const ZDvidReader &reader = getMainReader();
  if (reader.good()) {
    QByteArray value = reader.readKeyValue(
          getSplitStatusName().c_str(),
          getSeedKey(bodyId).c_str());

    if (!value.isEmpty()) {
      ZJsonObject statusJson;
      statusJson.decodeString(value.data());
      if (statusJson.hasKey("processed")) {
        isProcessed = ZJsonParser::booleanValue(statusJson["processed"]);
      }
    }
  }

  return isProcessed;
}

ZStackDoc* ZFlyEmBodySplitProject::getDocument() const
{
  if (getDataFrame() != NULL) {
    return getDataFrame()->document().get();
  }

  return m_doc.get();
}

ZSharedPointer<ZStackDoc> ZFlyEmBodySplitProject::getSharedDocument() const
{
  if (getDataFrame() != NULL) {
    return getDataFrame()->document();
  }

  return m_doc;
}

void ZFlyEmBodySplitProject::setDocument(ZSharedPointer<ZStackDoc> doc)
{
  m_doc = doc;
}

bool ZFlyEmBodySplitProject::isReadyForSplit(const ZDvidTarget &target)
{
  bool succ = true;

  ZWidgetMessage message;
//  QStringList infoList;

  ZDvidReader &reader = getMainReader();
  if (reader.good()) {
    if (!reader.hasSparseVolume()) {
      message.appendMessage(("Incomplete split database: data \"" +
                             target.getBodyLabelName() +
                             "\" missing").c_str());
//      infoList.append();
      succ = false;
    }

    std::string splitLabelName = target.getSplitLabelName();
    /*ZDvidData::GetName(
          ZDvidData::ROLE_SPLIT_LABEL, ZDvidData::ROLE_BODY_LABEL,
          target.getBodyLabelName());*/

    if (!reader.hasData(splitLabelName)) {
      message.appendMessage(("Incomplete split database: data \"" + splitLabelName +
                             "\" missing").c_str());
//      infoList.append();
      succ = false;
    }

    std::string splitStatusName =  ZDvidData::GetName(
          ZDvidData::ERole::SPLIT_STATUS, ZDvidData::ERole::BODY_LABEL,
          target.getBodyLabelName());
    if (!reader.hasData(splitStatusName)) {
      message.appendMessage(("Incomplete split database: data \"" + splitStatusName +
                             "\" missing").c_str());
//      infoList.append();
      succ = false;
    }
  } else {
    message.appendMessage("Cannot connect to database.");
//    infoList.append();
    succ = false;
  }

  message.setType(neutu::EMessageType::ERROR);

  emit messageGenerated(message.toHtmlString(), true);
//  emit messageGenerated(message);

  return succ;
}

void ZFlyEmBodySplitProject::emitMessage(const QString &msg, bool appending)
{
  ZOUT(LTRACE(), 5) << "Outputting message: " << msg;

  ZWidgetMessage::ETarget target = ZWidgetMessage::TARGET_TEXT;
  if (appending) {
    target = ZWidgetMessage::TARGET_TEXT_APPENDING;
  }

  emit messageGenerated(
        ZWidgetMessage(msg, neutu::EMessageType::INFORMATION,
                       target | ZWidgetMessage::TARGET_KAFKA));
}

void ZFlyEmBodySplitProject::emitPopoupMessage(const QString &msg)
{
  ZWidgetMessage message(msg, neutu::EMessageType::INFORMATION);
  message.setTarget(ZWidgetMessage::TARGET_DIALOG);
  emit messageGenerated(message);
}


void ZFlyEmBodySplitProject::emitError(const QString &msg, bool appending)
{
  ZWidgetMessage::ETarget target = ZWidgetMessage::TARGET_TEXT;
  if (appending) {
    target = ZWidgetMessage::TARGET_TEXT_APPENDING;
  }

  emit messageGenerated(
        ZWidgetMessage(msg, neutu::EMessageType::ERROR,
                       target | ZWidgetMessage::TARGET_KAFKA));
}

void ZFlyEmBodySplitProject::update3DViewPlane()
{
  if (m_quickResultWindow) {
    if (m_dvidInfo.isValid()) {
      Z3DGraph *graph = flyem::MakePlaneGraph(getDocument(), m_dvidInfo);
      m_quickResultWindow->getDocument()->addObject(graph, true);
    }
  }
}

/*
void ZFlyEmBodySplitProject::attachBookmarkArray(ZFlyEmBookmarkArray *bookmarkArray)
{
  m_bookmarkArray = bookmarkArray;
}
*/

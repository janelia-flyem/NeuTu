#include "zflyembodysplitproject.h"

#include <QProcess>
#include <QByteArray>
#include <QtConcurrentRun>
#include <QWidget>

#include "zstackframe.h"
#include "z3dwindow.h"
#include "zstackdoclabelstackfactory.h"
#include "zstackobject.h"
#include "zstackball.h"
#include "zsparsestack.h"
#include "z3dvolumesource.h"
#include "zswctree.h"
#include "zwindowfactory.h"
#include "dvid/zdvidreader.h"
#include "zwindowfactory.h"
#include "zstackskeletonizer.h"
#include "neutubeconfig.h"
#include "zswcgenerator.h"
#include "z3dswcfilter.h"
#include "zobject3dscan.h"
#include "zstroke2d.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdviddata.h"
#include "zstring.h"
#include "zflyemcoordinateconverter.h"
#include "flyem/zflyemneuron.h"
#include "zstackview.h"
#include "zstackpatch.h"
#include "zstackobjectsource.h"
#include "neutubeconfig.h"
#include "zarray.h"
#include "dvid/zdvidgrayslice.h"
#include "zstackobjectsourcefactory.h"
#include "zflyemproofdoc.h"
#include "dvid/zdvidsparsestack.h"
#include "zwidgetmessage.h"

ZFlyEmBodySplitProject::ZFlyEmBodySplitProject(QObject *parent) :
  QObject(parent), m_bodyId(0), m_dataFrame(NULL),
  m_resultWindow(NULL), m_quickResultWindow(NULL),
  m_quickViewWindow(NULL), m_isBookmarkVisible(true), m_showingBodyMask(false)
{
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

void ZFlyEmBodySplitProject::clear()
{
  if (m_resultWindow != NULL) {
    m_resultWindow->hide();
    delete m_resultWindow;
    m_resultWindow = NULL;
  }

  clear(m_quickResultWindow);

  if (m_dataFrame != NULL) {
//    m_dataFrame->close3DWindow();
    m_dataFrame->hide();
    delete m_dataFrame;
    m_dataFrame = NULL;
  }

  if (m_quickViewWindow != NULL) {
    m_quickViewWindow->hide();
    delete m_quickViewWindow;
    m_quickViewWindow = NULL;
  }

  shallowClear();
}

void ZFlyEmBodySplitProject::shallowClearDataFrame()
{
  if (m_resultWindow != NULL) {
    m_resultWindow->hide();
    delete m_resultWindow;
    m_resultWindow = NULL;
  }

  clear(m_quickResultWindow);

  if (m_quickViewWindow != NULL) {
    m_quickViewWindow->hide();
    delete m_quickViewWindow;
    m_quickViewWindow = NULL;
  }

  shallowClear();
}

void ZFlyEmBodySplitProject::shallowClear()
{
  m_resultWindow = NULL;
  m_quickResultWindow = NULL;
  m_quickViewWindow = NULL;
  m_dataFrame = NULL;

  m_bodyId = 0;

  m_bookmarkDecoration.clear();
}

void ZFlyEmBodySplitProject::shallowClearResultWindow()
{
  m_resultWindow = NULL;
}

void ZFlyEmBodySplitProject::shallowClearQuickResultWindow()
{
  m_quickResultWindow = NULL;
}

void ZFlyEmBodySplitProject::shallowClearQuickViewWindow()
{
  m_quickViewWindow = NULL;
}

/*
void ZFlyEmBodySplitProject::shallowClearBodyWindow()
{
  m_bodyWindow = NULL;
}
*/
void ZFlyEmBodySplitProject::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
}

void ZFlyEmBodySplitProject::showDataFrame() const
{
  if (m_dataFrame != NULL) {
    m_dataFrame->show();
    m_dataFrame->raise();
    m_dataFrame->activateWindow();
  }
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
      window = factory.make3DWindow(getSharedDocument(), Z3DWindow::NORMAL_INIT);
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
      emit messageGenerated("3D functions are disabled");
    }

  }

#if 0
  if (m_dataFrame != NULL) {
    if (m_bodyWindow == NULL) {
    //emit messageGenerated("Showing data in 3D ...");
      m_bodyWindow = m_dataFrame->open3DWindow();

    } else {
      m_bodyWindow->show();
      m_bodyWindow->raise();
    }
    //emit messageGenerated("Done.");
  }
#endif
}

ZObject3dScan* ZFlyEmBodySplitProject::readBody(ZObject3dScan *out) const
{
  if (out != NULL) {
    out->clear();
  }
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    if (out == NULL) {
      out = new ZObject3dScan;
    }

    reader.readBody(getBodyId(), out);
  }

  return out;
}

void ZFlyEmBodySplitProject::quickView()
{
  if (m_quickViewWindow == NULL) {
    emit messageGenerated("Generating quick view ...");

    ZObject3dScan obj;
    if (getDocument() != NULL) {
      const ZObject3dScan *objMask = NULL;
      const ZStackDoc *doc = getDocument();
      if (doc->hasSparseStack()) {
        objMask = doc->getConstSparseStack()->getObjectMask();
      } else {
        if (getDocument<ZFlyEmProofDoc>() != NULL) {
          objMask = getDocument<ZFlyEmProofDoc>()->getDvidSparseStack()->getObjectMask();
        }
      }
      if (objMask != NULL) {
        obj = *objMask;
      }
    }

    if (obj.isEmpty()) {
      const ZDvidTarget &target = getDvidTarget();

      ZDvidReader reader;
      if (reader.open(target)) {
        int bodyId = getBodyId();
        obj = reader.readBody(bodyId);
        if (!obj.isEmpty()) {
          obj.canonize();
        }
      }
    }

    ZSwcTree *tree = ZSwcGenerator::createSurfaceSwc(obj, 2);

    ZStackDoc *doc = new ZStackDoc(NULL, NULL);
    doc->setTag(NeuTube::Document::FLYEM_BODY_DISPLAY);
    doc->addSwcTree(tree);

    ZWindowFactory factory;
    factory.setWindowTitle("Quick View");

    m_quickViewWindow = factory.make3DWindow(doc);
    m_quickViewWindow->getSwcFilter()->setRenderingPrimitive("Sphere");
    connect(m_quickViewWindow, SIGNAL(destroyed()),
            this, SLOT(shallowClearQuickViewWindow()));
    if (m_dataFrame != NULL) {
      connect(m_quickViewWindow, SIGNAL(locating2DViewTriggered(ZStackViewParam)),
              m_dataFrame, SLOT(setView(ZStackViewParam)));
    }
    connect(m_quickViewWindow, SIGNAL(locating2DViewTriggered(ZStackViewParam)),
            this, SIGNAL(locating2DViewTriggered(ZStackViewParam)));

    ZIntCuboid box = obj.getBoundBox();
    m_quickViewWindow->notifyUser(
          QString("Size: %1; Bounding box: %2 x %3 x %4").
          arg(obj.getVoxelNumber()).arg(box.getWidth()).arg(box.getHeight()).
          arg(box.getDepth()));
  }

  if (m_quickViewWindow != NULL) {
    m_quickViewWindow->show();
    m_quickViewWindow->raise();
    emit messageGenerated("Done.");
  } else {
    emit messageGenerated("Failed to lauch quick view.");
  }
}

void ZFlyEmBodySplitProject::showSkeleton(ZSwcTree *tree)
{
  if (tree != NULL) {
    if (m_quickViewWindow == NULL) {
      ZWindowFactory factory;
      factory.setWindowTitle("Quick View");
      ZStackDoc *doc = new ZStackDoc(NULL, NULL);
      doc->addSwcTree(tree);
      m_quickViewWindow = factory.make3DWindow(doc);
      connect(m_quickViewWindow, SIGNAL(destroyed()),
              this, SLOT(shallowClearQuickViewWindow()));
    } else {
      m_quickViewWindow->getDocument()->removeAllSwcTree();
      m_quickViewWindow->getDocument()->addSwcTree(tree);
    }

    m_quickViewWindow->show();
    m_quickViewWindow->raise();
  }
}

void ZFlyEmBodySplitProject::loadResult3dQuick(ZStackDoc *doc)
{
  if (doc != NULL && getDocument() != NULL) {
    doc->removeAllSwcTree();
    TStackObjectList objList =
        getDocument()->getObjectList(ZStackObject::TYPE_OBJ3D);
    const int maxSwcNodeNumber = 100000;
    const int maxScale = 50;
    const int minScale = 1;
    for (TStackObjectList::const_iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      ZObject3d *obj = dynamic_cast<ZObject3d*>(*iter);
      if (obj != NULL) {
        if (!obj->getRole().hasRole(ZStackObjectRole::ROLE_SEED)) {
          int ds = obj->size() / maxSwcNodeNumber + 1;
          if (ds < minScale) {
            ds = minScale;
          }
          if (ds > maxScale) {
            ds = maxScale;
          }

          ZSwcTree *tree = ZSwcGenerator::createSwc(
                *obj, dmin2(5.0, ds / 2.0), ds);
          tree->setAlpha(255);
          if (tree != NULL) {
            doc->addSwcTree(tree);
          }
        }
      }
    }
  }
}

void ZFlyEmBodySplitProject::updateResult3dQuick()
{
  if (m_quickResultWindow != NULL) {
    ZStackDoc *doc = m_quickResultWindow->getDocument();
    bool resetCamera = true;
    if (doc->hasSwc()) {
      resetCamera = false;
    }

    loadResult3dQuick(doc);
    if (resetCamera) {
      m_quickResultWindow->resetCamera();
    }
  }
}

void ZFlyEmBodySplitProject::showResult3dQuick()
{
  ZStackDoc *mainDoc = getDocument();

  if (mainDoc != NULL) {
    if (m_quickResultWindow == NULL) {
      ZWindowFactory windowFactory;
      windowFactory.setWindowTitle("Splitting Result");
      ZStackDoc *doc = new ZStackDoc(NULL, NULL);
      doc->setTag(NeuTube::Document::FLYEM_BODY_DISPLAY);
      loadResult3dQuick(doc);
      m_quickResultWindow = windowFactory.make3DWindow(doc);
      m_quickResultWindow->getSwcFilter()->setColorMode("Intrinsic");
      m_quickResultWindow->getSwcFilter()->setRenderingPrimitive("Sphere");

      connect(m_quickResultWindow, SIGNAL(destroyed()),
              this, SLOT(shallowClearQuickResultWindow()));
      connect(mainDoc, SIGNAL(labelFieldModified()),
              this, SLOT(updateResult3dQuick()));
      if (m_dataFrame != NULL) {
        connect(m_quickResultWindow, SIGNAL(locating2DViewTriggered(ZStackViewParam)),
                m_dataFrame, SLOT(setView(ZStackViewParam)));
      }
      connect(m_quickResultWindow, SIGNAL(locating2DViewTriggered(ZStackViewParam)),
              this, SIGNAL(locating2DViewTriggered(ZStackViewParam)));
    }

    m_quickResultWindow->show();
    m_quickResultWindow->raise();
  }
}

void ZFlyEmBodySplitProject::showResult3d()
{
  if (getDocument() != NULL) {
    if (m_resultWindow == NULL) {
      //emit messageGenerated("Showing results in 3D ...");
      //ZStackDocReader docReader;
      ZStackDocLabelStackFactory *factory = new ZStackDocLabelStackFactory;
      factory->setDocument(getDocument());
      ZStack *labeled = factory->makeStack();
      if (labeled != NULL) {
        //docReader.setStack(labeled);
        ZStackDoc *doc = new ZStackDoc(labeled, NULL);
        doc->setTag(NeuTube::Document::FLYEM_SPLIT);
        doc->setStackFactory(factory);
        ZWindowFactory windowFactory;
        windowFactory.setWindowTitle("Splitting Result");
        m_resultWindow = windowFactory.make3DWindow(doc);

        //ZStackFrame *newFrame = new ZStackFrame;
        //newFrame->addDocData(docReader);
        //newFrame->document()->setTag(NeuTube::Document::FLYEM_SPLIT);
        //newFrame->document()->setStackFactory(factory);

        connect(getDocument(), SIGNAL(labelFieldModified()),
                doc, SLOT(reloadStack()));
        /*
        m_dataFrame->connect(
              m_dataFrame->document().get(), SIGNAL(labelFieldModified()),
              doc, SLOT(reloadStack()));
              */
        //m_resultWindow = newFrame->open3DWindow(NULL);
        if (getDocument()->hasVisibleSparseStack()) {
          ZIntPoint dsIntv =
              getDocument()->getConstSparseStack()->getDownsampleInterval();
          if (dsIntv.getX() != dsIntv.getZ()) {
            m_resultWindow->getVolumeSource()->setZScale(
                  ((float) (dsIntv.getZ() + 1)) / (dsIntv.getX() + 1));
            m_resultWindow->resetCamera();
          }
        }
        connect(m_resultWindow, SIGNAL(destroyed()),
                this, SLOT(shallowClearResultWindow()));
        //delete newFrame;
      }
    }
    m_resultWindow->show();
    m_resultWindow->raise();

    //emit messageGenerated("Done.");
  }
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

  updateBookDecoration();
}

void ZFlyEmBodySplitProject::removeAllBookmark()
{
  m_bookmarkArray.clear();
  clearBookmarkDecoration();
}

void ZFlyEmBodySplitProject::loadBookmark(const QString &filePath)
{
  ZDvidReader reader;
  ZFlyEmCoordinateConverter converter;
  if (reader.open(m_dvidTarget)) {
    ZDvidInfo info = reader.readGrayScaleInfo();
    converter.configure(info);
    m_bookmarkArray.importJsonFile(filePath.toStdString(), NULL/*&converter*/);
  }
}

void ZFlyEmBodySplitProject::locateBookmark(const ZFlyEmBookmark &bookmark)
{
  if (m_dataFrame != NULL) {
    m_dataFrame->viewRoi(bookmark.getLocation().getX(),
                         bookmark.getLocation().getY(),
                         bookmark.getLocation().getZ(), 5);
  }
}

void ZFlyEmBodySplitProject::clearBookmarkDecoration()
{
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
}

void ZFlyEmBodySplitProject::addBookmarkDecoration(
    const ZFlyEmBookmarkArray &bookmarkArray)
{
  if (getDocument() != NULL) {
    for (ZFlyEmBookmarkArray::const_iterator iter = bookmarkArray.begin();
         iter != bookmarkArray.end(); ++iter) {
      const ZFlyEmBookmark &bookmark = *iter;
      ZPunctum *circle = new ZPunctum;
      circle->set(bookmark.getLocation(), 5);

//      ZStackBall *circle = new ZStackBall;
//      circle->set(bookmark.getLocation(), 5);
      circle->setColor(255, 0, 0);
      circle->setVisible(m_isBookmarkVisible);
//      circle->setRole(ZStackObjectRole::ROLE_3DGRAPH_DECORATOR);
      getDocument()->addObject(circle);
      m_bookmarkDecoration.push_back(circle);
    }
  }
}

void ZFlyEmBodySplitProject::updateBookDecoration()
{
  clearBookmarkDecoration();

  if (getDocument() != NULL) {
    ZFlyEmBookmarkArray bookmarkArray;
    foreach (ZFlyEmBookmark bookmark, m_bookmarkArray) {
      if (bookmark.getBodyId() == getBodyId()) {
        bookmarkArray.append(bookmark);
      }
    }
    addBookmarkDecoration(bookmarkArray);
  }
}

void ZFlyEmBodySplitProject::showBookmark(bool visible)
{
  m_isBookmarkVisible = visible;
  for (std::vector<ZStackObject*>::iterator iter = m_bookmarkDecoration.begin();
       iter != m_bookmarkDecoration.end(); ++iter) {
    ZStackObject *obj = *iter;
    obj->setVisible(visible);
  }
  if (m_dataFrame != NULL && !m_bookmarkDecoration.empty()) {
    m_dataFrame->updateView();
  }
}

std::set<int> ZFlyEmBodySplitProject::getBookmarkBodySet() const
{
  std::set<int> bodySet;
  foreach (ZFlyEmBookmark bookmark, m_bookmarkArray) {
    bodySet.insert(bookmark.getBodyId());
  }

  return bodySet;
}

void ZFlyEmBodySplitProject::exportSplits()
{
  //ZObject3dScan body = *(getDataFrame()->document()->getSparseStack()->getObjectMask());

}

void ZFlyEmBodySplitProject::commitResult()
{
  ZFlyEmBodySplitProject::commitResultFunc(
        getDocument()->getConstSparseStack()->getObjectMask(),
        getDocument()->getLabelField(),
        getDocument()->getConstSparseStack()->getDownsampleInterval());

  deleteSavedSeed();
  getDocument()->undoStack()->clear();
//  getDocument()->removeObject(ZStackObject::TYPE_OBJ3D);
  removeAllSideSeed();
  downloadBodyMask();

  /*
  QtConcurrent::run(this, &ZFlyEmBodySplitProject::commitResultFunc,
                    getDataFrame()->document()->getSparseStack()->getObjectMask(),
                    getDataFrame()->document()->getLabelField(),
                    getDataFrame()->document()->getSparseStack()->getDownsampleInterval());
                    */
}

void ZFlyEmBodySplitProject::commitResultFunc(
    const ZObject3dScan *wholeBody, const ZStack *stack, const ZIntPoint &dsIntv)
{
  emit progressStarted("Uploading splitted bodies", 100);

  emit messageGenerated("Uploading results ...");

//  const ZObject3dScan *wholeBody =
//      getDataFrame()->document()->getSparseStack()->getObjectMask();

  ZObject3dScan body = *wholeBody;

  emit messageGenerated(QString("Backup ... %1").arg(getBodyId()));

  std::string backupDir = GET_TEST_DATA_DIR + "/backup";
  body.save(backupDir + "/" + getSeedKey(getBodyId()) + ".sobj");

//  const ZStack *stack = getDataFrame()->document()->getLabelField();
  QStringList filePathList;
  int maxNum = 1;

  if (stack != NULL) {
//    const ZIntPoint &dsIntv =
//        getDataFrame()->document()->getSparseStack()->getDownsampleInterval();
    std::vector<ZObject3dScan*> objArray =
        ZObject3dScan::extractAllObject(*stack);

    emit progressAdvanced(0.1);

    double dp = 0.3;

    if (!objArray.empty()) {
      dp = 0.3 / objArray.size();
    }

    for (std::vector<ZObject3dScan*>::iterator iter = objArray.begin();
         iter != objArray.end(); ++iter) {
      ZObject3dScan *obj = *iter;
      obj->upSample(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());

      ZObject3dScan currentBody = body.subtract(*obj);

      if (!currentBody.isEmpty() && obj->getLabel() > 1) {
        std::vector<ZObject3dScan> objArray =
            currentBody.getConnectedComponent();
        for (std::vector<ZObject3dScan>::iterator iter = objArray.begin();
             iter != objArray.end(); ++iter) {
          ZString output = QDir::tempPath() + "/body_";
          output.appendNumber(getBodyId());
          output += "_";
          output.appendNumber(maxNum++);
          iter->save(output + ".sobj");
          filePathList << (output + ".sobj").c_str();
        }
      }
      delete obj;

      emit progressAdvanced(dp);
    }
  }

  if (!body.isEmpty()) {
    std::vector<ZObject3dScan> objArray = body.getConnectedComponent();

#ifdef _DEBUG_2
    body.save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

    double dp = 0.3;

    if (!objArray.empty()) {
      dp = 0.3 / objArray.size();
    }

    const size_t sizeThreshold = 0;
    //if (objArray.size() > 1 || !filePathList.isEmpty()) {
    for (std::vector<ZObject3dScan>::const_iterator iter = objArray.begin();
         iter != objArray.end(); ++iter) {
      const ZObject3dScan &obj = *iter;
      if (obj.getVoxelNumber() > sizeThreshold) {
        ZString output = QDir::tempPath() + "/body_";
        output.appendNumber(getBodyId());
        output += "_";
        output.appendNumber(maxNum++);
        obj.save(output + ".sobj");
        filePathList << (output + ".sobj").c_str();
      }

      emit progressAdvanced(dp);
    }
    //}
  }

  ZDvidReader reader;
  reader.open(m_dvidTarget);
//  int bodyId = reader.readMaxBodyId();

  int bodyId = 1;

  double dp = 0.3;

  if (!filePathList.empty()) {
    dp = 0.3 / filePathList.size();
  }

  ZDvidWriter writer;
  writer.open(getDvidTarget());
  foreach (QString objFile, filePathList) {
    ZObject3dScan obj;
    obj.load(objFile.toStdString());
    writer.writeSplit(getDvidTarget().getBodyLabelName(), obj, getBodyId(),
                      ++bodyId);

#if 0
    QString command = buildemPath +
        QString("/bin/dvid_load_sparse http://emdata2:8000 %1 %2 %3 %4").
        arg(m_dvidTarget.getUuid().c_str()).
        arg(m_dvidTarget.getBodyLabelName().c_str()).
        arg(objFile).arg(++bodyId);

    qDebug() << command;

    QProcess::execute(command);
#endif

    QString msg = QString("%1 uploaded.").arg(bodyId);
    emit messageGenerated(msg);

    emit progressAdvanced(dp);
  }

  //writer.writeMaxBodyId(bodyId);

  emit progressDone();
  emit messageGenerated("Done.");
  emit resultCommitted();
}

void ZFlyEmBodySplitProject::selectSeed(int label)
{
  if (getDocument() != NULL) {
    QList<const ZDocPlayer*> playerList =
        getDocument()->getPlayerList(ZStackObjectRole::ROLE_SEED);
    getDocument()->deselectAllObject();
    foreach (const ZDocPlayer *player, playerList) {
      if (player->getLabel() == label) {
       getDocument()->setSelected(player->getData(), true);
      }
    }
    if (m_dataFrame != NULL) {
      m_dataFrame->view()->paintObject();
    }
  }
}

void ZFlyEmBodySplitProject::backupSeed()
{
  if (getBodyId() == 0) {
    return;
  }

  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    QList<const ZDocPlayer*> playerList;
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

    ZDvidWriter writer;
    if (writer.open(getDvidTarget())) {
      if (!jsonArray.isEmpty()) {
        ZJsonObject rootObj;
        rootObj.setEntry("seeds", jsonArray);
        writer.writeJson(getSplitLabelName(), getBackupSeedKey(getBodyId()),
                         rootObj);
      }
    }
  }
}

void ZFlyEmBodySplitProject::deleteSavedSeed()
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    if (!reader.hasData(getSplitLabelName())) {
      emit messageGenerated(
            ("Failed to delete seed: " + getSplitLabelName() +
            " has not been created on the server.").c_str());

      return;
    }
  }

  ZDvidWriter writer;
  if (writer.open(getDvidTarget())) {
    writer.deleteKey(getSplitLabelName(), getSeedKey(getBodyId()));
    emit messageGenerated(QString("All seeds of %1 have been deleted").
                          arg(getBodyId()));
  }
}

void ZFlyEmBodySplitProject::saveSeed()
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    if (!reader.hasData(getSplitLabelName())) {
      emit messageGenerated(
            ("Failed to save seed: " + getSplitLabelName() +
            " has not been created on the server.").c_str());

      return;
    }
  }

  QList<const ZDocPlayer*> playerList =
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
    if (jsonArray.isEmpty()) {
      writer.deleteKey(getSplitLabelName(), getSeedKey(getBodyId()));
      emit messageGenerated("All seeds deleted");
    } else {
      ZJsonObject rootObj;
      rootObj.setEntry("seeds", jsonArray);
      writer.writeJson(getSplitLabelName(), getSeedKey(getBodyId()), rootObj);
      emit messageGenerated("All seeds saved");
    }
  }
}

void ZFlyEmBodySplitProject::recoverSeed()
{
  downloadSeed(getBackupSeedKey(getBodyId()));
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
  ZDocPlayerList &playerList = getDocument()->getPlayerList();
  for (ZDocPlayerList::iterator iter = playerList.begin();
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

  getDocument()->getObjectGroup().removeObject(
        removeSet.begin(), removeSet.end(), true);

  if (!removeSet.empty()) {
    getDocument()->notifyObjectModified();
    getDocument()->notifyPlayerChanged(ZStackObjectRole::ROLE_SEED);
  }
}

void ZFlyEmBodySplitProject::downloadSeed(const std::string &seedKey)
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    removeAllSeed();
    QByteArray seedData = reader.readKeyValue(
          getSplitLabelName().c_str(), seedKey.c_str());
    if (!seedData.isEmpty()) {
      ZJsonObject obj;
      obj.decode(seedData.constData());

      if (obj.hasKey("seeds")) {
        ZLabelColorTable colorTable;
#ifdef _DEBUG_
        std::cout << getDocument()->getPlayerList(
                       ZStackObjectRole::ROLE_SEED).size() << " seeds" <<  std::endl;
#endif
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
#ifdef _DEBUG_
        std::cout << getDocument()->getPlayerList(
                       ZStackObjectRole::ROLE_SEED).size() << " seeds" <<  std::endl;
#endif
      }
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
            ZStackObject::TYPE_DVID_GRAY_SLICE,
            ZStackObjectSourceFactory::MakeDvidGraySliceSource());
      if (obj != NULL) {
        obj->setVisible(false);
        frame->updateView();
      }
    }
  }
}

void ZFlyEmBodySplitProject::viewFullGrayscale()
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    ZStackFrame *frame = getDataFrame();
    if (frame != NULL) {
      int currentSlice = frame->view()->sliceIndex();

      QRect rect = frame->view()->getViewPort(NeuTube::COORD_STACK);
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
              ZStackObject::TYPE_DVID_GRAY_SLICE,
              ZStackObjectSourceFactory::MakeDvidGraySliceSource()));

      if (graySlice == NULL) {
        graySlice = new ZDvidGraySlice();
        graySlice->setDvidTarget(getDvidTarget());
        graySlice->setSource(ZStackObjectSourceFactory::MakeDvidGraySliceSource());
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

void ZFlyEmBodySplitProject::downloadBodyMask()
{
  getDocument<ZFlyEmProofDoc>()->downloadBodyMask();
}

void ZFlyEmBodySplitProject::updateBodyMask()
{
  ZStackFrame *frame = getDataFrame();
  if (frame != NULL) {
    frame->document()->removeObject(ZStackObjectRole::ROLE_MASK, true);
    if (showingBodyMask()) {
      ZDvidReader reader;
      if (reader.open(getDvidTarget())) {
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

          std::map<int, ZObject3dScan*> *bodySet =
              ZObject3dScan::extractAllObject(
                array->getDataPointer<uint64_t>(), array->getDim(0),
                array->getDim(1), 1,
                array->getStartCoordinate(2), 1, NULL);

          frame->document()->blockSignals(true);
          for (std::map<int, ZObject3dScan*>::const_iterator iter = bodySet->begin();
               iter != bodySet->end(); ++iter) {
            int label = iter->first;
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
        ZDvidData::ROLE_SPLIT_STATUS, ZDvidData::ROLE_BODY_LABEL,
        getDvidTarget().getBodyLabelName());
}

std::string ZFlyEmBodySplitProject::getSplitLabelName() const
{
  return ZDvidData::GetName(ZDvidData::ROLE_SPLIT_LABEL,
                            ZDvidData::ROLE_BODY_LABEL,
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

void ZFlyEmBodySplitProject::runSplit()
{
//  backupSeed();

  ZStackFrame *frame = getDataFrame();
  if (frame != NULL) {
    frame->document()->runSeededWatershed();
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

  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
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

  QStringList infoList;

  ZDvidReader reader;
  if (reader.open(target)) {
    if (!reader.hasSparseVolume()) {
      infoList.append(("Incomplete split database: data \"" +
                       target.getBodyLabelName() +
                       "\" missing").c_str());
      succ = false;
    }

    std::string splitLabelName = ZDvidData::GetName(
          ZDvidData::ROLE_SPLIT_LABEL, ZDvidData::ROLE_BODY_LABEL,
          target.getBodyLabelName());

    if (!reader.hasData(splitLabelName)) {
      infoList.append(("Incomplete split database: data \"" + splitLabelName +
                       "\" missing").c_str());
      succ = false;
    }

    std::string splitStatusName =  ZDvidData::GetName(
          ZDvidData::ROLE_SPLIT_STATUS, ZDvidData::ROLE_BODY_LABEL,
          target.getBodyLabelName());
    if (!reader.hasData(splitStatusName)) {
      infoList.append(("Incomplete split database: data \"" + splitStatusName +
                       "\" missing").c_str());
      succ = false;
    }
  } else {
    infoList.append("Cannot connect to database.");
    succ = false;
  }

  emit messageGenerated(ZWidgetMessage::toHtmlString(
                          infoList, NeuTube::MSG_ERROR));

  return succ;
}

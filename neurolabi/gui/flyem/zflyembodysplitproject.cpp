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


ZFlyEmBodySplitProject::ZFlyEmBodySplitProject(QObject *parent) :
  QObject(parent), m_bodyId(-1), m_dataFrame(NULL), m_resultWindow(NULL),
  m_quickResultWindow(NULL),
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
    m_dataFrame->close3DWindow();
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

  m_bodyId = -1;

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
  if (m_dataFrame != NULL) {
    //emit messageGenerated("Showing data in 3D ...");
    m_dataFrame->open3DWindow();
    //emit messageGenerated("Done.");
  }
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

    const ZDvidTarget &target = getDvidTarget();

    ZDvidReader reader;
    if (reader.open(target)) {
      int bodyId = getBodyId();
      ZObject3dScan obj = reader.readBody(bodyId);
      if (!obj.isEmpty()) {
        obj.canonize();

#if 0
        size_t voxelNumber =obj.getVoxelNumber();
        int intv = iround(Cube_Root((double) voxelNumber / 1000000));
        obj.downsampleMax(intv, intv, intv);

        ZSwcTree *tree = ZSwcGenerator::createSwc(obj);
#endif
        ZSwcTree *tree = ZSwcGenerator::createSurfaceSwc(obj);
        /*
        if (tree == NULL) {
          ZStackSkeletonizer skeletonizer;
          ZJsonObject config;
          config.load(NeutubeConfig::getInstance().getApplicatinDir() +
                      "/json/skeletonize.json");
          skeletonizer.configure(config);
          tree = skeletonizer.makeSkeleton(obj);
        }
        */

        ZStackDoc *doc = new ZStackDoc(NULL, NULL);
        doc->addSwcTree(tree);

        ZWindowFactory factory;
        factory.setWindowTitle("Quick View");

        m_quickViewWindow = factory.make3DWindow(doc);
        m_quickViewWindow->getSwcFilter()->setRenderingPrimitive("Sphere");
        connect(m_quickViewWindow, SIGNAL(destroyed()),
                this, SLOT(shallowClearQuickViewWindow()));
        connect(m_quickViewWindow, SIGNAL(locating2DViewTriggered(ZStackViewParam)),
                m_dataFrame, SLOT(setView(ZStackViewParam)));

        ZIntCuboid box = obj.getBoundBox();
        m_quickViewWindow->notifyUser(
              QString("Size: %1; Bounding box: %2 x %3 x %4").
              arg(obj.getVoxelNumber()).arg(box.getWidth()).arg(box.getHeight()).
              arg(box.getDepth()));
      }
    }
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
  if (doc != NULL) {
    doc->removeAllSwcTree();
    TStackObjectList objList =
        m_dataFrame->document()->getObjectList(ZStackObject::TYPE_OBJ3D);
    const int maxSwcNodeNumber = 200000;
    const int maxScale = 30;
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
  if (m_dataFrame != NULL) {
    if (m_quickResultWindow == NULL) {
      ZWindowFactory windowFactory;
      windowFactory.setWindowTitle("Splitting Result");
      ZStackDoc *doc = new ZStackDoc(NULL, NULL);
      loadResult3dQuick(doc);
      m_quickResultWindow = windowFactory.make3DWindow(doc);
      m_quickResultWindow->getSwcFilter()->setColorMode("Intrinsic");
      m_quickResultWindow->getSwcFilter()->setRenderingPrimitive("Sphere");

      connect(m_quickResultWindow, SIGNAL(destroyed()),
              this, SLOT(shallowClearQuickResultWindow()));
      connect(m_dataFrame->document().get(), SIGNAL(labelFieldModified()),
              this, SLOT(updateResult3dQuick()));
      connect(m_quickResultWindow, SIGNAL(locating2DViewTriggered(ZStackViewParam)),
              m_dataFrame, SLOT(setView(ZStackViewParam)));
    }

    m_quickResultWindow->show();
    m_quickResultWindow->raise();
  }
}

void ZFlyEmBodySplitProject::showResult3d()
{
  if (m_dataFrame != NULL) {
    if (m_resultWindow == NULL) {
      //emit messageGenerated("Showing results in 3D ...");
      //ZStackDocReader docReader;
      ZStackDocLabelStackFactory *factory = new ZStackDocLabelStackFactory;
      factory->setDocument(m_dataFrame->document().get());
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
        m_dataFrame->connect(
              m_dataFrame->document().get(), SIGNAL(labelFieldModified()),
              doc, SLOT(reloadStack()));
        //m_resultWindow = newFrame->open3DWindow(NULL);
        if (m_dataFrame->document()->hasSparseStack()) {
          ZIntPoint dsIntv =
              m_dataFrame->document()->getSparseStack()->getDownsampleInterval();
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
  if (m_dataFrame != NULL) {
    for (std::vector<ZStackObject*>::iterator iter = m_bookmarkDecoration.begin();
         iter != m_bookmarkDecoration.end(); ++iter) {
      ZStackObject *obj = *iter;
      m_dataFrame->document()->removeObject(obj, false);
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
  if (m_dataFrame != NULL) {
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
      m_dataFrame->document()->addObject(circle);
      m_bookmarkDecoration.push_back(circle);
    }
  }
}

void ZFlyEmBodySplitProject::updateBookDecoration()
{
  clearBookmarkDecoration();

  if (m_dataFrame != NULL) {
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
        getDataFrame()->document()->getSparseStack()->getObjectMask(),
        getDataFrame()->document()->getLabelField(),
        getDataFrame()->document()->getSparseStack()->getDownsampleInterval());
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

      if (!currentBody.isEmpty()) {
        ZString output = QDir::tempPath() + "/body_";
        output.appendNumber(getBodyId());
        output += "_";
        output.appendNumber(maxNum++);
        currentBody.save(output + ".sobj");
        filePathList << (output + ".sobj").c_str();
      }
      delete obj;

      emit progressAdvanced(dp);
    }
  }

  if (!body.isEmpty()) {
    std::vector<ZObject3dScan> objArray = body.getConnectedComponent();

#ifdef _DEBUG_
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

#ifdef _DEBUG_
  QString buildemPath = "/groups/flyem/home/zhaot/Downloads/buildem";
#else
  QString buildemPath = "/opt/Download/buildem";
#endif


  ZDvidReader reader;
  reader.open(m_dvidTarget);
  int bodyId = reader.readMaxBodyId();

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

  writer.writeMaxBodyId(bodyId);

  emit progressDone();
  emit messageGenerated("Done.");
  emit resultCommitted();
}

void ZFlyEmBodySplitProject::selectSeed(int label)
{
  QList<const ZDocPlayer*> playerList =
      m_dataFrame->document()->getPlayerList(ZStackObjectRole::ROLE_SEED);
  m_dataFrame->document()->deselectAllObject();
  foreach (const ZDocPlayer *player, playerList) {
    if (player->getLabel() == label) {
      m_dataFrame->document()->setSelected(player->getData(), true);
    }
  }
  m_dataFrame->view()->paintObject();
}

void ZFlyEmBodySplitProject::backupSeed()
{
  if (getBodyId() < 0) {
    return;
  }

  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    QList<const ZDocPlayer*> playerList =
        m_dataFrame->document()->getPlayerList(ZStackObjectRole::ROLE_SEED);
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
      m_dataFrame->document()->getPlayerList(ZStackObjectRole::ROLE_SEED);
  ZJsonArray jsonArray;
  foreach (const ZDocPlayer *player, playerList) {
    ZJsonObject jsonObj = player->toJsonObject();
    if (!jsonObj.isEmpty()) {
      jsonArray.append(jsonObj);
    }
    /*
    ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(player->getData());
    if (stroke != NULL) {
      ZJsonObject jsonObj = stroke->toJsonObject();
      jsonArray.append(jsonObj);
    }
    */
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
  m_dataFrame->document()->removeObject(ZStackObjectRole::ROLE_SEED, true);
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
        std::cout << m_dataFrame->document()->getPlayerList(
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
              m_dataFrame->document()->addObject(stroke);
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
              m_dataFrame->document()->addObject(obj3d);
            } else {
              delete obj3d;
            }
          }
        }
#ifdef _DEBUG_
        std::cout << m_dataFrame->document()->getPlayerList(
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
  return ZDvidData::getName(
        ZDvidData::ROLE_SPLIT_STATUS, ZDvidData::ROLE_BODY_LABEL,
        getDvidTarget().getBodyLabelName());
}

std::string ZFlyEmBodySplitProject::getSplitLabelName() const
{
  return ZDvidData::getName(ZDvidData::ROLE_SPLIT_LABEL,
                            ZDvidData::ROLE_BODY_LABEL,
                            getDvidTarget().getBodyLabelName());
}

std::string ZFlyEmBodySplitProject::getSeedKey(int bodyId) const
{
  return getDvidTarget().getBodyLabelName() + "_seed_" +
      ZString::num2str(bodyId);
}

std::string ZFlyEmBodySplitProject::getBackupSeedKey(int bodyId) const
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

void ZFlyEmBodySplitProject::setSeedProcessed(int bodyId)
{
  ZDvidWriter writer;
  if (writer.open(getDvidTarget())) {
    ZJsonObject statusJson;
    statusJson.setEntry("processed", true);
    writer.writeJson(getSplitStatusName(),
                     getSeedKey(bodyId), statusJson);
  }
}

bool ZFlyEmBodySplitProject::isSeedProcessed(int bodyId) const
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

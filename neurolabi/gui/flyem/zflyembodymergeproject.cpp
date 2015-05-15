#include "zflyembodymergeproject.h"

#include <QtConcurrentRun>
#include <QItemSelectionModel>

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

ZFlyEmBodyMergeProject::ZFlyEmBodyMergeProject(QObject *parent) :
  QObject(parent), m_dataFrame(NULL), m_bodyWindow(NULL),
  m_showingBodyMask(true)
{
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

  if (m_bodyWindow != NULL) {
    m_bodyWindow->hide();
    delete m_bodyWindow;
    m_bodyWindow = NULL;
  }

  m_currentSelected.clear();
}

int ZFlyEmBodyMergeProject::getCurrentZ() const
{
  int z = 0;
  if (m_dataFrame != NULL) {
    z = m_dataFrame->document()->getStackOffset().getZ();
  }

  return z;
}

void ZFlyEmBodyMergeProject::test()
{
  ZStackDocReader *reader = new ZStackDocReader();
  reader->loadStack(
        (GET_TEST_DATA_DIR + "/benchmark/em_stack_slice.tif").c_str());

  if (m_showingBodyMask) {
    emit newDocReady(reader, false);
    ZStack stack;
    stack.load(GET_TEST_DATA_DIR + "/benchmark/em_stack_slice_seg.tif");
    ZArray *array = ZArrayFactory::MakeArray(&stack);
    emit originalLabelUpdated(array, &m_currentSelected);
  } else {
    emit newDocReady(reader, true);
  }
}

void ZFlyEmBodyMergeProject::changeDvidNode(const std::string &newUuid)
{
  m_dvidTarget.setUuid(newUuid);
  m_currentSelected.clear();
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
      ZArray *array = reader.readLabels64(
            m_dvidTarget.getLabelBlockName(),
            //ZDvidData::getName(ZDvidData::ROLE_BODY_LABEL),
            x0, y0, z, width, height, 1);
#endif
      emit originalLabelUpdated(array, &m_currentSelected);
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

  m_dataFrame = dynamic_cast<ZFlyEmBodyMergeFrame*>(frame);

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

void ZFlyEmBodyMergeProject::uploadResult()
{
  ZDvidWriter dvidWriter;
  if (dvidWriter.open(m_dvidTarget)) {
    ZFlyEmBodyMerger *bodyMerger =
        m_dataFrame->getCompleteDocument()->getBodyMerger();
    ZFlyEmBodyMerger::TLabelMap labelMap = bodyMerger->getFinalMap();

    //reorganize the map
    QMap<int, std::vector<int> > mergeMap;
    foreach (int sourceId, labelMap.keys()) {
      int targetId = labelMap.value(sourceId);
      if (mergeMap.contains(targetId)) {
        std::vector<int> &idArray = mergeMap[targetId];
        idArray.push_back(sourceId);
      } else {
        mergeMap[targetId] = std::vector<int>();
        mergeMap[targetId].push_back(sourceId);
      }
    }

    foreach (int targetId, mergeMap.keys()) {
      dvidWriter.mergeBody(
            m_dvidTarget.getBodyLabelName(),
            targetId, mergeMap.value(targetId));
    }
    bodyMerger->clear();
  }
}

void ZFlyEmBodyMergeProject::detachBodyWindow()
{
  m_bodyWindow = NULL;
}

void ZFlyEmBodyMergeProject::showBody3d()
{
  if (m_bodyWindow == NULL) {
    ZStackDoc *doc = new ZStackDoc(NULL, NULL);
    ZWindowFactory factory;
    factory.setControlPanelVisible(false);
    factory.setObjectViewVisible(false);

    QRect rect;
    if (m_dataFrame != NULL) {
      rect = m_dataFrame->getViewGeometry();
    } else {
      if (getDocument()->getParentMvc() != NULL) {
        rect = getDocument()->getParentMvc()->getViewGeometry();
      }
    }
    if (rect.isValid()) {
      rect.moveTo(rect.right(), rect.top());
      rect.setSize(rect.size() / 2);
      factory.setWindowGeometry(rect);
    }

    m_bodyWindow = factory.make3DWindow(doc);
    //m_bodyWindow->setParent(m_dataFrame);

    connect(m_bodyWindow, SIGNAL(closed()), this, SLOT(detachBodyWindow()));
    m_bodyWindow->getSwcFilter()->setColorMode("Intrinsic");
    m_bodyWindow->getSwcFilter()->setRenderingPrimitive("Sphere");
    m_bodyWindow->getSwcFilter()->setStayOnTop(false);
    m_bodyWindow->setYZView();

    update3DBodyView();
  } else {
    m_bodyWindow->show();
    m_bodyWindow->raise();
  }
}

void ZFlyEmBodyMergeProject::update3DBodyView()
{
  if (m_bodyWindow != NULL) {
    std::set<std::string> currentBodySourceSet;
    for (QSet<uint64_t>::const_iterator iter = m_currentSelected.begin();
         iter != m_currentSelected.end(); ++iter) {
      currentBodySourceSet.insert(
            ZStackObjectSourceFactory::MakeFlyEmBodySource(*iter));
    }

    std::set<std::string> oldBodySourceSet;
    QList<ZSwcTree*> bodyList = m_bodyWindow->getDocument()->getSwcList();
    for (QList<ZSwcTree*>::iterator iter = bodyList.begin();
         iter != bodyList.end(); ++iter) {
      ZSwcTree *tree = *iter;
      if (currentBodySourceSet.count(tree->getSource()) == 0) {
        m_bodyWindow->getDocument()->removeObject(
              dynamic_cast<ZStackObject*>(tree), true);
      } else {
        oldBodySourceSet.insert(tree->getSource());
      }
    }

    ZFlyEmDvidReader reader;
    reader.open(getDvidTarget());

    if (getDataFrame() != NULL) {
      ZStack *oldStack = m_bodyWindow->getDocument()->getStack();
      ZStack *newStack = getDataFrame()->document()->getStack();
      if (oldStack != NULL) {
        if (oldStack->getBoundBox().equals(newStack->getBoundBox())) {
          newStack = NULL;
        }
      }
      if (newStack != NULL) {
        m_bodyWindow->getDocument()->loadStack(newStack->clone());
      }
    }

    ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
    m_bodyWindow->getDocument()->blockSignals(true);

    for (QSet<uint64_t>::const_iterator iter = m_currentSelected.begin();
         iter != m_currentSelected.end(); ++iter) {
      uint64_t label = *iter;
      std::string source = ZStackObjectSourceFactory::MakeFlyEmBodySource(label);
      if (oldBodySourceSet.count(source) == 0) {
        ZObject3dScan body = reader.readCoarseBody(label);
        if (!body.isEmpty()) {
//          body.setColor(sparseObject->getColor());
          if (getDocument<ZFlyEmProofDoc>() != NULL) {
            ZDvidLabelSlice *labelSlice =
                getDocument<ZFlyEmProofDoc>()->getDvidLabelSlice();
            if (labelSlice != NULL) {
              body.setColor(labelSlice->getColorScheme().getColor(label));
            }
          }
          body.setAlpha(255);
          ZSwcTree *tree = ZSwcGenerator::createSurfaceSwc(body);
          tree->translate(-dvidInfo.getStartBlockIndex());
          tree->rescale(dvidInfo.getBlockSize().getX(),
                        dvidInfo.getBlockSize().getY(),
                        dvidInfo.getBlockSize().getZ());
          tree->translate(dvidInfo.getStartCoordinates());
          tree->setSource(source);
          m_bodyWindow->getDocument()->addSwcTree(tree, true);
        }
      }
    }
    m_bodyWindow->getDocument()->blockSignals(false);
    m_bodyWindow->getDocument()->notifySwcModified();

    m_bodyWindow->show();
    m_bodyWindow->raise();
    m_bodyWindow->resetCameraCenter();
  }
}

void ZFlyEmBodyMergeProject::update3DBodyView(
    const ZStackObjectSelector &selector)
{
  if (m_bodyWindow != NULL) {
//    m_bodyWindow->getDocument()->removeAllObject();
    std::vector<ZStackObject*> objList =
        selector.getSelectedList(ZStackObject::TYPE_OBJECT3D_SCAN);
    ZFlyEmDvidReader reader;
    reader.open(getDvidTarget());

    ZStack *oldStack = m_bodyWindow->getDocument()->getStack();
    ZStack *newStack = getDataFrame()->document()->getStack();
    if (oldStack != NULL) {
      if (oldStack->getBoundBox().equals(newStack->getBoundBox())) {
        newStack = NULL;
      }
    }
    if (newStack != NULL) {
      m_bodyWindow->getDocument()->loadStack(newStack->clone());
    }

    ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
    m_bodyWindow->getDocument()->blockSignals(true);
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
                getDocument<ZFlyEmProofDoc>()->getDvidLabelSlice();
            if (labelSlice != NULL) {
              body.setColor(labelSlice->getColorScheme().getColor(label));
            }
          }
          body.setAlpha(255);
          //        tic();
          ZSwcTree *tree = ZSwcGenerator::createSurfaceSwc(body);
          tree->translate(-dvidInfo.getStartBlockIndex());
          tree->rescale(dvidInfo.getBlockSize().getX(),
                        dvidInfo.getBlockSize().getY(),
                        dvidInfo.getBlockSize().getZ());
          tree->translate(dvidInfo.getStartCoordinates());
          tree->setSource(ZStackObjectSourceFactory::MakeFlyEmBodySource(label));

          //Add bound box
          /*
          ZObject3dScan slice =
              body.getSlice(dvidInfo.getBlockIndexZ(getCurrentZ()));
          ZIntCuboid box = slice.getBoundBox();
          ZSwcTree *boxTree = ZSwcGenerator::createBoxSwc(box, 0.1);
          tree->merge(boxTree, true);
          */

          //        ptoc();

          m_bodyWindow->getDocument()->addSwcTree(tree, true);
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
        ZStackObject *obj = m_bodyWindow->getDocument()->getObjectGroup().
            findFirstSameSource(
              ZStackObject::TYPE_SWC,
              ZStackObjectSourceFactory::MakeFlyEmBodySource(label));
        if (obj != NULL) {
          m_bodyWindow->getDocument()->removeObject(obj, true);
        }
      }
    }
    m_bodyWindow->getDocument()->blockSignals(false);
    m_bodyWindow->getDocument()->notifySwcModified();

    m_bodyWindow->show();
    m_bodyWindow->raise();
    m_bodyWindow->resetCameraCenter();
  }
}

int ZFlyEmBodyMergeProject::getSelectedBodyId() const
{
  int bodyId = -1;
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
  int bodyId = getSelectedBodyId();
  if (bodyId > 0) {
    emit splitSent(m_dvidTarget, bodyId);
  }
}


void ZFlyEmBodyMergeProject::addSelected(uint64_t label)
{
  m_currentSelected.insert(label);
}

void ZFlyEmBodyMergeProject::removeSelected(uint64_t label)
{
  m_currentSelected.remove(label);
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

ZStackDoc* ZFlyEmBodyMergeProject::getDocument() const
{
  return m_doc.get();
}

ZFlyEmBodyMerger* ZFlyEmBodyMergeProject::getBodyMerger()
{
  if (getDocument<ZFlyEmBodyMergeDoc>() != NULL) {
    return getDocument<ZFlyEmBodyMergeDoc>()->getBodyMerger();
  }

  if (getDocument<ZFlyEmProofDoc>() != NULL) {
    return getDocument<ZFlyEmProofDoc>()->getBodyMerger();
  }

  return NULL;
}

void ZFlyEmBodyMergeProject::setSelection(const std::set<uint64_t> &selected)
{
  m_currentSelected.clear();
  for (std::set<uint64_t>::const_iterator iter = selected.begin();
       iter != selected.end(); ++iter) {
    m_currentSelected.insert(*iter);
  }
}

void ZFlyEmBodyMergeProject::syncWithDvid()
{
  if (getDvidTarget().isValid()) {
    ZFlyEmBodyMerger *bodyMerger = getBodyMerger();
    if (bodyMerger != NULL) {
      ZDvidBufferReader reader;
      reader.read(ZDvidUrl(getDvidTarget()).getMergeOperationUrl().c_str());
      const QByteArray& buffer = reader.getBuffer();
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

#include "zflyembody3ddoc.h"

#include <QtConcurrentRun>
#include <QMutexLocker>
#include <QElapsedTimer>

#include "dvid/zdvidreader.h"
#include "dvid/zdvidinfo.h"
#include "zswcfactory.h"
#include "zstackobjectsourcefactory.h"
#include "z3dgraphfactory.h"
#include "zflyemproofdoc.h"
#include "dvid/zdvidlabelslice.h"
#include "zwidgetmessage.h"
#include "dvid/zdvidsparsestack.h"
#include "zstring.h"

ZFlyEmBody3dDoc::ZFlyEmBody3dDoc(QObject *parent) :
  ZStackDoc(parent), m_bodyType(BODY_FULL), m_quitting(false),
  m_showingSynapse(true), m_garbageJustDumped(false)
{
  m_timer = new QTimer(this);
  m_timer->setInterval(200);
  m_timer->start();

  m_garbageTimer = new QTimer(this);
  m_garbageTimer->setInterval(60000);
  m_garbageTimer->start();

  enableAutoSaving(false);

  connectSignalSlot();
}

ZFlyEmBody3dDoc::~ZFlyEmBody3dDoc()
{
  m_quitting = true;

  QMutexLocker locker(&m_eventQueueMutex);
  m_eventQueue.clear();
  locker.unlock();

  m_futureMap.waitForFinished();

  m_garbageJustDumped = false;
  clearGarbage();
}

void ZFlyEmBody3dDoc::connectSignalSlot()
{
  connect(m_timer, SIGNAL(timeout()), this, SLOT(processEvent()));
  connect(m_garbageTimer, SIGNAL(timeout()), this, SLOT(clearGarbage()));
}

void ZFlyEmBody3dDoc::updateBodyFunc()
{
}

void ZFlyEmBody3dDoc::clearGarbage()
{
  QMutexLocker locker(&m_garbageMutex);

  if (!m_garbageJustDumped) {
#ifdef _DEBUG_
    std::cout << "Clear garbage objects ..." << std::endl;
#endif

    for (QList<ZStackObject*>::iterator iter = m_garbageList.begin();
         iter != m_garbageList.end(); ++iter) {
      ZStackObject *obj = *iter;
      std::cout << "Deleting " << obj << std::endl;
      delete obj;
    }

    m_garbageList.clear();
  }

  m_garbageJustDumped = false;
}

void ZFlyEmBody3dDoc::processBodySetBuffer()
{
  /*
  for (QSet<uint64_t>::const_iterator iter = m_bodySetBuffer.begin();
       iter != m_bodySetBuffer.end(); ++iter) {
    if (!m_bodySetBuffer.contains(*iter)) {
      addEvent(BodyEvent::ACTION_ADD, *iter);
    }
  }

  for (QSet<uint64_t>::const_iterator iter = m_bodySet.begin();
       iter != m_bodySet.end(); ++iter) {
    if (!m_bodySetBuffer.contains(*iter)) {
      addEvent(BodyEvent::ACTION_REMOVE, *iter);
    }
  }
*/
//  m_bodySetBuffer.clear();
}

const ZFlyEmBody3dDoc::BodyEvent::TUpdateFlag
ZFlyEmBody3dDoc::BodyEvent::UPDATE_CHANGE_COLOR = 1;

const ZFlyEmBody3dDoc::BodyEvent::TUpdateFlag
ZFlyEmBody3dDoc::BodyEvent::UPDATE_ADD_SYNAPSE = 2;

void ZFlyEmBody3dDoc::BodyEvent::print() const
{
  switch (m_action) {
  case ACTION_UPDATE:
    std::cout << "Update: ";
    break;
  case ACTION_ADD:
    std::cout << "Add: ";
    break;
  case ACTION_REMOVE:
    std::cout << "Remove: ";
    break;
  case ACTION_NULL:
    std::cout << "No action: ";
    break;
  }

  std::cout << getBodyId() << std::endl;
}

void ZFlyEmBody3dDoc::BodyEvent::mergeEvent(
    const BodyEvent &event, NeuTube::EBiDirection direction)
{
  if (getBodyId() != event.getBodyId())  {
    return;
  }

  switch (direction) {
  case NeuTube::DIRECTION_FORWARD: //event comes first
    switch (getAction()) {
    case ACTION_UPDATE:
      switch (event.getAction()) {
      case ACTION_ADD:
        m_action = ACTION_ADD;
        m_bodyColor = event.m_bodyColor;
//        m_refreshing |= event.m_refreshing;
        break;
      case ACTION_UPDATE:
        addUpdateFlag(event.getUpdateFlag());
//        m_refreshing |= event.m_refreshing;
        break;
      case ACTION_REMOVE:
        m_action = ACTION_REMOVE;
        break;
      default:
        break;
      }
      break;
    case ACTION_ADD:
      if (event.getAction() == ACTION_ADD || event.getAction() == ACTION_UPDATE) {
//        m_refreshing |= event.m_refreshing;
      }
      break;
    case ACTION_NULL:
      *this = event;
      break;
    default:
      break;
    }
    break;
  case NeuTube::DIRECTION_BACKWARD:
  {
    BodyEvent tmpEvent = event;
    tmpEvent.mergeEvent(*this, NeuTube::DIRECTION_FORWARD);
    *this = tmpEvent;
  }
    break;
  }
}

void ZFlyEmBody3dDoc::setDataDoc(ZSharedPointer<ZStackDoc> doc)
{
  m_dataDoc = doc;
}

ZFlyEmProofDoc* ZFlyEmBody3dDoc::getDataDocument() const
{
  return qobject_cast<ZFlyEmProofDoc*>(m_dataDoc.get());
}

void ZFlyEmBody3dDoc::processEventFunc(const BodyEvent &event)
{
  switch (event.getAction()) {
  case BodyEvent::ACTION_REMOVE:
    removeBodyFunc(event.getBodyId());
    break;
  case BodyEvent::ACTION_ADD:
    addBodyFunc(event.getBodyId(), event.getBodyColor());
    break;
  case BodyEvent::ACTION_UPDATE:
//    if (event.updating(BodyEvent::UPDATE_CHANGE_COLOR)) {
      updateBody(event.getBodyId(), event.getBodyColor());
//    }
    if (event.updating(BodyEvent::UPDATE_ADD_SYNAPSE)) {
      addSynapse(event.getBodyId());
    }
    break;
  default:
    break;
  }
}

void ZFlyEmBody3dDoc::processEvent(const BodyEvent &event)
{
  switch (event.getAction()) {
  case BodyEvent::ACTION_REMOVE:
    removeBody(event.getBodyId());
    break;
  case BodyEvent::ACTION_ADD:
    addBody(event.getBodyId(), event.getBodyColor());
    break;
  case BodyEvent::ACTION_UPDATE:
    updateBody(event.getBodyId(), event.getBodyColor());
    if (event.updating(BodyEvent::UPDATE_ADD_SYNAPSE)) {
      addSynapse(event.getBodyId());
    }
    break;
  default:
    break;
  }
}

void ZFlyEmBody3dDoc::processEventFunc()
{
  std::cout << "Locking process event" << std::endl;
  QMutexLocker locker(&m_eventQueueMutex);

//  QSet<uint64_t> bodySet = m_bodySet;
  QMap<uint64_t, BodyEvent> m_actionMap;
  for (QQueue<BodyEvent>::const_iterator iter = m_eventQueue.begin();
       iter != m_eventQueue.end(); ++iter) {
    const BodyEvent &event = *iter;
    uint64_t bodyId = event.getBodyId();
    if (m_actionMap.contains(bodyId)) {
      m_actionMap[bodyId].mergeEvent(event, NeuTube::DIRECTION_BACKWARD);
    } else {
      m_actionMap[bodyId] = event;
    }
  }

  for (QMap<uint64_t, BodyEvent>::iterator iter = m_actionMap.begin();
       iter != m_actionMap.end(); ++iter) {
    BodyEvent &event = iter.value();
    if (event.getAction() == BodyEvent::ACTION_ADD) {
      if (m_bodySet.contains(event.getBodyId())) {
        event.setAction(BodyEvent::ACTION_UPDATE);
      } else {
        m_bodySet.insert(event.getBodyId());
      }
    } else if (event.getAction() == BodyEvent::ACTION_REMOVE) {
      if (m_bodySet.contains(event.getBodyId())) {
        m_bodySet.remove(event.getBodyId());
      } else {
        event.setAction(BodyEvent::ACTION_NULL);
      }
    }
//    event.print();
  }

  m_eventQueue.clear();
  locker.unlock();
  std::cout << "Unlock process event" << std::endl;

  if (!m_actionMap.isEmpty()) {
    emit messageGenerated(ZWidgetMessage("Syncing 3D Body view ..."));

    std::cout << "====Processing Event====" << std::endl;
    for (QMap<uint64_t, BodyEvent>::const_iterator iter = m_actionMap.begin();
         iter != m_actionMap.end(); ++iter) {
      const BodyEvent &event = iter.value();
      event.print();
    }
  }

  for (QMap<uint64_t, BodyEvent>::const_iterator iter = m_actionMap.begin();
       iter != m_actionMap.end(); ++iter) {
    const BodyEvent &event = iter.value();
    processEventFunc(event);
    if (m_quitting) {
      break;
    }
  }

  emit messageGenerated(ZWidgetMessage("3D Body view updated."));
  std::cout << "====Processing done====" << std::endl;
}

void ZFlyEmBody3dDoc::processEvent()
{
  if (m_eventQueue.empty()) {
    return;
  }

  QString threadId = QString("processEvent()");

  if (!m_futureMap.isAlive(threadId)) {
    m_futureMap.removeDeadThread();
    QFuture<void> future =
        QtConcurrent::run(this, &ZFlyEmBody3dDoc::processEventFunc);
    m_futureMap[threadId] = future;
  }
}

bool ZFlyEmBody3dDoc::hasBody(uint64_t bodyId)
{
  return m_bodySet.contains(bodyId);
}

void ZFlyEmBody3dDoc::addBody(uint64_t bodyId, const QColor &color)
{
  if (!hasBody(bodyId)) {
    m_bodySet.insert(bodyId);
    addBodyFunc(bodyId, color);
  }
}

void ZFlyEmBody3dDoc::setBodyType(EBodyType type)
{
  m_bodyType = type;
  switch (m_bodyType) {
  case BODY_COARSE:
    setTag(NeuTube::Document::FLYEM_QUICK_BODY_COARSE);
    break;
  case BODY_FULL:
    setTag(NeuTube::Document::FLYEM_QUICK_BODY);
    break;
  case BODY_SKELETON:
    setTag(NeuTube::Document::FLYEM_SKELETON);
    break;
  }
}

void ZFlyEmBody3dDoc::updateBody(uint64_t bodyId, const QColor &color)
{
  beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  ZSwcTree *tree = getBodyModel(bodyId);
  if (tree != NULL) {
    if (tree->getColor() != color) {
      tree->setColor(color);
      processObjectModified(tree);
    }
  }
  endObjectModifiedMode();
  notifyObjectModified(true);
}

ZSwcTree* ZFlyEmBody3dDoc::getBodyModel(uint64_t bodyId)
{
  return retrieveBodyModel(bodyId);
}


void ZFlyEmBody3dDoc::addEvent(BodyEvent::EAction action, uint64_t bodyId,
                               BodyEvent::TUpdateFlag flag, QMutex *mutex)
{
  QMutexLocker locker(mutex);

  BodyEvent event(action, bodyId);
  event.addUpdateFlag(flag);
  if (getDataDocument() != NULL) {
    ZDvidLabelSlice *labelSlice =
        getDataDocument()->getDvidLabelSlice(NeuTube::Z_AXIS);

    if (labelSlice != NULL) {
      QColor color = labelSlice->getColor(bodyId, NeuTube::BODY_LABEL_ORIGINAL);
      color.setAlpha(255);
      event.setBodyColor(color);
    }
  }

  m_eventQueue.enqueue(event);
}

void ZFlyEmBody3dDoc::addBodyFunc(uint64_t bodyId, const QColor &color)
{
  ZSwcTree *tree = getBodyModel(bodyId);
  if (tree == NULL) {
    tree = makeBodyModel(bodyId);
  }

  if (tree != NULL) {
#ifdef _DEBUG_
    std::cout << "Adding object: " << dynamic_cast<ZStackObject*>(tree) << std::endl;
#endif
    tree->setColor(color);

//    delete tree;
    beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
    addObject(tree, true);
    processObjectModified(tree);
    endObjectModifiedMode();
    notifyObjectModified(true);

    //Add synapse
    if (m_showingSynapse) {
      beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
//      std::vector<ZPunctum*> puncta = getDataDocument()->getTbar(bodyId);
      std::pair<std::vector<ZPunctum*>, std::vector<ZPunctum*> > synapse =
          getDataDocument()->getSynapse(bodyId);
      {
        std::vector<ZPunctum*> &puncta = synapse.first;
        for (std::vector<ZPunctum*>::const_iterator iter = puncta.begin();
             iter != puncta.end(); ++iter) {
          ZPunctum *punctum = *iter;
          punctum->setRadius(30);
          punctum->setColor(255, 255, 0);
          punctum->setSource(ZStackObjectSourceFactory::MakeFlyEmTBarSource(bodyId));
          addObject(punctum, false);
        }
      }
      {
        std::vector<ZPunctum*> &puncta = synapse.second;
        for (std::vector<ZPunctum*>::const_iterator iter = puncta.begin();
             iter != puncta.end(); ++iter) {
          ZPunctum *punctum = *iter;
          punctum->setRadius(30);
          punctum->setColor(128, 128, 128);
          punctum->setSource(ZStackObjectSourceFactory::MakeFlyEmPsdSource(bodyId));
          addObject(punctum, false);
        }
      }

      endObjectModifiedMode();
      notifyObjectModified(true);
    }
//    removeObject(tree->getSource(), true);
//    removeObject(tree, true);
  }
}

void ZFlyEmBody3dDoc::addSynapse(bool on)
{
  if (on) {
    for (QSet<uint64_t>::const_iterator iter = m_bodySet.begin();
         iter != m_bodySet.end(); ++iter) {
      uint64_t bodyId = *iter;
      addEvent(BodyEvent::ACTION_UPDATE, bodyId, BodyEvent::UPDATE_ADD_SYNAPSE);
    }
  }
}

void ZFlyEmBody3dDoc::showSynapse(bool on)
{
  m_showingSynapse = on;
  addSynapse(on);
}

void ZFlyEmBody3dDoc::addSynapse(uint64_t bodyId)
{
  if (m_showingSynapse) {
    beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
//      std::vector<ZPunctum*> puncta = getDataDocument()->getTbar(bodyId);

    if (getObjectGroup().findFirstSameSource(
          ZStackObject::TYPE_PUNCTUM,
          ZStackObjectSourceFactory::MakeFlyEmTBarSource(bodyId)) == NULL) {
      std::pair<std::vector<ZPunctum*>, std::vector<ZPunctum*> > synapse =
          getDataDocument()->getSynapse(bodyId);
      {
        std::vector<ZPunctum*> &puncta = synapse.first;
        for (std::vector<ZPunctum*>::const_iterator iter = puncta.begin();
             iter != puncta.end(); ++iter) {
          ZPunctum *punctum = *iter;
          punctum->setRadius(30);
          punctum->setColor(255, 255, 0);
          punctum->setSource(ZStackObjectSourceFactory::MakeFlyEmTBarSource(bodyId));
          addObject(punctum, false);
        }
      }
      {
        std::vector<ZPunctum*> &puncta = synapse.second;
        for (std::vector<ZPunctum*>::const_iterator iter = puncta.begin();
             iter != puncta.end(); ++iter) {
          ZPunctum *punctum = *iter;
          punctum->setRadius(30);
          punctum->setColor(128, 128, 128);
          punctum->setSource(ZStackObjectSourceFactory::MakeFlyEmPsdSource(bodyId));
          addObject(punctum, false);
        }
      }
    }

    endObjectModifiedMode();
    notifyObjectModified(true);
  }
}

void ZFlyEmBody3dDoc::removeBody(uint64_t bodyId)
{
  m_bodySet.remove(bodyId);
  removeBodyFunc(bodyId);
}

void ZFlyEmBody3dDoc::removeBodyFunc(uint64_t bodyId)
{
  QString threadId = QString("removeBody(%1)").arg(bodyId);
  if (!m_futureMap.isAlive(threadId)) {
    TStackObjectList objList = getObjectGroup().findSameSource(
          ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId));
    beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
    for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
         ++iter) {
      removeObject(*iter, false);
      dumpGarbage(*iter);
    }

    objList = getObjectGroup().findSameSource(
          ZStackObjectSourceFactory::MakeFlyEmTBarSource(bodyId));
    for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
         ++iter) {
      removeObject(*iter, false);
      dumpGarbage(*iter);
    }

    objList = getObjectGroup().findSameSource(
          ZStackObjectSourceFactory::MakeFlyEmPsdSource(bodyId));
    for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
         ++iter) {
      removeObject(*iter, false);
      dumpGarbage(*iter);
    }

    endObjectModifiedMode();
    notifyObjectModified(true);
  }
}

ZSwcTree* ZFlyEmBody3dDoc::retrieveBodyModel(uint64_t bodyId)
{
  ZStackObject *obj = getObjectGroup().findFirstSameSource(
        ZStackObject::TYPE_SWC,
        ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId));

  ZSwcTree *tree = dynamic_cast<ZSwcTree*>(obj);

  return tree;
}

ZSwcTree* ZFlyEmBody3dDoc::makeBodyModel(uint64_t bodyId)
{
  ZSwcTree *tree = NULL;

  if (bodyId > 0) {
    if (getBodyType() == BODY_SKELETON) {
      ZDvidReader reader;
      if (reader.open(getDvidTarget())) {
        tree = reader.readSwc(bodyId);
      }
    } else if (getBodyType() == BODY_COARSE) {
      ZDvidReader reader;
      if (reader.open(getDvidTarget())) {
        ZObject3dScan obj = reader.readCoarseBody(bodyId);
        if (!obj.isEmpty()) {
          tree = ZSwcFactory::CreateSurfaceSwc(obj);
          tree->translate(-m_dvidInfo.getStartBlockIndex());
          tree->rescale(m_dvidInfo.getBlockSize().getX(),
                        m_dvidInfo.getBlockSize().getY(),
                        m_dvidInfo.getBlockSize().getZ());
          tree->translate(m_dvidInfo.getStartCoordinates());
        }
      }
    } else {
      ZDvidSparseStack *cachedStack = getDataDocument()->getBodyForSplit();
      ZObject3dScan *cachedBody = NULL;
      if (cachedStack != NULL) {
        if (cachedStack->getObjectMask() != NULL) {
          if (cachedStack->getObjectMask()->getLabel() == bodyId) {
            cachedBody = cachedStack->getObjectMask();
          }
        }
      }

      if (cachedBody == NULL) {
        ZDvidReader reader;
        if (reader.open(getDvidTarget())) {
          ZObject3dScan obj;
          reader.readBody(bodyId, &obj);
          if (!obj.isEmpty()) {
            obj.canonize();
            tree = ZSwcFactory::CreateSurfaceSwc(obj, 3);
          }
        }
      } else {
        tree = ZSwcFactory::CreateSurfaceSwc(*cachedBody);
      }
    }

    if (tree != NULL) {
      tree->setSource(ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId));
    }
  }

  return tree;
}

const ZDvidInfo& ZFlyEmBody3dDoc::getDvidInfo() const
{
  return m_dvidInfo;
}

void ZFlyEmBody3dDoc::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
  updateDvidInfo();
}

void ZFlyEmBody3dDoc::updateDvidInfo()
{
  m_dvidInfo.clear();

  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    m_dvidInfo = reader.readGrayScaleInfo();
  }
}

/*
void ZFlyEmBody3dDoc::updateFrame()
{
  ZCuboid box;
  box.setFirstCorner(getDvidInfo().getStartCoordinates().toPoint());
  box.setLastCorner(getDvidInfo().getEndCoordinates().toPoint());
  Z3DGraph *graph = Z3DGraphFactory::MakeBox(
        box, dmax2(1.0, dmax3(box.width(), box.height(), box.depth()) / 1000.0));
  graph->setSource(ZStackObjectSourceFactory::MakeFlyEmBoundBoxSource());

  addObject(graph, true);
}
*/

void ZFlyEmBody3dDoc::printEventQueue() const
{
  for (QQueue<BodyEvent>::const_iterator iter = m_eventQueue.begin();
       iter != m_eventQueue.end(); ++iter) {
    const BodyEvent &event  = *iter;
    event.print();
  }
}

void ZFlyEmBody3dDoc::dumpGarbage(ZStackObject *obj)
{
  QMutexLocker locker(&m_garbageMutex);

  m_garbageList.append(obj);
  m_garbageJustDumped = true;
}

void ZFlyEmBody3dDoc::dumpAllSwc()
{
  QList<ZSwcTree*> treeList = getSwcList();
  for (QList<ZSwcTree*>::const_iterator iter = treeList.begin();
       iter != treeList.end(); ++iter) {
    ZSwcTree *tree = *iter;
    removeObject(tree, false);
    dumpGarbage(tree);
  }
  m_bodySet.clear();
}

void ZFlyEmBody3dDoc::mergeBodyModel(const ZFlyEmBodyMerger &merger)
{
  if (!merger.isEmpty()) {
    QMap<uint64_t, ZSwcTree*> treeMap;
    QList<ZSwcTree*> treeList = getSwcList();
    for (QList<ZSwcTree*>::const_iterator iter = treeList.begin();
         iter != treeList.end(); ++iter) {
      ZSwcTree *tree = *iter;
      if (tree != NULL) {
        uint64_t bodyId =
            ZStackObjectSourceFactory::ExtractIdFromFlyEmBodySource(
              tree->getSource());
        if (bodyId > 0) {
          treeMap[bodyId] = tree;
        }
      }
    }

    for (QMap<uint64_t, ZSwcTree*>::iterator iter = treeMap.begin();
         iter != treeMap.end(); ++iter) {
      uint64_t bodyId = iter.key();
      ZSwcTree *tree = iter.value();
      uint64_t finalLabel = merger.getFinalLabel(bodyId);
      if (finalLabel != bodyId) {
        ZSwcTree *targetTree = treeMap[finalLabel];
        if (targetTree == NULL) {
          removeObject(tree, false);
        } else {
          targetTree->merge(tree);
        }
        dumpGarbage(tree);
      }
    }
    removeEmptySwcTree(false);
  }
}

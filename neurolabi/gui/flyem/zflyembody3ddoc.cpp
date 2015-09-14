#include "zflyembody3ddoc.h"

#include <QtConcurrentRun>
#include <QMutexLocker>

#include "dvid/zdvidreader.h"
#include "dvid/zdvidinfo.h"
#include "zswcfactory.h"
#include "zstackobjectsourcefactory.h"
#include "z3dgraphfactory.h"
#include "zflyemproofdoc.h"
#include "dvid/zdvidlabelslice.h"
#include "zwidgetmessage.h"
#include "dvid/zdvidsparsestack.h"

ZFlyEmBody3dDoc::ZFlyEmBody3dDoc(QObject *parent) :
  ZStackDoc(parent), m_garbageJustDumped(false)
{
  m_timer = new QTimer(this);
  m_timer->setInterval(200);
  m_timer->start();

  m_garbageTimer = new QTimer(this);
  m_garbageTimer->setInterval(60000);
  m_garbageTimer->start();

  connectSignalSlot();
}

ZFlyEmBody3dDoc::~ZFlyEmBody3dDoc()
{
  QMutexLocker locker(&m_eventQueueMutex);
  m_eventQueue.clear();
  locker.unlock();
//  m_eventQueueMutex.unlock();

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
        m_refreshing |= event.m_refreshing;
        break;
      case ACTION_UPDATE:
        m_refreshing |= event.m_refreshing;
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
        m_refreshing |= event.m_refreshing;
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
  return dynamic_cast<ZFlyEmProofDoc*>(m_dataDoc.get());
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
    updateBody(event.getBodyId(), event.getBodyColor());
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
        event.setAction(BodyEvent::ACTION_NULL);
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


  /*
  for (QSet<uint64_t>::const_iterator iter = removingSet.begin();
       iter != removingSet.end(); ++iter) {
    uint64_t bodyId = *iter;
    removeBody(bodyId);
  }

  for (QSet<uint64_t>::const_iterator iter = addingSet.begin();
       iter != addingSet.end(); ++iter) {
    uint64_t bodyId = *iter;
    addBody(bodyId);
  }
  */

//  locker.unlock();
//  processBodySetBuffer();
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
    /*
    QString threadId = QString("addBody(%1)").arg(bodyId);
    if (!m_futureMap.isAlive(threadId)) {
      m_futureMap.removeDeadThread();
      QFuture<void> future =
          QtConcurrent::run(this, &ZFlyEmBody3dDoc::addBodyFunc, bodyId, color);
      m_futureMap[threadId] = future;
    }
    */
  }
}

void ZFlyEmBody3dDoc::updateBody(uint64_t bodyId, const QColor &color)
{

}

void ZFlyEmBody3dDoc::addEvent(
    BodyEvent::EAction action, uint64_t bodyId, QMutex *mutex)
{
  QMutexLocker locker(mutex);

  BodyEvent event(action, bodyId);
  if (getDataDocument() != NULL) {
    ZDvidLabelSlice *labelSlice = getDataDocument()->getDvidLabelSlice();

    if (labelSlice != NULL) {
      QColor color = labelSlice->getColor(bodyId, NeuTube::BODY_LABEL_MAPPED);
      color.setAlpha(255);
      event.setBodyColor(color);
    }
  }

  m_eventQueue.enqueue(event);
}

void ZFlyEmBody3dDoc::addBodyFunc(uint64_t bodyId, const QColor &color)
{
  ZSwcTree *tree = makeBodyModel(bodyId);
  if (tree != NULL) {
#ifdef _DEBUG_
    std::cout << "Adding object: " << dynamic_cast<ZStackObject*>(tree) << std::endl;
#endif
    tree->setColor(color);

//    delete tree;
    addObject(tree, true);

//    removeObject(tree->getSource(), true);
//    removeObject(tree, true);
  }
}

void ZFlyEmBody3dDoc::removeBody(uint64_t bodyId)
{
  m_bodySet.remove(bodyId);
  removeBodyFunc(bodyId);
}

void ZFlyEmBody3dDoc::removeBodyFunc(uint64_t bodyId)
{
  QString threadId = QString("addBody(%1)").arg(bodyId);
  if (!m_futureMap.isAlive(threadId)) {
    TStackObjectList objList = getObjectGroup().findSameSource(
          ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId));
    beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
    for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
         ++iter) {
      removeObject(*iter, false);
      dumpGarbage(*iter);
    }
    endObjectModifiedMode();
    notifyObjectModified();
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

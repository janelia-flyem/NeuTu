#include "zflyembody3ddoc.h"

#include <QtConcurrentRun>
#include <QMutexLocker>

#include "dvid/zdvidreader.h"
#include "dvid/zdvidinfo.h"
#include "zswcfactory.h"
#include "zstackobjectsourcefactory.h"
#include "z3dgraphfactory.h"

ZFlyEmBody3dDoc::ZFlyEmBody3dDoc(QObject *parent) :
  ZStackDoc(parent)
{
  m_timer = new QTimer(this);
  m_timer->setInterval(200);
  m_timer->start();

  connectSignalSlot();
}

void ZFlyEmBody3dDoc::connectSignalSlot()
{
  connect(m_timer, SIGNAL(timeout()), this, SLOT(processEvent()));
}

void ZFlyEmBody3dDoc::updateBodyFunc()
{

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

void ZFlyEmBody3dDoc::processEvent()
{
  if (m_futureMap.hasThreadAlive()) {
    return;
  }

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

  for (QMap<uint64_t, BodyEvent>::const_iterator iter = m_actionMap.begin();
       iter != m_actionMap.end(); ++iter) {
    const BodyEvent &event = iter.value();
    processEvent(event);
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

  m_eventQueue.clear();
}

bool ZFlyEmBody3dDoc::hasBody(uint64_t bodyId)
{
  return m_bodySet.contains(bodyId);
}

void ZFlyEmBody3dDoc::addBody(uint64_t bodyId, const QColor &color)
{
  QString threadId = QString("addBody(%1)").arg(bodyId);
  if (!m_futureMap.isAlive(threadId)) {
    m_futureMap.removeDeadThread();
    QFuture<void> future =
        QtConcurrent::run(this, &ZFlyEmBody3dDoc::addBodyFunc, bodyId, color);
    m_futureMap[threadId] = future;
  }
}

void ZFlyEmBody3dDoc::updateBody(uint64_t bodyId, const QColor &color)
{

}

void ZFlyEmBody3dDoc::addEvent(BodyEvent::EAction action, uint64_t bodyId)
{
  QMutexLocker locker(&m_eventQueueMutex);

  m_eventQueue.enqueue(BodyEvent(action, bodyId));
}

void ZFlyEmBody3dDoc::addBodyFunc(uint64_t bodyId, const QColor &color)
{
  if (!hasBody(bodyId)) {
    m_bodySet.insert(bodyId);
    ZSwcTree *tree = makeBodyModel(bodyId);
    if (tree != NULL) {
      addObject(tree, true);
      tree->setColor(color);
    }
  }
}

void ZFlyEmBody3dDoc::removeBody(uint64_t bodyId)
{
  removeBodyFunc(bodyId);
}

void ZFlyEmBody3dDoc::removeBodyFunc(uint64_t bodyId)
{
  QString threadId = QString("addBody(%1)").arg(bodyId);
  if (!m_futureMap.isAlive(threadId)) {
    m_bodySet.remove(bodyId);
    removeObject(ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId), true);
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
    ZDvidReader reader;
    if (reader.open(getDvidTarget())) {
      ZObject3dScan obj;
      reader.readBody(bodyId, &obj);
      if (!obj.isEmpty()) {
        obj.canonize();
        tree = ZSwcFactory::CreateSurfaceSwc(obj, 3);
        if (tree != NULL) {
          tree->setSource(ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId));
        }
      }
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

void ZFlyEmBody3dDoc::updateFrame()
{
  ZCuboid box;
  box.setFirstCorner(getDvidInfo().getStartCoordinates().toPoint());
  box.setLastCorner(getDvidInfo().getEndCoordinates().toPoint());
  Z3DGraph *graph = Z3DGraphFactory::MakeBox(
        box, dmax2(1.0, dmax3(box.width(), box.height(), box.depth()) / 500.0));
  graph->setSource(ZStackObjectSourceFactory::MakeFlyEmBoundBoxSource());

  addObject(graph, true);
}

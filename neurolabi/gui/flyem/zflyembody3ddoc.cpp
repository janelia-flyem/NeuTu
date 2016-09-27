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
#include "neutubeconfig.h"

const int ZFlyEmBody3dDoc::OBJECT_GARBAGE_LIFE = 30000;
const int ZFlyEmBody3dDoc::OBJECT_ACTIVE_LIFE = 15000;

ZFlyEmBody3dDoc::ZFlyEmBody3dDoc(QObject *parent) :
  ZStackDoc(parent), m_bodyType(BODY_FULL), m_quitting(false),
  m_showingSynapse(true), m_showingTodo(true), m_garbageJustDumped(false)
{
  m_timer = new QTimer(this);
  m_timer->setInterval(200);
  m_timer->start();

  m_garbageTimer = new QTimer(this);
  m_garbageTimer->setInterval(60000);
  m_garbageTimer->start();

  m_objectTime.start();

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
//  clearGarbage();

  QMutexLocker garbageLocker(&m_garbageMutex);
  for (QMap<ZStackObject*, ObjectStatus>::iterator iter = m_garbageMap.begin();
       iter != m_garbageMap.end(); ++iter) {
    delete iter.key();
  }
}

void ZFlyEmBody3dDoc::connectSignalSlot()
{
  connect(m_timer, SIGNAL(timeout()), this, SLOT(processEvent()));
  connect(m_garbageTimer, SIGNAL(timeout()), this, SLOT(clearGarbage()));
}

void ZFlyEmBody3dDoc::updateBodyFunc()
{
}

template<typename T>
T* ZFlyEmBody3dDoc::recoverFromGarbage(const std::string &source)
{
  QMutexLocker locker(&m_garbageMutex);

  T* obj = NULL;

  if (!source.empty()) {
    int currentTime = m_objectTime.elapsed();
    QMutableMapIterator<ZStackObject*, ObjectStatus> iter(m_garbageMap);
    int minDt = -1;
    while (iter.hasNext()) {
      iter.next();
      if (iter.value().isRecycable()) {
        int t = iter.value().getTimeStamp();
        int dt = currentTime - t;
        if (dt < 0) {
          iter.value() = 0;
        } else if (dt < OBJECT_ACTIVE_LIFE){
          if (minDt < 0 || minDt > dt) {
            if (iter.key()->getSource() == source) {
              uint64_t bodyId =
                  ZStackObjectSourceFactory::ExtractIdFromFlyEmBodySource(
                    source);
              bool recycable = true;
              obj = dynamic_cast<T*>(iter.key());
              if (m_bodyUpdateMap.contains(bodyId)) {
                if (m_bodyUpdateMap[bodyId] >= obj->getTimeStamp()) { //not recycable
                  recycable = false;
                }
              }
              if (recycable) {
                minDt = dt;
              } else {
                obj = NULL;
              }
            }
          }
        }
      }
    }
  }

  if (obj != NULL) {
    m_garbageMap.remove(obj);
  }

  return obj;
}

void ZFlyEmBody3dDoc::setUnrecycable(const QSet<uint64_t> &bodySet)
{
  QMutexLocker locker(&m_garbageMutex);
  int currentTime = m_objectTime.elapsed();
  for (QSet<uint64_t>::const_iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    m_bodyUpdateMap[*iter] = currentTime;
  }

//  m_unrecycableSet = bodySet;
  for (QMap<ZStackObject*, ObjectStatus>::iterator iter = m_garbageMap.begin();
       iter != m_garbageMap.end(); ++iter) {
    ZStackObject *obj = iter.key();
    if (obj != NULL) {
      ObjectStatus &status = iter.value();
      if (status.isRecycable()) {
        uint64_t bodyId =
            ZStackObjectSourceFactory::ExtractIdFromFlyEmBodySource(
              obj->getSource());
        if (bodySet.contains(bodyId)) {
          status.setRecycable(false);
        }
      }
    }
  }
}

void ZFlyEmBody3dDoc::clearGarbage()
{
  QMutexLocker locker(&m_garbageMutex);

  ZOUT(LTRACE(), 5) << "Clear garbage objects ...";

  int currentTime = m_objectTime.elapsed();
  QMutableMapIterator<ZStackObject*, ObjectStatus> iter(m_garbageMap);
   while (iter.hasNext()) {
     iter.next();
     int t = iter.value().getTimeStamp();
     int dt = currentTime - t;
     if (dt < 0) {
       iter.value() = 0;
     } else if (dt > OBJECT_GARBAGE_LIFE){
       ZStackObject *obj = iter.key();
       if (obj->getType() == ZStackObject::TYPE_SWC) {
         ZOUT(LTRACE(), 5) << "Deleting SWC object: " << obj;
       }
       delete iter.key();
       iter.remove();
     }
   }

#if 0
  if (!m_garbageJustDumped) {
    ZOUT(LINFO(), 3) << "Clear garbage objects ...";

    for (QList<ZStackObject*>::iterator iter = m_garbageList.begin();
         iter != m_garbageList.end(); ++iter) {
      ZStackObject *obj = *iter;
      ZOUT(LINFO(), 3)<< "Deleting " << obj;
      delete obj;
    }

    m_garbageList.clear();
  }
#endif

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

const ZFlyEmBody3dDoc::BodyEvent::TUpdateFlag
ZFlyEmBody3dDoc::BodyEvent::UPDATE_ADD_TODO_ITEM = 4;

const ZFlyEmBody3dDoc::BodyEvent::TUpdateFlag
ZFlyEmBody3dDoc::BodyEvent::UPDATE_MULTIRES = 8;

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
  case ACTION_FORCE_ADD:
    std::cout << "Force add: ";
    break;
  case ACTION_NULL:
    std::cout << "No action: ";
    break;
  }

  std::cout << getBodyId() << std::endl;
  std::cout << "  resolution: " << getResLevel() << std::endl;
  std::cout << "  flag: " << getUpdateFlag() << std::endl;
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
        m_updateFlag |= event.m_updateFlag;
        if (getResLevel() < event.getResLevel()) {
          setResLevel(event.getResLevel());
        }
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

  if (getDataDocument() != NULL) {
    connect(getDataDocument(), SIGNAL(todoModified(uint64_t)),
            this, SLOT(updateTodo(uint64_t)));
  }
}

ZFlyEmProofDoc* ZFlyEmBody3dDoc::getDataDocument() const
{
  return qobject_cast<ZFlyEmProofDoc*>(m_dataDoc.get());
}

void ZFlyEmBody3dDoc::processEventFunc(const BodyEvent &event)
{
  switch (event.getAction()) {
  case BodyEvent::ACTION_REMOVE:
    removeBodyFunc(event.getBodyId(), true);
    break;
  case BodyEvent::ACTION_ADD:
    addBodyFunc(event.getBodyId(), event.getBodyColor(), event.getResLevel());
    break;
  case BodyEvent::ACTION_UPDATE:
//    if (event.updating(BodyEvent::UPDATE_CHANGE_COLOR)) {
    if (event.updating(BodyEvent::UPDATE_MULTIRES)) {
      addBodyFunc(event.getBodyId(), event.getBodyColor(), event.getResLevel());
    } else {
      updateBody(event.getBodyId(), event.getBodyColor(), getBodyType());
    }
//    }
    if (event.updating(BodyEvent::UPDATE_ADD_SYNAPSE)) {
      addSynapse(event.getBodyId());
    }
    if (event.updating(BodyEvent::UPDATE_ADD_TODO_ITEM)) {
      addTodo(event.getBodyId());
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
    if (event.updating(BodyEvent::UPDATE_CHANGE_COLOR)) {
      updateBody(event.getBodyId(), event.getBodyColor());
    }
    if (event.updating(BodyEvent::UPDATE_ADD_SYNAPSE)) {
      addSynapse(event.getBodyId());
    }
    if (event.updating(BodyEvent::UPDATE_ADD_TODO_ITEM)) {
      addTodo(event.getBodyId());
    }
    break;
  default:
    break;
  }
}

void ZFlyEmBody3dDoc::setTodoItemSelected(
    ZFlyEmToDoItem *item, bool select)
{
  getObjectGroup().setSelected(item, select);
}

bool ZFlyEmBody3dDoc::hasTodoItemSelected() const
{
  return !getObjectGroup().getSelectedSet(
        ZStackObject::TYPE_FLYEM_TODO_ITEM).empty();
}

ZFlyEmToDoItem* ZFlyEmBody3dDoc::getOneSelectedTodoItem() const
{
  ZFlyEmToDoItem *item = NULL;

  const TStackObjectSet &objSet = getObjectGroup().getSelectedSet(
        ZStackObject::TYPE_FLYEM_TODO_ITEM);
  if (!objSet.empty()) {
    item = const_cast<ZFlyEmToDoItem*>(
          dynamic_cast<const ZFlyEmToDoItem*>(*(objSet.begin())));
  }

  return item;
}

QMap<uint64_t, ZFlyEmBody3dDoc::BodyEvent> ZFlyEmBody3dDoc::makeEventMap(
    bool synced, QSet<uint64_t> &bodySet)
{
  if (synced) {
    std::cout << "Locking process event" << std::endl;
    QMutexLocker locker(&m_eventQueueMutex);
  }

//  QSet<uint64_t> bodySet = m_bodySet;
  QMap<uint64_t, BodyEvent> actionMap;
  for (QQueue<BodyEvent>::const_iterator iter = m_eventQueue.begin();
       iter != m_eventQueue.end(); ++iter) {
    const BodyEvent &event = *iter;
    uint64_t bodyId = event.getBodyId();
    if (actionMap.contains(bodyId)) {
      actionMap[bodyId].mergeEvent(event, NeuTube::DIRECTION_BACKWARD);
    } else {
      actionMap[bodyId] = event;
    }
  }

  for (QMap<uint64_t, BodyEvent>::iterator iter = actionMap.begin();
       iter != actionMap.end(); ++iter) {
    BodyEvent &event = iter.value();
    switch (event.getAction()) {
    case BodyEvent::ACTION_ADD:
      if (bodySet.contains(event.getBodyId())) {
        event.setAction(BodyEvent::ACTION_UPDATE);
      } else {
        bodySet.insert(event.getBodyId());
      }
      break;
    case BodyEvent::ACTION_FORCE_ADD:
      event.setAction(BodyEvent::ACTION_UPDATE);
      bodySet.insert(event.getBodyId());
      break;
    case BodyEvent::ACTION_REMOVE:
      if (bodySet.contains(event.getBodyId())) {
        bodySet.remove(event.getBodyId());
      } else {
        event.setAction(BodyEvent::ACTION_NULL);
      }
      break;
    default:
      break;
    }
//    event.print();
  }
  m_eventQueue.clear();

  return actionMap;
}

void ZFlyEmBody3dDoc::cancelEventThread()
{
  m_quitting = true;
  m_futureMap.waitForFinished();
  m_quitting = false;
}

void ZFlyEmBody3dDoc::processEventFunc()
{
  QMap<uint64_t, BodyEvent> actionMap = makeEventMap(true, m_bodySet);
  if (!actionMap.isEmpty()) {
    std::cout << "====Processing Event====" << std::endl;
    for (QMap<uint64_t, BodyEvent>::const_iterator iter = actionMap.begin();
         iter != actionMap.end(); ++iter) {
      const BodyEvent &event = iter.value();
      event.print();
    }
  }

  for (QMap<uint64_t, BodyEvent>::const_iterator iter = actionMap.begin();
       iter != actionMap.end(); ++iter) {
    const BodyEvent &event = iter.value();
    processEventFunc(event);
    if (m_quitting) {
      break;
    }
  }

//  emit messageGenerated(ZWidgetMessage("3D Body view updated."));
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
    if (getBodyType() == BODY_SKELETON) {
      addBodyFunc(bodyId, color, -1);
    } else {
      addBodyFunc(bodyId, color, 5);
    }
  }
}

void ZFlyEmBody3dDoc::setBodyType(EBodyType type)
{
  m_bodyType = type;
  switch (m_bodyType) {
  case BODY_COARSE:
    setTag(NeuTube::Document::FLYEM_BODY_3D_COARSE);
    break;
  case BODY_FULL:
    setTag(NeuTube::Document::FLYEM_BODY_3D);
    break;
  case BODY_SKELETON:
    setTag(NeuTube::Document::FLYEM_SKELETON);
    break;
  }
}

void ZFlyEmBody3dDoc::updateBody(
    uint64_t bodyId, const QColor &color)
{
  updateBody(bodyId, color, BODY_COARSE);
  updateBody(bodyId, color, BODY_FULL);
  updateBody(bodyId, color, BODY_SKELETON);
}

void ZFlyEmBody3dDoc::updateBody(
    uint64_t bodyId, const QColor &color, EBodyType type)
{
  beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  ZSwcTree *tree = getBodyModel(bodyId, type);
  if (tree != NULL) {
    if (tree->getColor() != color) {
      tree->setColor(color);
      processObjectModified(tree);
    }
  }
  endObjectModifiedMode();
  notifyObjectModified(true);
}

ZSwcTree* ZFlyEmBody3dDoc::getBodyModel(uint64_t bodyId, EBodyType bodyType)
{
  return retrieveBodyModel(bodyId, bodyType);
}

void ZFlyEmBody3dDoc::addEvent(const BodyEvent &event)
{
  m_eventQueue.append(event);
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
      QColor color;

      if (getBodyType() == BODY_FULL) { //using the original color
        color = labelSlice->getColor(bodyId, NeuTube::BODY_LABEL_MAPPED);
      } else {
        color = labelSlice->getColor(bodyId, NeuTube::BODY_LABEL_ORIGINAL);
      }
      color.setAlpha(255);
      event.setBodyColor(color);
    }
  }

  if (event.getAction() == BodyEvent::ACTION_ADD &&
      getBodyType() != BODY_SKELETON) {
    event.setResLevel(5);
  }

  m_eventQueue.enqueue(event);
}

void ZFlyEmBody3dDoc::addBodyFunc(
    uint64_t bodyId, const QColor &color, int resLevel)
{
  bool loaded =
      !(getObjectGroup().findSameClass(
          ZStackObject::TYPE_SWC,
          ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId)).isEmpty());

  ZSwcTree *tree = NULL;
  if (tree == NULL) {
    if (resLevel == 5) {
      tree = makeBodyModel(bodyId, ZFlyEmBody3dDoc::BODY_COARSE);
      if (tree != NULL) {
        if (tree->getSource() == ZStackObjectSourceFactory::MakeFlyEmBodySource(
              bodyId, GetBodyTypeName(BODY_FULL))) {
          resLevel = 0;
        }
      }
    } else if (resLevel == 0) {
      emit messageGenerated(ZWidgetMessage("Syncing 3D Body view ..."));
      tree = makeBodyModel(bodyId, getBodyType());
      emit messageGenerated(ZWidgetMessage("3D Body view synced"));
    }

    if (resLevel > 0 && getBodyType() == ZFlyEmBody3dDoc::BODY_FULL) {
      QMutexLocker locker(&m_eventQueueMutex);
      BodyEvent bodyEvent(BodyEvent::ACTION_ADD, bodyId);
      bodyEvent.setBodyColor(color);
      bodyEvent.setResLevel(--resLevel);
      bodyEvent.addUpdateFlag(BodyEvent::UPDATE_MULTIRES);
      bool removing = false;

      for (QQueue<BodyEvent>::iterator iter = m_eventQueue.begin();
           iter != m_eventQueue.end(); ++iter) {
        BodyEvent &event = *iter;
        if (event.getBodyId() == bodyId) {
          if (event.getAction() == BodyEvent::ACTION_REMOVE) {
            removing = true;
          } else {
            removing = false;
          }
        }
      }
      if (!removing) {
        m_eventQueue.enqueue(bodyEvent);
      }
    }
  }

  if (tree != NULL) {
//    if (getBodyType() != BODY_SKELETON) {
    tree->setStructrualMode(ZSwcTree::STRUCT_POINT_CLOUD);
//    }

#ifdef _DEBUG_
    std::cout << "Adding object: " << dynamic_cast<ZStackObject*>(tree) << std::endl;
#endif
    tree->setColor(color);

//    delete tree;
    beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
    removeBodyFunc(bodyId, false);
    TStackObjectList objList = takeObject(tree->getType(), tree->getSource());
    if (!objList.empty()) {
      dumpGarbage(objList.begin(), objList.end(), false);
    }
    addObject(tree, false);
    processObjectModified(tree);
    endObjectModifiedMode();
    notifyObjectModified(true);

    if (!loaded) {
      addSynapse(bodyId);
//      addTodo(bodyId);
      updateTodo(bodyId);
    }
    //Add synapse
#if 0
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
#if 0
      std::vector<ZPunctum*> todoPuncta =
          getDataDocument()->getTodoPuncta(bodyId);
      for (std::vector<ZPunctum*>::const_iterator iter = todoPuncta.begin();
           iter != todoPuncta.end(); ++iter) {
        ZPunctum *punctum = *iter;
//        punctum->setRadius(30);
//        punctum->setColor(128, 128, 128);
        punctum->setSource(ZStackObjectSourceFactory::MakeTodoPunctaSource(bodyId));
        addObject(punctum, false);
      }
#endif
      endObjectModifiedMode();
      notifyObjectModified(true);
    }
#endif
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

void ZFlyEmBody3dDoc::addTodo(bool on)
{
  if (on) {
    for (QSet<uint64_t>::const_iterator iter = m_bodySet.begin();
         iter != m_bodySet.end(); ++iter) {
      uint64_t bodyId = *iter;
      addEvent(BodyEvent::ACTION_UPDATE, bodyId, BodyEvent::UPDATE_ADD_TODO_ITEM);
    }
  }
}

void ZFlyEmBody3dDoc::showSynapse(bool on)
{
  m_showingSynapse = on;
  addSynapse(on);
}

void ZFlyEmBody3dDoc::showTodo(bool on)
{
  m_showingTodo = on;
  addTodo(on);
}

void ZFlyEmBody3dDoc::addSynapse(uint64_t bodyId)
{
  if (m_showingSynapse) {
    if (getObjectGroup().findFirstSameSource(
          ZStackObject::TYPE_PUNCTUM,
          ZStackObjectSourceFactory::MakeFlyEmTBarSource(bodyId)) == NULL) {
      beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
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
          if (punctum->name().isEmpty()) {
            punctum->setName(QString("%1").arg(bodyId));
          }
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
      notifyObjectModified();
    }
  }
}

void ZFlyEmBody3dDoc::updateTodo(uint64_t bodyId)
{
  if (m_showingTodo) {
    ZOUT(LTRACE(), 5) << "Add synases";

    std::string source = ZStackObjectSourceFactory::MakeTodoPunctaSource(bodyId);
//    removeObject(source, true);
    TStackObjectList objList = getObjectGroup().findSameSource(source);
    for (TStackObjectList::iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      removeObject(*iter, false);
      dumpGarbage(*iter, true);
    }

    if (hasBody(bodyId)) {
      beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);

      std::vector<ZFlyEmToDoItem*> itemList =
          getDataDocument()->getTodoItem(bodyId);

      for (std::vector<ZFlyEmToDoItem*>::const_iterator iter = itemList.begin();
           iter != itemList.end(); ++iter) {
        ZFlyEmToDoItem *item = *iter;
        item->setRadius(30);
        //        item->setColor(255, 255, 0);
        item->setSource(source);
        addObject(item, false);
      }

      endObjectModifiedMode();
      notifyObjectModified(true);
    }
  }
}

void ZFlyEmBody3dDoc::addTodo(uint64_t bodyId)
{
  if (m_showingTodo) {
    ZOUT(LTRACE(), 5) << "Add todo items";

    beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);

    std::string source = ZStackObjectSourceFactory::MakeTodoPunctaSource(bodyId);
    if (getObjectGroup().findFirstSameSource(
          ZStackObject::TYPE_FLYEM_TODO_ITEM, source) == NULL) {
      std::vector<ZFlyEmToDoItem*> itemList =
          getDataDocument()->getTodoItem(bodyId);

      for (std::vector<ZFlyEmToDoItem*>::const_iterator iter = itemList.begin();
           iter != itemList.end(); ++iter) {
        ZFlyEmToDoItem *item = *iter;
        item->setRadius(30);
//        item->setColor(255, 255, 0);
        item->setSource(source);
        addObject(item, false);
      }
    }

    endObjectModifiedMode();
    notifyObjectModified(true);
  }
}


void ZFlyEmBody3dDoc::removeBody(uint64_t bodyId)
{
  m_bodySet.remove(bodyId);
  removeBodyFunc(bodyId, true);
}

void ZFlyEmBody3dDoc::removeBodyFunc(uint64_t bodyId, bool removingAnnotation)
{
  ZOUT(LTRACE(), 5) << "Remove body: " << bodyId;

  QString threadId = QString("removeBody(%1)").arg(bodyId);
  if (!m_futureMap.isAlive(threadId)) {
    //TStackObjectList objList = getObjectGroup().findSameSource(
        //  ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId));
    TStackObjectList objList = getObjectGroup().findSameClass(
          ZStackObject::TYPE_SWC,
          ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId));

    beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
    for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
         ++iter) {
      removeObject(*iter, false);
      dumpGarbage(*iter, true);
    }

    if (removingAnnotation) {
      objList = getObjectGroup().findSameSource(
            ZStackObjectSourceFactory::MakeTodoPunctaSource(bodyId));
      for (TStackObjectList::iterator iter = objList.begin();
           iter != objList.end(); ++iter) {
        removeObject(*iter, false);
        dumpGarbage(*iter, true);
      }

      objList = getObjectGroup().findSameSource(
            ZStackObjectSourceFactory::MakeFlyEmTBarSource(bodyId));
      for (TStackObjectList::iterator iter = objList.begin();
           iter != objList.end(); ++iter) {
        removeObject(*iter, false);
        dumpGarbage(*iter, true);
      }

      objList = getObjectGroup().findSameSource(
            ZStackObjectSourceFactory::MakeFlyEmPsdSource(bodyId));
      for (TStackObjectList::iterator iter = objList.begin();
           iter != objList.end(); ++iter) {
        removeObject(*iter, false);
        dumpGarbage(*iter, true);
      }
    }

    endObjectModifiedMode();
    notifyObjectModified(true);
  }
}

ZSwcTree* ZFlyEmBody3dDoc::retrieveBodyModel(uint64_t bodyId, EBodyType bodyType)
{
  ZStackObject *obj = getObjectGroup().findFirstSameSource(
        ZStackObject::TYPE_SWC,
        ZStackObjectSourceFactory::MakeFlyEmBodySource(
          bodyId, GetBodyTypeName(bodyType)));

  ZSwcTree *tree = dynamic_cast<ZSwcTree*>(obj);

  return tree;
}

ZSwcTree* ZFlyEmBody3dDoc::makeBodyModel(uint64_t bodyId)
{
  return makeBodyModel(bodyId, getBodyType());
}

ZSwcTree* ZFlyEmBody3dDoc::makeBodyModel(
    uint64_t bodyId, ZFlyEmBody3dDoc::EBodyType bodyType)
{
  ZSwcTree *tree = NULL;

  if (bodyType == BODY_COARSE) {
    tree = recoverFromGarbage<ZSwcTree>(
          ZStackObjectSourceFactory::MakeFlyEmBodySource(
            bodyId, GetBodyTypeName(BODY_FULL)));
  }

  if (tree == NULL) {
    tree = recoverFromGarbage<ZSwcTree>(
          ZStackObjectSourceFactory::MakeFlyEmBodySource(
            bodyId, GetBodyTypeName(bodyType)));
  }

  if (tree == NULL) {
    if (bodyId > 0) {
      int t = m_objectTime.elapsed();
      if (bodyType == BODY_SKELETON) {
        ZDvidReader reader;
        if (reader.open(getDvidTarget())) {
          tree = reader.readSwc(bodyId);
        }
      } else if (bodyType == BODY_COARSE) {
        ZDvidReader reader;
        if (reader.open(getDvidTarget())) {
          ZObject3dScan obj = reader.readCoarseBody(bodyId);
          if (!obj.isEmpty()) {
            tree = ZSwcFactory::CreateSurfaceSwc(obj);
            tree->translate(-m_dvidInfo.getStartBlockIndex());
            tree->rescale(m_dvidInfo.getBlockSize().getX(),
                          m_dvidInfo.getBlockSize().getY(),
                          m_dvidInfo.getBlockSize().getZ());
            tree->translate(m_dvidInfo.getStartCoordinates() +
                            m_dvidInfo.getBlockSize() / 2);
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
        tree->setTimeStamp(t);
        tree->setSource(
              ZStackObjectSourceFactory::MakeFlyEmBodySource(
                bodyId, GetBodyTypeName(bodyType)));
        tree->setObjectClass(
              ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId));
      }
    }
  }

  return tree;
}

std::string ZFlyEmBody3dDoc::GetBodyTypeName(EBodyType bodyType)
{
  switch (bodyType) {
  case BODY_FULL:
    return "full";
  case BODY_COARSE:
    return "coarse";
  case BODY_SKELETON:
    return "skeleton";
  }

  return "";
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

void ZFlyEmBody3dDoc::forceBodyUpdate()
{
  QSet<uint64_t> bodySet = m_bodySet;
  dumpAllBody(false);
  addBodyChangeEvent(bodySet.begin(), bodySet.end());
}

template <typename InputIterator>
void ZFlyEmBody3dDoc::dumpGarbage(
    const InputIterator &first, const InputIterator &last, bool recycable)
{
  QMutexLocker locker(&m_garbageMutex);


  for (InputIterator iter = first; iter != last; ++iter) {
    m_garbageMap[*iter].setTimeStamp(m_objectTime.elapsed());
    m_garbageMap[*iter].setRecycable(recycable);
  }

  m_garbageJustDumped = true;
}

void ZFlyEmBody3dDoc::dumpGarbage(ZStackObject *obj, bool recycable)
{
  QMutexLocker locker(&m_garbageMutex);

//  m_garbageList.append(obj);
//  m_garbageMap[obj] = m_objectTime.elapsed();
  m_garbageMap[obj].setTimeStamp(m_objectTime.elapsed());
  m_garbageMap[obj].setRecycable(recycable);

  m_garbageJustDumped = true;
}

void ZFlyEmBody3dDoc::dumpAllBody(bool recycable)
{
  cancelEventThread();

  ZOUT(LTRACE(), 5) << "Dump puncta";
  QList<ZPunctum*> punctumList = getObjectList<ZPunctum>();
  for (QList<ZPunctum*>::const_iterator iter = punctumList.begin();
       iter != punctumList.end(); ++iter) {
    ZPunctum *p = *iter;
    removeObject(p, false);
    dumpGarbage(p, false);
  }

  ZOUT(LTRACE(), 5) << "Dump todo list";
  QList<ZFlyEmToDoItem*> todoList = getObjectList<ZFlyEmToDoItem>();
  for (QList<ZFlyEmToDoItem*>::const_iterator iter = todoList.begin();
       iter != todoList.end(); ++iter) {
    ZFlyEmToDoItem *p = *iter;
    removeObject(p, false);
    dumpGarbage(p, false);
  }


  ZOUT(LTRACE(), 5) << "Dump swc";
  QList<ZSwcTree*> treeList = getSwcList();
  for (QList<ZSwcTree*>::const_iterator iter = treeList.begin();
       iter != treeList.end(); ++iter) {
    ZSwcTree *tree = *iter;
    removeObject(tree, false);
    dumpGarbage(tree, recycable);
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
        dumpGarbage(tree, false);
      }
    }
    removeEmptySwcTree(false);
  }
}


//////////////////////////////////////////////
ZFlyEmBody3dDoc::ObjectStatus::ObjectStatus(int timeStamp)
{
  init(timeStamp);
}

void ZFlyEmBody3dDoc::ObjectStatus::init(int timeStatus)
{
  m_recycable = true;
  m_timeStamp = timeStatus;
  m_resLevel = 0;
}

void ZFlyEmBody3dDoc::ObjectStatus::setRecycable(bool on)
{
  m_recycable = on;
}

bool ZFlyEmBody3dDoc::ObjectStatus::isRecycable() const
{
  return m_recycable;
}

void ZFlyEmBody3dDoc::ObjectStatus::setTimeStamp(int t)
{
  m_timeStamp = t;
}

int ZFlyEmBody3dDoc::ObjectStatus::getTimeStamp() const
{
  return m_timeStamp;
}

void ZFlyEmBody3dDoc::ObjectStatus::setResLevel(int level)
{
  m_resLevel = level;
}

int ZFlyEmBody3dDoc::ObjectStatus::getResLevel() const
{
  return m_resLevel;
}



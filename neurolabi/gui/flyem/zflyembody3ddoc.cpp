//#define _NEUTU_USE_REF_KEY_
#include "zflyembody3ddoc.h"

#include <archive.h>
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
#include "z3dwindow.h"
#include "zstackdocdatabuffer.h"
#include "dialogs/zflyembodycomparisondialog.h"
#include "zswcutil.h"
#include "zflyembody3ddockeyprocessor.h"
#include "zflyembody3ddoccommand.h"
#include "zmesh.h"
#include "zglobal.h"
#include "zflyemmisc.h"
#include "zstroke2d.h"
#include "zobject3d.h"
#include "zmeshfactory.h"
#include "zstackdocaccessor.h"
#include "zstackwatershedcontainer.h"
#include "zstackobjectaccessor.h"
#include "flyem/zflyembodysplitter.h"
#include "zactionlibrary.h"
#include "dvid/zdvidgrayslice.h"
#include "flyem/zflyemtodoitem.h"

const int ZFlyEmBody3dDoc::OBJECT_GARBAGE_LIFE = 30000;
const int ZFlyEmBody3dDoc::OBJECT_ACTIVE_LIFE = 15000;
const int ZFlyEmBody3dDoc::MAX_RES_LEVEL = 5;
const char* ZFlyEmBody3dDoc::THREAD_SPLIT_KEY = "split";

ZFlyEmBody3dDoc::ZFlyEmBody3dDoc(QObject *parent) :
  ZStackDoc(parent)
{
  initArbGraySlice();

  m_timer = new QTimer(this);
  m_timer->setInterval(200);
  m_timer->start();

  m_garbageTimer = new QTimer(this);
  m_garbageTimer->setInterval(60000);
  m_garbageTimer->start();

  m_objectTime.start();

  enableAutoSaving(false);

  connectSignalSlot();
//  disconnectSwcNodeModelUpdate();

  m_splitter = new ZFlyEmBodySplitter(this);
  ZWidgetMessage::ConnectMessagePipe(m_splitter, this);
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

void ZFlyEmBody3dDoc::waitForAllEvent()
{
  processEvent();
  m_futureMap.waitForFinished();
}

bool ZFlyEmBody3dDoc::updating() const
{
  QString threadId = QString("processEvent()");
  return m_futureMap.isAlive(threadId);
}

void ZFlyEmBody3dDoc::connectSignalSlot()
{
  connect(m_timer, SIGNAL(timeout()), this, SLOT(processEvent()));
  connect(m_garbageTimer, SIGNAL(timeout()), this, SLOT(clearGarbage()));
}

void ZFlyEmBody3dDoc::updateBodyFunc()
{
}

void ZFlyEmBody3dDoc::enableNodeSeeding(bool on)
{
  m_nodeSeeding = on;
}

void ZFlyEmBody3dDoc::enableBodySelectionSync(bool on)
{
  m_syncyingBodySelection = on;
}

void ZFlyEmBody3dDoc::enableGarbageLifetimeLimit(bool on)
{
  m_limitGarbageLifetime = on;
}

bool ZFlyEmBody3dDoc::garbageLifetimeLimitEnabled() const
{
  return m_limitGarbageLifetime;
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
          iter.value().setTimeStamp(0);
        } else if (!m_limitGarbageLifetime || (dt < OBJECT_ACTIVE_LIFE)) {
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
    ZOUT(LTRACE(), 5) << obj << "recovered." << obj->getSource();
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

void ZFlyEmBody3dDoc::clearGarbage(bool force)
{
  if (!m_limitGarbageLifetime && !force) {
    return;
  }

  QMutexLocker locker(&m_garbageMutex);

  ZOUT(LTRACE(), 5) << "Clear garbage objects ..." << m_garbageMap.size();

  int count = 0;
  int currentTime = m_objectTime.elapsed();
  QMutableMapIterator<ZStackObject*, ObjectStatus> iter(m_garbageMap);
   while (iter.hasNext()) {
     iter.next();
     int t = iter.value().getTimeStamp();
     int dt = currentTime - t;
     if (!force && dt < 0) {
       iter.value().setTimeStamp(0);
     } else if (force || dt > OBJECT_GARBAGE_LIFE){
       ZStackObject *obj = iter.key();
       if (obj->getType() == ZStackObject::TYPE_SWC) {
         ZOUT(LTRACE(), 5) << "Deleting SWC object: " << obj << obj->getSource();
       }

       if (obj != iter.key()) {
         LTRACE() << "Deleting failed";
       }

       delete iter.key();
       iter.remove();
       ++count;

       if (m_garbageMap.contains(obj)) {
         LTRACE() << "Deleting failed";
       }
     }
   }

   ZOUT(LTRACE(), 5) << count << "removed;" << m_garbageMap.size() << "left";

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

const ZFlyEmBody3dDoc::BodyEvent::TUpdateFlag
ZFlyEmBody3dDoc::BodyEvent::UPDATE_SEGMENTATION = 16;

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
  case ACTION_CACHE:
    std::cout << "Cache: ";
    break;
  }

  std::cout << getBodyId() << std::endl;
  std::cout << "  resolution: " << getResLevel() << std::endl;
  std::cout << "  flag: " << getUpdateFlag() << std::endl;
}

void ZFlyEmBody3dDoc::BodyEvent::mergeEvent(
    const BodyEvent &event, neutube::EBiDirection direction)
{
  if (getBodyId() != event.getBodyId())  {
    return;
  }

  switch (direction) {
  case neutube::DIRECTION_FORWARD: //event comes first
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
  case neutube::DIRECTION_BACKWARD:
  {
    BodyEvent tmpEvent = event;
    tmpEvent.mergeEvent(*this, neutube::DIRECTION_FORWARD);
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
    connect(getDataDocument(), SIGNAL(objectModified(ZStackObjectInfoSet)),
            this, SLOT(processObjectModifiedFromDataDoc(ZStackObjectInfoSet)));
  }
}

ZFlyEmProofDoc* ZFlyEmBody3dDoc::getDataDocument() const
{
  return qobject_cast<ZFlyEmProofDoc*>(m_dataDoc.get());
}

void ZFlyEmBody3dDoc::initArbGraySlice()
{
  ZDvidGraySlice *slice = new ZDvidGraySlice();
  slice->setSliceAxis(neutube::A_AXIS);
  slice->setTarget(ZStackObject::TARGET_3D_CANVAS);
  slice->setSource(
        ZStackObjectSourceFactory::MakeDvidGraySliceSource(neutube::A_AXIS));
  addObject(slice);
}

ZDvidGraySlice* ZFlyEmBody3dDoc::getArbGraySlice() const
{
  ZDvidGraySlice *slice = getObject<ZDvidGraySlice>(
        ZStackObjectSourceFactory::MakeDvidGraySliceSource(neutube::A_AXIS));

  return slice;
}

/*
void ZFlyEmBody3dDoc::updateArbGraySlice(const ZArbSliceViewParam &viewParam)
{
  ZDvidGraySlice *slice = getObject<ZDvidGraySlice>(
        ZStackObjectSourceFactory::MakeDvidGraySliceSource(neutube::A_AXIS));
  if (slice != NULL) {
    if (slice->update(viewParam)) {
      bufferObjectModified(slice, ZStackObjectInfo::STATE_MODIFIED);
      processObjectModified();
    }
  }
}
*/

void ZFlyEmBody3dDoc::setArbGraySliceVisible(bool v)
{
  ZDvidGraySlice *slice = getObject<ZDvidGraySlice>(
        ZStackObjectSourceFactory::MakeDvidGraySliceSource(neutube::A_AXIS));
  if (slice != NULL) {
    if (v != slice->isVisible()) {
      slice->setVisible(v);
      bufferObjectVisibilityChanged(slice);
    }
  }
}

void ZFlyEmBody3dDoc::hideArbGrayslice()
{
  setArbGraySliceVisible(false);
}

int ZFlyEmBody3dDoc::getMinResLevel() const
{
  int resLevel = 0;
  switch (getBodyType()) {
  case flyem::BODY_COARSE:
    resLevel = MAX_RES_LEVEL;
    break;
  default:
    break;
  }

  return resLevel;
}

int ZFlyEmBody3dDoc::getMaxResLevel() const
{
  return m_maxResLevel;
}

void ZFlyEmBody3dDoc::removeDiffBody()
{
  QList<ZSwcTree*> treeList = getSwcList();
  for (QList<ZSwcTree*>::iterator iter = treeList.begin();
       iter != treeList.end(); ++iter) {
    ZSwcTree *tree = *iter;
    if (ZStackObjectSourceFactory::IsBodyDiffSource(tree->getSource())) {
      getDataBuffer()->addUpdate(tree, ZStackDocObjectUpdate::ACTION_KILL);
    }
  }
  getDataBuffer()->deliver();
}

ZStackObject* ZFlyEmBody3dDoc::takeObjectFromCache(
    ZStackObject::EType type, const std::string &source)
{
  TStackObjectList objList = m_objCache.takeSameSource(type, source);
  if (!objList.isEmpty()) {
    return objList.back();
  }

  return NULL;
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
    if (event.updating(BodyEvent::UPDATE_SEGMENTATION)) {
      updateSegmentation();
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

#if 0
void ZFlyEmProofDoc::setTodoVisible(
    ZFlyEmToDoItem::EToDoAction action, bool visible)
{
  QList<ZFlyEmToDoItem*> objList = getObjectList<ZFlyEmToDoItem>();
  for (QList<ZFlyEmToDoItem*>::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZFlyEmToDoItem *item = *iter;
    if (item->getAction() == action) {
      item->setVisible(visible);
    }
  }

  emit todoVisibleChanged();
}
#endif

void ZFlyEmBody3dDoc::setNormalTodoVisible(bool visible)
{
  QList<ZFlyEmToDoItem*> objList = getObjectList<ZFlyEmToDoItem>();
  for (QList<ZFlyEmToDoItem*>::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZFlyEmToDoItem *item = *iter;
    if (item->getAction() == neutube::TO_DO) {
      item->setVisible(visible);
    }
  }

  emit todoVisibleChanged();
}

void ZFlyEmBody3dDoc::setTodoItemAction(neutube::EToDoAction action)
{
  const TStackObjectSet& objSet = getObjectGroup().getSelectedSet(
        ZStackObject::TYPE_FLYEM_TODO_ITEM);
  for (TStackObjectSet::const_iterator iter = objSet.begin();
       iter != objSet.end(); ++iter) {
    ZFlyEmToDoItem *item = dynamic_cast<ZFlyEmToDoItem*>(*iter);
    if (item != NULL) {
      if (action != item->getAction()) {
        item->setAction(action);
        m_mainDvidWriter.writeToDoItem(*item);
        bufferObjectModified(item);
      }
    }
  }

  processObjectModified();
}

void ZFlyEmBody3dDoc::setSelectedTodoItemChecked(bool on)
{
//  bool changed = false;

  const TStackObjectSet& objSet = getObjectGroup().getSelectedSet(
        ZStackObject::TYPE_FLYEM_TODO_ITEM);
  for (TStackObjectSet::const_iterator iter = objSet.begin();
       iter != objSet.end(); ++iter) {
    ZFlyEmToDoItem *item = dynamic_cast<ZFlyEmToDoItem*>(*iter);
    if (item != NULL) {
      if (item->isChecked() != on) {
        item->setChecked(on);
        m_mainDvidWriter.writeToDoItem(*item);
//        changed = true;
        bufferObjectModified(item);
      }
    }
  }
  processObjectModified();

//  if (changed) {
//    notifyTodoModified();
//  }
}

void ZFlyEmBody3dDoc::checkSelectedTodoItem()
{
  setSelectedTodoItemChecked(true);
}

void ZFlyEmBody3dDoc::uncheckSelectedTodoItem()
{
  setSelectedTodoItemChecked(false);
}

void ZFlyEmBody3dDoc::setSeedType(int type)
{
  zswc::SetType(getSelectedSwcNodeSet(), type);
  notifySwcModified();
}

void ZFlyEmBody3dDoc::setBodyModelSelected(const QSet<uint64_t> &bodySet)
{
  m_selectedBodySet = bodySet;
}

void ZFlyEmBody3dDoc::setBodyModelSelected(const QSet<uint64_t> &select,
                                           const QSet<uint64_t> &deselect)
{
  m_selectedBodySet -= deselect;
  m_selectedBodySet += select;
}

bool ZFlyEmBody3dDoc::hasTodoItemSelected() const
{
  return !getObjectGroup().getSelectedSet(
        ZStackObject::TYPE_FLYEM_TODO_ITEM).empty();
}

void ZFlyEmBody3dDoc::deleteSplitSeed()
{
  executeRemoveObjectCommand(ZStackObjectRole::ROLE_SEED);
}

void ZFlyEmBody3dDoc::deleteSelectedSplitSeed()
{
  executeRemoveSelectedObjectCommand(ZStackObjectRole::ROLE_SEED);
}

void ZFlyEmBody3dDoc::processObjectModifiedFromDataDoc(
    const ZStackObjectInfoSet &objInfo)
{
  if (objInfo.contains(ZStackObjectRole::ROLE_SEGMENTATION)) {
    //Update the segmentation only when one body is selected
    //Todo: association between bodies and segmentations
    if (m_bodySet.size() == 1) {
      uint64_t bodyId = *m_bodySet.begin();
      addEvent(BodyEvent::ACTION_UPDATE, bodyId, BodyEvent::UPDATE_SEGMENTATION);
    }
  }
}

void ZFlyEmBody3dDoc::saveSplitTask()
{
  if (m_bodySet.size() == 1) {
    uint64_t bodyId = *m_bodySet.begin();
    if (bodyId > 0) {
      ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriterFromUrl(
            GET_FLYEM_CONFIG.getTaskServer());
      if (writer != NULL) {
        ZJsonArray seedJson = ZFlyEmMisc::GetSeedJson(this);

        ZDvidUrl dvidUrl(getDvidTarget());
        QString taskKey = dvidUrl.getSplitTaskKey(bodyId).c_str();
        if (seedJson.isEmpty()) {
          if (writer->getDvidReader().hasSplitTask(taskKey)) {
            writer->deleteSplitTask(taskKey);
            std::cout << "Split task deleted: " << taskKey.toStdString() << std::endl;
          }
        } else {
          ZJsonArray roiJson;
          ZJsonObject task = ZFlyEmMisc::MakeSplitTask(
                getDvidTarget(), bodyId, seedJson, roiJson);

//          std::string bodyUrl = dvidUrl.getSparsevolUrl(bodyId);
//          task.setEntry("signal", bodyUrl);

//          task.setEntry("seeds", seedJson);

          std::string location = writer->writeServiceTask("split", task);

          //Save the entry point
          ZJsonObject taskJson;
          taskJson.setEntry(neutube::Json::REF_KEY, location);
          taskJson.setEntry("user", neutube::GetCurrentUserName());
          writer->writeSplitTask(taskKey, taskJson);

          std::cout << "Split task saved @" << taskKey.toStdString() << std::endl;
        }
      }
    }
  }
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

QMap<uint64_t, ZFlyEmBody3dDoc::BodyEvent> ZFlyEmBody3dDoc::makeEventMapUnsync(
    QSet<uint64_t> &bodySet)
{
//  QSet<uint64_t> bodySet = m_bodySet;
  QMap<uint64_t, BodyEvent> actionMap;
  for (QQueue<BodyEvent>::const_iterator iter = m_eventQueue.begin();
       iter != m_eventQueue.end(); ++iter) {
    const BodyEvent &event = *iter;
    uint64_t bodyId = event.getBodyId();
    if (actionMap.contains(bodyId)) {
      actionMap[bodyId].mergeEvent(event, neutube::DIRECTION_BACKWARD);
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


QMap<uint64_t, ZFlyEmBody3dDoc::BodyEvent> ZFlyEmBody3dDoc::makeEventMap(
    bool synced, QSet<uint64_t> &bodySet)
{
  if (synced) {
    std::cout << "Locking process event" << std::endl;
    QMutexLocker locker(&m_eventQueueMutex);

    std::cout << "Making event map" << std::endl;
    return makeEventMapUnsync(bodySet);
  }

  return makeEventMapUnsync(bodySet);
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

  //Process removing events first
  for (QMap<uint64_t, BodyEvent>::const_iterator iter = actionMap.begin();
       iter != actionMap.end(); ++iter) {
    const BodyEvent &event = iter.value();
    if (event.getAction() == BodyEvent::ACTION_REMOVE) {
      processEventFunc(event);
    }
    if (m_quitting) {
      break;
    }
  }

  //Process other events
  for (QMap<uint64_t, BodyEvent>::const_iterator iter = actionMap.begin();
       iter != actionMap.end(); ++iter) {
    const BodyEvent &event = iter.value();
    if (event.getAction() != BodyEvent::ACTION_REMOVE) {
      processEventFunc(event);
    }
    if (m_quitting) {
      break;
    }
  }


//  emit messageGenerated(ZWidgetMessage("3D Body view updated."));
  std::cout << "====Processing done====" << std::endl;
}

ZStackObject::EType ZFlyEmBody3dDoc::getBodyObjectType() const
{
  if (getBodyType() == flyem::BODY_MESH) {
    return ZStackObject::TYPE_MESH;
  }

  return ZStackObject::TYPE_SWC;
}

uint64_t ZFlyEmBody3dDoc::getSingleBody() const
{
  QMutexLocker locker(&m_BodySetMutex);

  uint64_t bodyId = 0;
  if (m_bodySet.size() == 1) {
    bodyId = *(m_bodySet.begin());
  }

  return bodyId;
}

void ZFlyEmBody3dDoc::activateSplit(uint64_t bodyId, flyem::EBodyLabelType type)
{
  if (!isSplitActivated()) {
    uint64_t parentId = bodyId;
    if (type == flyem::LABEL_SUPERVOXEL) {
      parentId = m_workDvidReader.readParentBodyId(bodyId);
    }

    if (getDataDocument()->checkOutBody(parentId, flyem::BODY_SPLIT_ONLINE)) {
      m_splitter->setBody(bodyId, type);
      emit interactionStateChanged();
    } else {
      notifyWindowMessageUpdated("Failed to lock the body for split.");
    }
  }
}

void ZFlyEmBody3dDoc::activateSplitForSelected()
{
  TStackObjectSet objSet = getSelected(ZStackObject::TYPE_MESH);
  if (objSet.size() == 1) {
    ZStackObject *obj = *(objSet.begin());
    activateSplit(obj->getLabel(), getBodyLabelType());
  }
}

void ZFlyEmBody3dDoc::deactivateSplit()
{
  if (m_splitter->getBodyId() > 0) {
    waitForSplitToBeDone();

    uint64_t parentId = m_splitter->getBodyId();

    if (m_splitter->getLabelType() == flyem::LABEL_SUPERVOXEL) {
      parentId = m_workDvidReader.readParentBodyId(m_splitter->getBodyId());
    }
    getDataDocument()->checkInBodyWithMessage(
          parentId, flyem::BODY_SPLIT_ONLINE);

    m_splitter->setBodyId(0);
    emit interactionStateChanged();
  }
}

void ZFlyEmBody3dDoc::makeAction(ZActionFactory::EAction item)
{
  if (!m_actionLibrary->contains(item)) {
    QAction *action = m_actionLibrary->getAction(item);
    if (action != NULL) {
      switch (item) {
      case ZActionFactory::ACTION_START_SPLIT:
        connect(action, SIGNAL(triggered()),
                this, SLOT(activateSplitForSelected()));
        break;
      case ZActionFactory::ACTION_COMMIT_SPLIT:
        connect(action, SIGNAL(triggered()),
                this, SLOT(commitSplitResult()));
        break;
      default:
        break;
      }
    }

    ZStackDoc::makeAction(item);
  }
}

bool ZFlyEmBody3dDoc::isSplitActivated() const
{
  return m_splitter->getBodyId() > 0;
}

bool ZFlyEmBody3dDoc::isSplitFinished() const
{
  return m_splitter->getState() == ZFlyEmBodySplitter::STATE_FULL_SPLIT;
}

bool ZFlyEmBody3dDoc::protectBody(uint64_t bodyId)
{
  if (bodyId > 0) {
    QMutexLocker locker(&m_eventQueueMutex);
    QMap<uint64_t, BodyEvent> actionMap = makeEventMap(false, m_bodySet);
    if (actionMap.contains(bodyId)) {
      if (actionMap[bodyId].getAction() == BodyEvent::ACTION_REMOVE) {
        //Cannot protected a body to be removed
        return false;
      }
    }

    m_protectedBodySet.insert(bodyId);
  }

  return true;
}

void ZFlyEmBody3dDoc::releaseBody(uint64_t bodyId)
{
  QMutexLocker locker(&m_eventQueueMutex);

  m_protectedBodySet.remove(bodyId);

  emit interactionStateChanged();
}

bool ZFlyEmBody3dDoc::isBodyProtected(uint64_t bodyId) const
{
  QMutexLocker locker(&m_eventQueueMutex);

  return m_protectedBodySet.contains(bodyId);
}


uint64_t ZFlyEmBody3dDoc::protectBodyForSplit()
{
  uint64_t bodyId = m_splitter->getBodyId();

  QMutexLocker locker(&m_BodySetMutex);
  if (bodyId == 0) {
    if (m_bodySet.size() == 1) {
      bodyId = *(m_bodySet.begin());
    }
  }

  if (!protectBody(bodyId)) {
    bodyId = 0;
  }

  return bodyId;
}

ZDvidSparseStack* ZFlyEmBody3dDoc::loadDvidSparseStack(
    uint64_t bodyId, flyem::EBodyLabelType type)
{
  if (bodyId == 0) {
    return NULL;
  }

  ZDvidSparseStack *body = getDataDocument()->getCachedBodyForSplit(bodyId);

  if (body != NULL) {
    if (body->getLabel() != bodyId || body->getLabelType() != type) {
      body = NULL;
    }
  }

  if (body == NULL) {
#ifdef _DEBUG_
    std::cout << "Reading body " << bodyId << " ..." << std::endl;
#endif
    body = getBodyReader().readDvidSparseStackAsync(bodyId);
    body->setSource(ZStackObjectSourceFactory::MakeSplitObjectSource());
    getDataDocument()->addObject(body, true);
  } else {
#ifdef _DEBUG_
    std::cout << "Body obtained from cache." << std::endl;
#endif
  }

  if (body != NULL) {
    body->setLabelType(type);
  }

  return body;
}

#if 0
ZDvidSparseStack* ZFlyEmBody3dDoc::loadDvidSparseStack()
{
//  return getDataDocument()->getDvidSparseStack();

  ZDvidSparseStack *stack = NULL;

  if (m_bodySet.size() == 1) {
    uint64_t bodyId = *(m_bodySet.begin());
    stack = loadDvidSparseStack(bodyId, );
  }

  return stack;
}
#endif

ZDvidSparseStack* ZFlyEmBody3dDoc::loadDvidSparseStackForSplit()
{
  ZDvidSparseStack *stack = NULL;

  stack = loadDvidSparseStack(
        m_splitter->getBodyId(), m_splitter->getLabelType());
  if (stack != NULL) {
    stack->setLabelType(m_splitter->getLabelType());
  }

  return stack;
}

void ZFlyEmBody3dDoc::updateBodyModelSelection()
{
  QList<ZSwcTree*> swcList = getSwcList();
  foreach (ZSwcTree *tree, swcList) {
    if (m_selectedBodySet.contains(tree->getLabel())) {
      if (!tree->isSelected()) {
        getDataBuffer()->addUpdate(tree, ZStackDocObjectUpdate::ACTION_SELECT);
      }
    } else if (tree->isSelected()) {
      getDataBuffer()->addUpdate(tree, ZStackDocObjectUpdate::ACTION_DESELECT);
    }
  }

  QList<ZMesh*> meshList = getMeshList();
  foreach (ZMesh *mesh, meshList) {
    if (m_selectedBodySet.contains(mesh->getLabel())) {
      if (!mesh->isSelected()) {
        getDataBuffer()->addUpdate(mesh, ZStackDocObjectUpdate::ACTION_SELECT);
      }
    } else if (mesh->isSelected()) {
      getDataBuffer()->addUpdate(mesh, ZStackDocObjectUpdate::ACTION_DESELECT);
    }
  }

  getDataBuffer()->deliver();
}

void ZFlyEmBody3dDoc::processEvent()
{
  if (m_syncyingBodySelection) {
    updateBodyModelSelection();
  }

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

bool ZFlyEmBody3dDoc::hasBody(uint64_t bodyId, bool encoded) const
{
  QMutexLocker locker(&m_BodySetMutex);

  if (!encoded) {
    return getUnencodedBodySet().contains(bodyId);
  }

  return m_bodySet.contains(bodyId);
}

void ZFlyEmBody3dDoc::addBody(uint64_t bodyId, const QColor &color)
{
  QMutexLocker locker(&m_BodySetMutex);
  if (!m_bodySet.contains(bodyId)) {
    m_bodySet.insert(bodyId);
    if (getBodyType() == flyem::BODY_SKELETON) {
      addBodyFunc(bodyId, color, -1);
    } else {
      addBodyFunc(bodyId, color, m_maxResLevel);
    }
  }
}

void ZFlyEmBody3dDoc::setBodyType(flyem::EBodyType type)
{
  m_bodyType = type;
  switch (m_bodyType) {
  case flyem::BODY_COARSE:
    setTag(neutube::Document::FLYEM_BODY_3D_COARSE);
    setMaxResLevel(MAX_RES_LEVEL);
    break;
  case flyem::BODY_FULL:
    setTag(neutube::Document::FLYEM_BODY_3D);
    setMaxResLevel(MAX_RES_LEVEL);
    break;
  case flyem::BODY_SKELETON:
    setTag(neutube::Document::FLYEM_SKELETON);
    setMaxResLevel(0);
    break;
  case flyem::BODY_MESH:
    setTag(neutube::Document::FLYEM_MESH);
    setMaxResLevel(MAX_RES_LEVEL);
    break;
  case flyem::BODY_NULL:
    break;
  }
}

void ZFlyEmBody3dDoc::updateBody(
    uint64_t bodyId, const QColor &color)
{
  updateBody(bodyId, color, flyem::BODY_COARSE);
  updateBody(bodyId, color, flyem::BODY_FULL);
  updateBody(bodyId, color, flyem::BODY_SKELETON);
  updateBody(bodyId, color, flyem::BODY_MESH);
}

void ZFlyEmBody3dDoc::updateBody(
    uint64_t bodyId, const QColor &color, flyem::EBodyType type)
{
  beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  if (type != flyem::BODY_MESH) {
    ZSwcTree *tree = getBodyModel(bodyId, 0, type);
    if (tree != NULL) {
      if (tree->getColor() != color) {
        tree->setColor(color);
        processObjectModified(tree);
      }
    }
  } else {
    // A mesh from a tar archive is never needs its color updated, and checking
    // for it is a slow loop that is quadratic in the number of meshes.

    if (!fromTar(bodyId)) {
      ZMesh *mesh = getBodyMesh(bodyId, 0);
      if (mesh != NULL) {
        if (mesh->getColor() != color) {
          mesh->setColor(color);
          mesh->pushObjectColor();
          processObjectModified(mesh);
        }
      }
    }
  }
  endObjectModifiedMode();
  processObjectModified();
}

ZSwcTree* ZFlyEmBody3dDoc::getBodyModel(
    uint64_t bodyId, int zoom, flyem::EBodyType bodyType)
{
  return retrieveBodyModel(bodyId, zoom, bodyType);
}

ZMesh* ZFlyEmBody3dDoc::getBodyMesh(uint64_t bodyId, int zoom)
{
  return retrieveBodyMesh(bodyId, zoom);
}

void ZFlyEmBody3dDoc::addEvent(const BodyEvent &event, QMutex *mutex)
{
  QMutexLocker locker(mutex);

  if (event.getAction() == BodyEvent::ACTION_REMOVE) {
    //When a body is removed, its associated objects will be removed as well.
    //Clear the undo queue to avoid potential crash
    undoStack()->clear();
  }

  m_eventQueue.enqueue(event);
}

void ZFlyEmBody3dDoc::addEvent(BodyEvent::EAction action, uint64_t bodyId,
                               BodyEvent::TUpdateFlag flag, QMutex *mutex)
{
  QMutexLocker locker(mutex);

  BodyEvent event(action, bodyId);
  event.addUpdateFlag(flag);
  if (getDataDocument() != NULL) {
    ZDvidLabelSlice *labelSlice =
        getDataDocument()->getDvidLabelSlice(neutube::Z_AXIS);

    if (labelSlice != NULL) {
      QColor color;

      if (getBodyType() == flyem::BODY_FULL) { //using the original color
        color = labelSlice->getLabelColor(bodyId, neutube::BODY_LABEL_MAPPED);
      } else {
        color = labelSlice->getLabelColor(bodyId, neutube::BODY_LABEL_ORIGINAL);
      }
      color.setAlpha(255);
      event.setBodyColor(color);
    }
  }

#if defined(_NEU3_)
  event.setBodyColor(Qt::white);
#endif

  if (event.getAction() == BodyEvent::ACTION_ADD &&
      getBodyType() != flyem::BODY_SKELETON) {
    event.setResLevel(getMaxResLevel());
  }

  addEvent(event);
//  m_eventQueue.enqueue(event);
}

ZSwcTree* ZFlyEmBody3dDoc::getBodyQuickly(uint64_t bodyId)
{
  ZSwcTree *tree = NULL;

  if (getBodyType() == flyem::BODY_FULL) {
    tree = recoverFullBodyFromGarbage(bodyId, getMaxResLevel());
  }

  if (tree == NULL) {
    tree = makeBodyModel(bodyId, 0, flyem::BODY_COARSE);
  }

  return tree;
}

ZFlyEmBody3dDoc::BodyEvent ZFlyEmBody3dDoc::makeMultresBodyEvent(
    uint64_t bodyId, int resLevel, const QColor &color)
{
  BodyEvent bodyEvent(BodyEvent::ACTION_ADD, bodyId);
  bodyEvent.setBodyColor(color);
  if (resLevel > getDvidTarget().getMaxLabelZoom()) {
    resLevel = getDvidTarget().getMaxLabelZoom();
  }
  bodyEvent.setResLevel(resLevel);
  bodyEvent.addUpdateFlag(BodyEvent::UPDATE_MULTIRES);

  return bodyEvent;
}

void ZFlyEmBody3dDoc::notifyBodyUpdate(uint64_t bodyId, int resLevel)
{
  notifyWindowMessageUpdated(
        QString("Updating body %1 (scale=%2) ...").arg(bodyId).arg(resLevel));
}

void ZFlyEmBody3dDoc::notifyBodyUpdated(uint64_t bodyId, int resLevel)
{
  if (resLevel > 0) {
    notifyWindowMessageUpdated(
          QString("Body %1 updated (scale=%2).").arg(bodyId).arg(resLevel));
  } else {
    notifyWindowMessageUpdated(
          QString("Body %1 updated.").arg(bodyId));
  }
}

void ZFlyEmBody3dDoc::addBodyMeshFunc(
    uint64_t id, const QColor &color, int resLevel)
{
  notifyBodyUpdate(id, resLevel);

  std::map<uint64_t, ZMesh*> meshes;
  makeBodyMeshModels(id, resLevel, meshes);

  if (!meshes.empty()) {
    addSynapse(decode(id));
    //      addTodo(bodyId);
    updateTodo(decode(id));
  }

  for (auto it : meshes) {
//    emit messageGenerated(ZWidgetMessage("3D Body view synced"));

    uint64_t bodyId = it.first;
    ZMesh *mesh = it.second;

    if (mesh != NULL) {
      resLevel = ZStackObjectSourceFactory::ExtractZoomFromFlyEmBodySource(
            mesh->getSource());
    }

    if (resLevel > getMinResLevel()) {
      QMutexLocker locker(&m_eventQueueMutex);
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
        BodyEvent bodyEvent = makeMultresBodyEvent(bodyId, resLevel - 1, color);
        m_eventQueue.enqueue(bodyEvent);
      }
    }

    if (mesh != NULL) {
  #ifdef _DEBUG_
      std::cout << "Adding object: " << dynamic_cast<ZStackObject*>(mesh) << std::endl;
      std::cout << "Color count: " << mesh->colors().size() << std::endl;
      std::cout << "Vertex count: " << mesh->vertices().size() << std::endl;
  #endif
      mesh->setColor(color);
      mesh->pushObjectColor();

      // The findSameClass() function has performance that iis quadratic in the number of meshes,
      // and is unnecessary for meshes from a tar archive.

      bool loaded = fromTar(id);
      if (!loaded) {
        loaded =
          !(getObjectGroup().findSameClass(
              ZStackObject::TYPE_MESH,
              ZStackObjectSourceFactory::MakeFlyEmBodySource(mesh->getLabel())).
            isEmpty());
      }

      updateBodyFunc(bodyId, mesh);

      if (!loaded) {
        // TODO: As of December, 2017, the following is slow due to access of a desktop server,
        // http://zhaot-ws1:9000.  This server should be replaced with a faster one.
        // The problem is most noticeable for the functionality of taskbodyhistory.cpp.
        loadSplitTask(bodyId);
      }

      // If the argument ID loads an archive, then makeBodyMeshModels() can create
      // multiple meshes whose IDs need to be recorded, to make operations like
      // selection work correctly.

      m_bodySet.insert(mesh->getLabel());
    }
  }

  notifyBodyUpdated(id, resLevel);

  if (encodesTar(id)) {

    // Meshes loaded from an archive are ready at this point, so emit a signal, which
    // can be used by code that needs to know the IDs of the loaded meshes (instead of
    // the ID of the archive).

    emit bodyMeshesAdded();
  }
}

void ZFlyEmBody3dDoc::addBodyFunc(
    uint64_t bodyId, const QColor &color, int resLevel)
{
  if (getBodyType() == flyem::BODY_MESH) {
    addBodyMeshFunc(bodyId, color, resLevel);
  } else {
    removeDiffBody();

    bool loaded =
        !(getObjectGroup().findSameClass(
            ZStackObject::TYPE_SWC,
            ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId)).
          isEmpty());

    ZSwcTree *tree = NULL;
    if (resLevel == getMaxResLevel() && resLevel > 0) {
      notifyBodyUpdate(bodyId, getMaxResLevel());
      tree = getBodyQuickly(bodyId);
      notifyBodyUpdated(bodyId, getMaxResLevel());
    } else {
      notifyBodyUpdate(bodyId, resLevel);
      tree = makeBodyModel(bodyId, resLevel, getBodyType());
      notifyBodyUpdated(bodyId, resLevel);
    }

    if (tree != NULL) {
      if (ZStackObjectSourceFactory::ExtractBodyTypeFromFlyEmBodySource(
            tree->getSource()) == flyem::BODY_FULL) {
        resLevel = ZStackObjectSourceFactory::ExtractZoomFromFlyEmBodySource(
              tree->getSource());
      }
    }

    if (resLevel > getMinResLevel()) {
      QMutexLocker locker(&m_eventQueueMutex);
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
        BodyEvent bodyEvent = makeMultresBodyEvent(bodyId, resLevel - 1, color);
        m_eventQueue.enqueue(bodyEvent);
      }
    }

    if (tree != NULL) {
      tree->setStructrualMode(ZSwcTree::STRUCT_POINT_CLOUD);
      if (m_nodeSeeding) {
        tree->setType(0);
      }

#ifdef _DEBUG_
      std::cout << "Adding object: " << dynamic_cast<ZStackObject*>(tree) << std::endl;
#endif
      tree->setColor(color);

      updateBodyFunc(bodyId, tree);

      if (!loaded) {
        addSynapse(bodyId);
        //      addTodo(bodyId);
        updateTodo(bodyId);
      }
    }
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

void ZFlyEmBody3dDoc::cacheObject(ZStackObject *obj)
{
  m_objCache.add(obj, true);
  if (m_objCache.size() > m_objCacheCapacity) {
    m_objCache.removeFirstObject(true);
  }
}

void ZFlyEmBody3dDoc::showSynapse(bool on)
{
  m_showingSynapse = on;
  addSynapse(on);
}

bool ZFlyEmBody3dDoc::showingSynapse() const
{
  return m_showingSynapse;
}

void ZFlyEmBody3dDoc::showTodo(bool on)
{
  m_showingTodo = on;
  addTodo(on);
}

bool ZFlyEmBody3dDoc::showingTodo() const
{
  return m_showingTodo;
}

bool ZFlyEmBody3dDoc::synapseLoaded(uint64_t bodyId) const
{
  return getObjectGroup().findFirstSameSource(
        ZStackObject::TYPE_PUNCTUM,
        ZStackObjectSourceFactory::MakeFlyEmTBarSource(bodyId)) != NULL;
}

void ZFlyEmBody3dDoc::addSynapse(
    std::vector<ZPunctum*> &puncta,
    uint64_t bodyId, const std::string &source, double radius, const QColor &color)
{
  for (std::vector<ZPunctum*>::const_iterator iter = puncta.begin();
       iter != puncta.end(); ++iter) {
    ZPunctum *punctum = *iter;
    punctum->setRadius(radius);
    punctum->setColor(color);
    punctum->setSource(source);
    if (punctum->name().isEmpty()) {
      punctum->setName(QString("%1").arg(bodyId));
    }
    getDataBuffer()->addUpdate(punctum, ZStackDocObjectUpdate::ACTION_ADD_NONUNIQUE);
  }
}

void ZFlyEmBody3dDoc::addSynapse(uint64_t bodyId)
{
  if (m_showingSynapse && !synapseLoaded(bodyId)) {
      std::pair<std::vector<ZPunctum*>, std::vector<ZPunctum*> > synapse =
          getDataDocument()->getSynapse(bodyId);
      addSynapse(
            synapse.first, bodyId,
            ZStackObjectSourceFactory::MakeFlyEmTBarSource(bodyId),
            30, QColor(255, 255, 0));
      addSynapse(
            synapse.second, bodyId,
            ZStackObjectSourceFactory::MakeFlyEmPsdSource(bodyId),
            30, QColor(128, 128, 128));

      getDataBuffer()->deliver();
      //      endObjectModifiedMode();
      //      notifyObjectModified();
  }
}

void ZFlyEmBody3dDoc::makeKeyProcessor()
{
  m_keyProcessor = new ZFlyEmBody3dDocKeyProcessor(this);
}

void ZFlyEmBody3dDoc::updateTodo(uint64_t bodyId)
{
  if (m_showingTodo) {
    ZOUT(LTRACE(), 5) << "Update todo";

    std::string source = ZStackObjectSourceFactory::MakeTodoPunctaSource(bodyId);

    TStackObjectList objList = getObjectGroup().findSameSource(source);
    for (TStackObjectList::iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      getDataBuffer()->addUpdate(*iter, ZStackDocObjectUpdate::ACTION_KILL);
    }

    if (hasBody(bodyId, false)) {
      std::vector<ZFlyEmToDoItem*> itemList =
          getDataDocument()->getTodoItem(bodyId);

      for (std::vector<ZFlyEmToDoItem*>::const_iterator iter = itemList.begin();
           iter != itemList.end(); ++iter) {
        ZFlyEmToDoItem *item = *iter;
        item->setRadius(30);
        item->setSource(source);
        getDataBuffer()->addUpdate(
              item, ZStackDocObjectUpdate::ACTION_ADD_NONUNIQUE);
      }
    }

    getDataBuffer()->deliver();
  }
}

void ZFlyEmBody3dDoc::addTodo(uint64_t bodyId)
{
  if (m_showingTodo) {
    ZOUT(LTRACE(), 5) << "Add todo items";

    std::string source = ZStackObjectSourceFactory::MakeTodoPunctaSource(bodyId);
    if (getObjectGroup().findFirstSameSource(
          ZStackObject::TYPE_FLYEM_TODO_ITEM, source) == NULL) {
      std::vector<ZFlyEmToDoItem*> itemList =
          getDataDocument()->getTodoItem(bodyId);

      for (std::vector<ZFlyEmToDoItem*>::const_iterator iter = itemList.begin();
           iter != itemList.end(); ++iter) {
        ZFlyEmToDoItem *item = *iter;
        item->setRadius(30);
        item->setSource(source);
        getDataBuffer()->addUpdate(
              item, ZStackDocObjectUpdate::ACTION_ADD_NONUNIQUE);
      }
    }
    getDataBuffer()->deliver();
  }
}

void ZFlyEmBody3dDoc::updateSegmentation()
{
  if (getBodyType() == flyem::BODY_MESH) {
    ZOUT(LTRACE(), 5) << "Update segmentation";
    QList<ZStackObject*> oldObjList =
        getObjectList(ZStackObjectRole::ROLE_TMP_RESULT);
    getDataBuffer()->addUpdate(oldObjList, ZStackDocObjectUpdate::ACTION_KILL);


    QMutexLocker locker(getDataDocument()->getObjectGroup().getMutex());
    QList<ZObject3dScan*> objList =
        getDataDocument()->getObjectGroup().getObjectListUnsync<ZObject3dScan>();
    foreach(ZObject3dScan *obj, objList) {
      if (obj->hasRole(ZStackObjectRole::ROLE_SEGMENTATION)) {
        ZMesh *mesh = ZMeshFactory::MakeMesh(*obj);
        if (mesh != NULL) {
          mesh->setColor(obj->getColor());
          mesh->pushObjectColor();
          mesh->setVisible(obj->isVisible());
          mesh->setSelectable(false);
          mesh->addRole(ZStackObjectRole::ROLE_TMP_RESULT);
          mesh->setSource(
                ZStackObjectSourceFactory::MakeSplitResultSource(obj->getLabel()));
          getDataBuffer()->addUpdate(
                mesh, ZStackDocObjectUpdate::ACTION_ADD_NONUNIQUE);
        }
      }
    }
    getDataBuffer()->deliver();
  }
}

void ZFlyEmBody3dDoc::loadSplitTask(uint64_t bodyId)
{
  if (!m_splitTaskLoadingEnabled) {
    return;
  }

  QList<ZStackObject*> seedList =
      ZFlyEmMisc::LoadSplitTask(getDvidTarget(), bodyId);

  if (!seedList.isEmpty()) {
    getDataBuffer()->addUpdate(
          seedList, ZStackDocObjectUpdate::ACTION_ADD_NONUNIQUE);
    getDataBuffer()->deliver();

//    ZStackDocAccessor::RemoveObject(
//          getDataDocument(), ZStackObjectRole::ROLE_SEED, false);
    //A dangerous way to share objects. Need object clone or shared pointer.
//    ZStackDocAccessor::AddObject(getDataDocument(), seedList);
  }
}

void ZFlyEmBody3dDoc::removeSplitTask(uint64_t bodyId)
{
  ZFlyEmMisc::RemoveSplitTask(getDvidTarget(), bodyId);
}

void ZFlyEmBody3dDoc::enableSplitTaskLoading(bool enable)
{
  m_splitTaskLoadingEnabled = enable;
}

bool ZFlyEmBody3dDoc::splitTaskLoadingEnabled() const
{
  return m_splitTaskLoadingEnabled;
}

ZFlyEmToDoItem ZFlyEmBody3dDoc::makeTodoItem(
    int x, int y, int z, bool checked, uint64_t bodyId)
{
  ZFlyEmToDoItem item;

  ZIntPoint position = getMainDvidReader().readPosition(
        bodyId, ZIntPoint(x, y, z));

  if (position.isValid()) {
    item.setPosition(position);
    item.setKind(ZFlyEmToDoItem::KIND_NOTE);
    item.setUserName(neutube::GetCurrentUserName());
    if (checked) {
      item.setChecked(checked);
    }

    if (!getMainDvidReader().getDvidTarget().isSegmentationSyncable()) {
      //A workaround for syncing to labelmap (temporary solution)
      item.setBodyId(bodyId);
      item.addBodyIdTag();
    }
  }

  return item;
}

ZFlyEmToDoItem ZFlyEmBody3dDoc::readTodoItem(int x, int y, int z) const
{
  ZFlyEmToDoItem item = getMainDvidReader().readToDoItem(x, y, z);

  return item;
}

bool ZFlyEmBody3dDoc::addTodo(const ZFlyEmToDoItem &item, uint64_t bodyId)
{
  bool succ = false;
  if (item.isValid()) {
    m_mainDvidWriter.writeToDoItem(item);
    if (m_mainDvidWriter.isStatusOk()) {
      emit messageGenerated(ZWidgetMessage(
                              QString("Todo added at %1").
                              arg(item.getPosition().toString().c_str())));
      if (m_mainDvidWriter.getDvidTarget().hasSupervoxel()) {
        uint64_t svId = m_mainDvidWriter.getDvidReader().readSupervoxelIdAt(
              item.getPosition());
        emit messageGenerated(ZWidgetMessage(
                               QString("Supervoxel ID: %1").arg(svId)));
      }

      if (m_bodySet.contains(bodyId)) {
        updateTodo(bodyId);
      }
      succ = true;
    }
  }

  return succ;
}

void ZFlyEmBody3dDoc::addTodoSliently(const ZFlyEmToDoItem &item)
{
  if (item.isValid()) {
    m_mainDvidWriter.writeToDoItem(item);
  }
}

void ZFlyEmBody3dDoc::addTodo(const QList<ZFlyEmToDoItem> &itemList)
{
  QSet<uint64_t> bodySet;
  foreach (const ZFlyEmToDoItem &item, itemList) {
    addTodoSliently(item);
    if (item.getBodyId() > 0) {
      bodySet.insert(item.getBodyId());
    }
  }

  foreach (uint64_t bodyId, bodySet) {
    updateTodo(bodyId);
  }
}

void ZFlyEmBody3dDoc::removeTodo(const QList<ZFlyEmToDoItem> &itemList)
{
  QSet<uint64_t> bodySet;
  foreach (const ZFlyEmToDoItem &item, itemList) {
    removeTodoSliently(item);
    if (item.getBodyId() > 0) {
      bodySet.insert(item.getBodyId());
    }
  }

  foreach (uint64_t bodyId, bodySet) {
    updateTodo(bodyId);
  }
}

void ZFlyEmBody3dDoc::removeTodo(ZFlyEmToDoItem &item, uint64_t bodyId)
{
  if (item.isValid()) {
    m_mainDvidWriter.deleteToDoItem(item.getX(), item.getY(), item.getZ());
    updateTodo(bodyId);
  }
}

void ZFlyEmBody3dDoc::removeTodoSliently(const ZFlyEmToDoItem &item)
{
  if (item.isValid()) {
    m_mainDvidWriter.deleteToDoItem(item.getX(), item.getY(), item.getZ());
  }
}

void ZFlyEmBody3dDoc::removeTodo(int x, int y, int z)
{
  m_mainDvidWriter.deleteToDoItem(x, y, z);
}

void ZFlyEmBody3dDoc::addTodo(int x, int y, int z, bool checked, uint64_t bodyId)
{
  ZFlyEmToDoItem item = makeTodoItem(x, y, z, checked, bodyId);
  addTodo(item, bodyId);
}

void ZFlyEmBody3dDoc::addTosplit(int x, int y, int z, bool checked, uint64_t bodyId)
{
  ZFlyEmToDoItem item = makeTodoItem(x, y, z, checked, bodyId);
  item.setAction(neutube::TO_SPLIT);
  addTodo(item, bodyId);
}

void ZFlyEmBody3dDoc::removeBody(uint64_t bodyId)
{
  QMutexLocker locker(&m_BodySetMutex);
  m_bodySet.remove(bodyId);
  removeBodyFunc(bodyId, true);
}

void ZFlyEmBody3dDoc::updateBodyFunc(uint64_t bodyId, ZStackObject *bodyObject)
{
  ZOUT(LTRACE(), 5) << "Update body: " << bodyId;

  QString threadId = QString("updateBody(%1)").arg(bodyId);
  if (!m_futureMap.isAlive(threadId)) {

    // The findSameClass() function has performance that iis quadratic in the number of meshes,
    // and is unnecessary for meshes from a tar archive.

    if (!fromTar(bodyId)) {
        TStackObjectList objList = getObjectGroup().findSameClass(
              bodyObject->getType(),
              ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId));

        for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
             ++iter) {
          getDataBuffer()->addUpdate(*iter, ZStackDocObjectUpdate::ACTION_RECYCLE);
        }
        getDataBuffer()->addUpdate(bodyObject, ZStackDocObjectUpdate::ACTION_ADD_UNIQUE);
    }
    else {

      // The event name is a bit confusing, but "NONUNIQUE" means that ZStackDoc::addObject()
      // won't get into a quadratic loop checking that this body is unique, at test that is
      // not necessary for bodies from tar archives.

      getDataBuffer()->addUpdate(bodyObject, ZStackDocObjectUpdate::ACTION_ADD_NONUNIQUE);
    }
    getDataBuffer()->deliver();
    emit bodyMeshLoaded();
  }

  ZOUT(LTRACE(), 5) << "Body updated: " << bodyId;
}

void ZFlyEmBody3dDoc::executeAddTodoCommand(
    int x, int y, int z, bool checked, neutube::EToDoAction action,
    uint64_t bodyId)
{
  ZFlyEmBody3dDocCommand::AddTodo *command =
      new ZFlyEmBody3dDocCommand::AddTodo(this);

  if (bodyId == 0) {
    bodyId = getMainDvidReader().readBodyIdAt(x, y, z);
  }

  if (m_bodySet.contains(bodyId)) {
    command->setTodoItem(x, y, z, checked, action, bodyId);
    if (command->hasValidItem()) {
      pushUndoCommand(command);
    } else {
      delete command;
    }
  }
}

void ZFlyEmBody3dDoc::executeRemoveTodoCommand()
{
  ZFlyEmBody3dDocCommand::RemoveTodo *command =
      new ZFlyEmBody3dDocCommand::RemoveTodo(this);
  const TStackObjectSet& objSet = getObjectGroup().getSelectedSet(
        ZStackObject::TYPE_FLYEM_TODO_ITEM);
  for (TStackObjectSet::const_iterator iter = objSet.begin();
       iter != objSet.end(); ++iter) {
    const ZFlyEmToDoItem *item = dynamic_cast<const ZFlyEmToDoItem*>(*iter);
    if (item != NULL) {
      command->addTodoItem(
            item->getX(), item->getY(), item->getZ(), item->getBodyId());
    }
  }
  if (command->hasValidItem()) {
    pushUndoCommand(command);
  } else {
    delete command;
  }
}

void ZFlyEmBody3dDoc::recycleObject(ZStackObject *obj)
{
  if (removeObject(obj, false)) {
    dumpGarbage(obj, true);
  }
}

void ZFlyEmBody3dDoc::killObject(ZStackObject *obj)
{
  if (removeObject(obj, false)) {
    dumpGarbage(obj, false);
  }
}

void ZFlyEmBody3dDoc::removeBodyFunc(uint64_t bodyId, bool removingAnnotation)
{
  ZOUT(LTRACE(), 5) << "Remove body:" << bodyId;

  QString threadId = QString("removeBody(%1)").arg(bodyId);
  if (!m_futureMap.isAlive(threadId)) {
    //TStackObjectList objList = getObjectGroup().findSameSource(
        //  ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId));
    TStackObjectList objList = getObjectGroup().findSameClass(
          getBodyObjectType(),
          ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId));

//    QMutexLocker locker(&m_garbageMutex);
//    beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
//    blockSignals(true);
    for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
         ++iter) {
//      removeObject(*iter, false);
//      dumpGarbageUnsync(*iter, true);
      LINFO() << "Put" << (*iter)->getSource() << " in recycle";
      getDataBuffer()->addUpdate(*iter, ZStackDocObjectUpdate::ACTION_RECYCLE);
    }

    if (!objList.isEmpty()) {
      emit bodyRemoved(bodyId);
    } else {
      LWARN() << "No object found for" << bodyId;
#ifdef _DEBUG_
      LINFO() << "Current sources:";

      TStackObjectList objList = getObjectGroup().getObjectList(
            getBodyObjectType());
      for (const ZStackObject *obj : objList) {
        LINFO() << obj->getObjectClass() << obj->getSource();
      }
#endif
    }

    if (removingAnnotation) {
      objList = getObjectGroup().findSameSource(
            ZStackObjectSourceFactory::MakeTodoPunctaSource(bodyId));
      for (TStackObjectList::iterator iter = objList.begin();
           iter != objList.end(); ++iter) {
//        removeObject(*iter, false);
//        dumpGarbageUnsync(*iter, true);
        getDataBuffer()->addUpdate(*iter, ZStackDocObjectUpdate::ACTION_KILL);
      }

      objList = getObjectGroup().findSameSource(
            ZStackObjectSourceFactory::MakeFlyEmTBarSource(bodyId));
      for (TStackObjectList::iterator iter = objList.begin();
           iter != objList.end(); ++iter) {
//        removeObject(*iter, false);
//        dumpGarbageUnsync(*iter, true);
        getDataBuffer()->addUpdate(*iter, ZStackDocObjectUpdate::ACTION_KILL);
      }

      objList = getObjectGroup().findSameSource(
            ZStackObjectSourceFactory::MakeFlyEmPsdSource(bodyId));
      for (TStackObjectList::iterator iter = objList.begin();
           iter != objList.end(); ++iter) {
//        removeObject(*iter, false);
//        dumpGarbageUnsync(*iter, true);
        getDataBuffer()->addUpdate(*iter, ZStackDocObjectUpdate::ACTION_KILL);
      }

      objList = getObjectGroup().findSameSource(
            ZStackObjectSourceFactory::MakeFlyEmSeedSource(bodyId));
      getDataBuffer()->addUpdate(objList, ZStackDocObjectUpdate::ACTION_KILL);

      objList = getObjectList(ZStackObjectRole::ROLE_TMP_RESULT);
      getDataBuffer()->addUpdate(objList, ZStackDocObjectUpdate::ACTION_KILL);
    }

    getDataBuffer()->deliver();
//    blockSignals(false);

    /*
    updateModelData(SWC_DATA);
    updateModelData(PUNCTA_DATA);

    QList<Z3DWindow*> windowList = getUserList<Z3DWindow>();
    foreach (Z3DWindow *window, windowList) {
      window->punctaChanged();
      window->swcChanged();
      window->updateTodoDisplay();
    }
    */

//    endObjectModifiedMode();
//    notifyObjectModified(true);
  }

  ZOUT(LTRACE(), 5) << "Remove body:" << bodyId << "Done.";
}

ZStackObject* ZFlyEmBody3dDoc::retriveBodyObject(
    uint64_t bodyId, int zoom, flyem::EBodyType bodyType,
    ZStackObject::EType objType)
{
  std::string source = ZStackObjectSourceFactory::MakeFlyEmBodySource(
        bodyId, zoom, bodyType);
  ZStackObject *obj = getObjectGroup().findFirstSameSource(
        objType, source);

  return obj;
}

ZStackObject* ZFlyEmBody3dDoc::retriveBodyObject(uint64_t bodyId, int zoom)
{
  return retriveBodyObject(bodyId, zoom, getBodyType(), getBodyObjectType());
}

ZSwcTree* ZFlyEmBody3dDoc::retrieveBodyModel(
    uint64_t bodyId, int zoom, flyem::EBodyType bodyType)
{
  ZStackObject *obj =
      retriveBodyObject(bodyId, zoom, bodyType, ZStackObject::TYPE_SWC);

  ZSwcTree *tree = dynamic_cast<ZSwcTree*>(obj);

  return tree;
}

ZMesh* ZFlyEmBody3dDoc::retrieveBodyMesh(uint64_t bodyId, int zoom)
{
  ZStackObject *obj = retriveBodyObject(bodyId, zoom);

  ZMesh *mesh = dynamic_cast<ZMesh*>(obj);

  return mesh;
}

#if 0
ZSwcTree* ZFlyEmBody3dDoc::makeBodyModel(uint64_t bodyId, int zoom)
{
  return makeBodyModel(bodyId, zoom, getBodyType());
}
#endif

ZSwcTree* ZFlyEmBody3dDoc::recoverFullBodyFromGarbage(uint64_t bodyId, int resLevel)
{
  ZSwcTree *tree = NULL;

//  int maxZoom = getDvidTarget().getMaxLabelZoom();
  for (int zoom = 0; zoom <= resLevel; ++zoom) {
    tree = recoverFromGarbage<ZSwcTree>(
          ZStackObjectSourceFactory::MakeFlyEmBodySource(
            bodyId, zoom, flyem::BODY_FULL));
    if (tree != NULL) {
      break;
    }
  }

  return tree;
}

ZMesh* ZFlyEmBody3dDoc::recoverMeshFromGarbage(uint64_t bodyId, int resLevel)
{
  ZMesh *mesh = NULL;

  for (int zoom = 0; zoom <= resLevel; ++zoom) {
    mesh = recoverFromGarbage<ZMesh>(
          ZStackObjectSourceFactory::MakeFlyEmBodySource(
            bodyId, zoom, flyem::BODY_MESH));
    if (mesh != NULL) {
      break;
    }
  }

  return mesh;
}

std::vector<ZSwcTree*> ZFlyEmBody3dDoc::makeDiffBodyModel(
    uint64_t bodyId1, ZDvidReader &diffReader, int zoom,
    flyem::EBodyType bodyType)
{
  if (!m_bodyReader.isReady()) {
    m_bodyReader.open(m_workDvidReader.getDvidTarget());
  }

  ZIntPoint pt = m_bodyReader.readBodyPosition(bodyId1);
  uint64_t bodyId2 = diffReader.readBodyIdAt(pt);

  std::vector<ZSwcTree*> treeArray =
      makeDiffBodyModel(bodyId1, bodyId2, diffReader, zoom, bodyType);

  for (std::vector<ZSwcTree*>::iterator iter = treeArray.begin();
       iter != treeArray.end(); ++iter) {
    ZSwcTree *tree = *iter;
    if (tree != NULL) {
      tree->setSource(
            ZStackObjectSourceFactory::MakeFlyEmBodyDiffSource(
              bodyId1, tree->getSource()));
    }
  }

  return treeArray;

}

std::vector<ZSwcTree*> ZFlyEmBody3dDoc::makeDiffBodyModel(
    const ZIntPoint &pt, ZDvidReader &diffReader, int zoom,
    flyem::EBodyType bodyType)
{
  if (!m_bodyReader.isReady()) {
    m_bodyReader.open(m_workDvidReader.getDvidTarget());
  }

  uint64_t bodyId1 = m_bodyReader.readBodyIdAt(pt);
  uint64_t bodyId2 = diffReader.readBodyIdAt(pt);

  std::vector<ZSwcTree*> treeArray =
      makeDiffBodyModel(bodyId1, bodyId2, diffReader, zoom, bodyType);

  for (std::vector<ZSwcTree*>::iterator iter = treeArray.begin();
       iter != treeArray.end(); ++iter) {
    ZSwcTree *tree = *iter;
    if (tree != NULL) {
      tree->setSource(
            ZStackObjectSourceFactory::MakeFlyEmBodyDiffSource(
              bodyId1, tree->getSource()));
    }
  }

  return treeArray;

}

uint64_t ZFlyEmBody3dDoc::getMappedId(uint64_t bodyId) const
{
  for (const auto &m : m_tarIdToMeshIds) {
    if (m.second.count(bodyId) > 0) {
      bodyId = m.first;
      break;
    }
  }

  return decode(bodyId);
}

QSet<uint64_t> ZFlyEmBody3dDoc::getUnencodedBodySet() const
{
  QSet<uint64_t> bodySet;
  foreach (uint64_t bodyId, m_bodySet) {
    bodySet.insert(decode(bodyId));
  }
  return bodySet;
}

ZDvidReader& ZFlyEmBody3dDoc::getBodyReader()
{
  if (!m_bodyReader.isReady()) {
    m_bodyReader.open(m_workDvidReader.getDvidTarget());
  }

  return m_bodyReader;
}

std::vector<ZSwcTree*> ZFlyEmBody3dDoc::makeDiffBodyModel(
    uint64_t bodyId1, uint64_t bodyId2, ZDvidReader &diffReader, int zoom,
    flyem::EBodyType bodyType)
{
  std::vector<ZSwcTree*> treeArray;

  if (bodyId1 > 0 && bodyId2 > 0) {
    if (bodyType == flyem::BODY_COARSE || bodyType == flyem::BODY_SKELETON) {
      zoom = 0;
    }

    if (bodyType == flyem::BODY_COARSE) {
      ZObject3dScan obj1 = getBodyReader().readCoarseBody(bodyId1);
      ZObject3dScan obj2 = diffReader.readCoarseBody(bodyId2);
      treeArray = ZSwcFactory::CreateDiffSurfaceSwc(obj1, obj2);
      for (std::vector<ZSwcTree*>::iterator iter = treeArray.begin();
           iter != treeArray.end(); ++iter) {
        ZSwcTree *tree = *iter;
        if (tree != NULL) {
//          tree->translate(-getDvidInfo().getStartBlockIndex());
          tree->rescale(getDvidInfo().getBlockSize().getX(),
                        getDvidInfo().getBlockSize().getY(),
                        getDvidInfo().getBlockSize().getZ());
          tree->translate(getDvidInfo().getBlockSize() / 2);
        }
      }
    } else {
      ZObject3dScan obj1;
      ZObject3dScan obj2;
      getBodyReader().readMultiscaleBody(bodyId1, zoom, true, &obj1);
      diffReader.readMultiscaleBody(bodyId2, zoom, true, &obj2);
      treeArray = ZSwcFactory::CreateDiffSurfaceSwc(obj1, obj2);
    }
  }

  return treeArray;
}


ZSwcTree* ZFlyEmBody3dDoc::makeBodyModel(
    uint64_t bodyId, int zoom, flyem::EBodyType bodyType)
{
  ZSwcTree *tree = NULL;

  if (bodyType == flyem::BODY_COARSE || bodyType == flyem::BODY_SKELETON) {
    zoom = 0;
  }

  if (bodyType == flyem::BODY_COARSE) {
    tree = recoverFromGarbage<ZSwcTree>(
          ZStackObjectSourceFactory::MakeFlyEmCoarseBodySource(bodyId));
  } else if (bodyType == flyem::BODY_SKELETON) {
    tree = recoverFromGarbage<ZSwcTree>(
          ZStackObjectSourceFactory::MakeFlyEmBodySource(
            bodyId, 0, bodyType));
  } else if (bodyType == flyem::BODY_FULL) {
    tree = recoverFullBodyFromGarbage(bodyId, zoom);
  }

  if (tree == NULL) {
    if (bodyId > 0) {
      int t = m_objectTime.elapsed();
      if (bodyType == flyem::BODY_SKELETON) {
        tree = m_workDvidReader.readSwc(bodyId);
      } else if (bodyType == flyem::BODY_COARSE) {
        ZObject3dScan obj = m_workDvidReader.readCoarseBody(bodyId);
        if (!obj.isEmpty()) {
          tree = ZSwcFactory::CreateSurfaceSwc(obj);
//          tree->translate(-getDvidInfo().getStartBlockIndex());
          tree->rescale(getDvidInfo().getBlockSize().getX(),
                        getDvidInfo().getBlockSize().getY(),
                        getDvidInfo().getBlockSize().getZ());
          tree->translate(/*getDvidInfo().getStartCoordinates()+*/
                          getDvidInfo().getBlockSize() / 2);
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
          ZObject3dScan obj;
          m_workDvidReader.readMultiscaleBody(bodyId, zoom, true, &obj);
#ifdef _DEBUG_2
          m_dvidReader.readBodyDs(bodyId, true, &obj);
#endif
          if (!obj.isEmpty()) {
            tree = ZSwcFactory::CreateSurfaceSwc(obj, 3);
          }
        } else {
          tree = ZSwcFactory::CreateSurfaceSwc(*cachedBody);
        }
      }

      if (tree != NULL) {
        tree->setTimeStamp(t);
        if (tree->getSource() == "oversize" || zoom <= 2) {
          zoom = 0;
        }
        tree->setSource(
              ZStackObjectSourceFactory::MakeFlyEmBodySource(
                bodyId, zoom, bodyType));
        tree->setObjectClass(
              ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId));
        tree->setLabel(bodyId);
      }
    }
  }

  return tree;
}

namespace {
  const uint64_t ENCODING_BASE = 100000000000;
  const uint64_t ENCODING_TAR = 100;
}

uint64_t ZFlyEmBody3dDoc::encode(uint64_t rawId, unsigned int level, bool tar)
{
  uint64_t tarEncoding = tar ? ENCODING_TAR : 0;
  return (level + tarEncoding) * ENCODING_BASE + rawId;
}

uint64_t ZFlyEmBody3dDoc::decode(uint64_t encodedId)
{
  return encodedId % ENCODING_BASE;
}

bool ZFlyEmBody3dDoc::encodesTar(uint64_t id) {
  uint64_t encoded = id / ENCODING_BASE;
  uint64_t encodedTar = encoded / ENCODING_TAR;
  return (encodedTar != 0);
}

unsigned int ZFlyEmBody3dDoc::encodedLevel(uint64_t id) {
  uint64_t encoded = id / ENCODING_BASE;
  uint64_t encodedLevel = encoded % ENCODING_TAR;
  return encodedLevel;
}

bool ZFlyEmBody3dDoc::fromTar(uint64_t id) const
{
  if (encodesTar(id)) {
    return true;
  } else {
    for (auto const& it : m_tarIdToMeshIds) {
      auto const& meshIds = it.second;
      if (meshIds.find(id) != meshIds.end()) {
        return true;
      }
    }
    return false;
  }
}


bool ZFlyEmBody3dDoc::getCachedMeshes(uint64_t bodyId, int zoom, std::map<uint64_t, ZMesh *> &result)
{
  if (encodesTar(bodyId)) {
    auto it = m_tarIdToMeshIds.find(bodyId);
    if (it != m_tarIdToMeshIds.end()) {
      auto& meshIds = it->second;
      std::vector<ZMesh *> recoveredMeshes;

      for (uint64_t meshId : meshIds) {
        if (ZMesh *mesh = recoverMeshFromGarbage(meshId, 0)) {
          recoveredMeshes.push_back(mesh);
        }
      }

      if (recoveredMeshes.size() == meshIds.size()) {
        for (ZMesh *mesh : recoveredMeshes) {
          result[mesh->getLabel()] = mesh;
        }
        return true;
      }
    }
    return false;
  } else {
    ZMesh *mesh = recoverMeshFromGarbage(bodyId, zoom);
  #if 0 //todo
    if (mesh == NULL) {
      std::string source = ZStackObjectSourceFactory::MakeFlyEmBodySource(
            bodyId, zoom, flyem::BODY_MESH);
      mesh = dynamic_cast<ZMesh*>(
            takeObjectFromCache(ZStackObject::TYPE_MESH, source));
    }
  #endif
    if (mesh) {
      result[bodyId] = mesh;
    }
    return (mesh != nullptr);
  }
}

ZMesh *ZFlyEmBody3dDoc::readMesh(
    const ZDvidReader &reader, uint64_t bodyId, int zoom)
{
  ZMesh *mesh = reader.readMesh(bodyId, zoom);

  if (mesh == NULL) {
    bool loaded = false;
    if (zoom > getMaxResLevel()) {
      //For zooming level beyond the range, ignore it if the mesh at any level exists.
      loaded = !(getObjectGroup().findSameClass(
                   ZStackObject::TYPE_MESH,
                   ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId)).
                 isEmpty());
    }

    if (!loaded) { //Now make mesh from sparse vol
      ZObject3dScan obj;
      if (zoom == 0) {
        reader.readMultiscaleBody(bodyId, zoom, true, &obj);
      } else if (zoom >= getMaxResLevel()){
        reader.readCoarseBody(bodyId, &obj);
        obj.setDsIntv(getDvidInfo().getBlockSize() - 1);
      }
      mesh = ZMeshFactory::MakeMesh(obj);
      if (mesh) {
        mesh->setLabel(bodyId);
      }
    }
  }

  return mesh;
}

ZMesh *ZFlyEmBody3dDoc::readMesh(uint64_t bodyId, int zoom)
{
  return readMesh(m_workDvidReader, bodyId, zoom);
}

namespace {
  void finalizeMesh(ZMesh *mesh, int zoom, int t)
  {
    if (mesh) {
      uint64_t id = mesh->getLabel();
      mesh->prepareNormals();
      mesh->setTimeStamp(t);
      auto source = ZStackObjectSourceFactory::MakeFlyEmBodySource(id, zoom, flyem::BODY_MESH);
      mesh->setSource(source);
      auto objClass = ZStackObjectSourceFactory::MakeFlyEmBodySource(id);
      mesh->setObjectClass(objClass);
    }
  }
}

void ZFlyEmBody3dDoc::makeBodyMeshModels(uint64_t id, int zoom, std::map<uint64_t, ZMesh*> &result)
{
  if ((id == 0) || getCachedMeshes(id, zoom, result)) {
    return;
  }

  int t = m_objectTime.elapsed();

  if (encodesTar(id)) {
    m_tarIdToMeshIds[id].clear();

    emit meshArchiveLoadingStarted();

    // It is challenging to emit progress updates as m_dvidReader reads the data for the
    // tar archive, so just initialize the progress meter to show ani ntermediate status.

    const float PROGRESS_FRACTION_START = 1 / 3.0;
    emit meshArchiveLoadingProgress(PROGRESS_FRACTION_START);

    size_t bytesTotal;
//    size_t bytesRead = 0;
    if (struct archive *arc = m_workDvidReader.readMeshArchiveStart(id, bytesTotal)) {
      std::vector<ZMesh*> resultVec;

      // When reading the meshes, decompress them in parallel to improve performance.
      // The following lambda function updates the progress dialog during the decompression.

      auto progress = [=](size_t i, size_t n) {
        float fraction = float(i) / n;
        float progressFraction = PROGRESS_FRACTION_START + (1 - PROGRESS_FRACTION_START) * fraction;
        emit meshArchiveLoadingProgress(progressFraction);
      };

      m_workDvidReader.readMeshArchiveAsync(arc, resultVec, progress);
      for (ZMesh *mesh : resultVec) {
        finalizeMesh(mesh, 0, t);
        result[mesh->getLabel()] = mesh;
        m_tarIdToMeshIds[id].insert(mesh->getLabel());
      }

      m_workDvidReader.readMeshArchiveEnd(arc);

      emit meshArchiveLoadingEnded();
    } else {
      QString title = "Mesh Loading Failed";
      uint64_t idUnencoded = decode(id);
      QString text = "DVID mesh archive does not contain ID " +
          QString::number(idUnencoded) + " (encoded as " + QString::number(id) + ")";
      ZWidgetMessage msg(title, text, neutube::MSG_ERROR, ZWidgetMessage::TARGET_DIALOG);
      emit messageGenerated(msg);
      emit meshArchiveLoadingEnded();
    }

  } else {
    ZStackObject *obj = takeObjectFromBuffer(
          ZStackObject::TYPE_MESH,
          ZStackObjectSourceFactory::MakeFlyEmBodySource(id, 0, flyem::BODY_MESH));
    ZMesh *mesh = dynamic_cast<ZMesh*>(obj);
    if (mesh == NULL) {
      mesh = readMesh(id, zoom);
    } else {
      zoom = 0;
    }
    finalizeMesh(mesh, zoom, t);
    result[id] = mesh;
  }
}

const ZDvidInfo& ZFlyEmBody3dDoc::getDvidInfo() const
{
  return m_dvidInfo;
}

void ZFlyEmBody3dDoc::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
  m_workDvidReader.clear();
  updateDvidInfo();
}

const ZDvidReader& ZFlyEmBody3dDoc::getMainDvidReader() const
{
  return m_mainDvidWriter.getDvidReader();
}

void ZFlyEmBody3dDoc::updateDvidInfo()
{
  m_dvidInfo.clear();

  if (!m_workDvidReader.isReady()) {
    m_workDvidReader.open(getDvidTarget());
    m_mainDvidWriter.open(m_workDvidReader.getDvidTarget());
  }

  if (m_workDvidReader.isReady()) {
    m_dvidInfo = m_workDvidReader.readLabelInfo();
    ZDvidGraySlice *slice = getArbGraySlice();
    if (slice != NULL) {
      slice->setDvidTarget(m_workDvidReader.getDvidTarget());
    }
  }
}

void ZFlyEmBody3dDoc::processBodySelectionChange()
{
  std::set<uint64_t> bodySet =
      getDataDocument()->getSelectedBodySet(neutube::BODY_LABEL_ORIGINAL);

  addBodyChangeEvent(bodySet.begin(), bodySet.end());
}

#if 0
void ZFlyEmBody3dDoc::runLocalSplitFunc()
{
  notifyWindowMessageUpdated("Starting local split ...");
  uint64_t bodyId = protectBodyForSplit();

  if (bodyId > 0) {
    if (loadDvidSparseStack(bodyId)) {
      notifyWindowMessageUpdated("Running local split ...");
      QList<ZStackObject*> seedList = getObjectList(ZStackObjectRole::ROLE_SEED);
      if (seedList.size() > 1) {
        ZStackWatershedContainer container(NULL, NULL);
        foreach (ZStackObject *seed, seedList) {
          container.addSeed(seed);
        }

        container.setRangeHint(ZStackWatershedContainer::RANGE_SEED_BOUND);

        ZDvidSparseStack *sparseStack =
            getDataDocument()->getDvidSparseStack(
              container.getRange(), flyem::BODY_SPLIT_ONLINE);
        container.setData(NULL, sparseStack->getSparseStack(container.getRange()));

        std::vector<ZStackWatershedContainer*> containerList =
            container.makeLocalSeedContainer(256);

        ZOUT(LINFO(), 5) << containerList.size() << "containers";
        for (ZStackWatershedContainer *subcontainer : containerList) {
          subcontainer->run();
          ZStackDocAccessor::ParseWatershedContainer(this, subcontainer);
          delete subcontainer;
        }
        notifyWindowMessageUpdated("Local split finished.");
      } else {
        //    std::cout << "Less than 2 seeds found. Abort." << std::endl;
        notifyWindowMessageUpdated("Less than 2 seeds found. Splitting canceled.");
      }
    } else {
      notifyWindowMessageUpdated("Failed to load body data. Split aborted.");
    }
    releaseBody(bodyId);
  } else {
    notifyWindowMessageUpdated("Failed to secure body data. Split aborted.");
  }
}

void ZFlyEmBody3dDoc::runFullSplitFunc()
{
  notifyWindowMessageUpdated("Starting split ...");
  uint64_t bodyId = protectBodyForSplit();

  if (bodyId > 0) {
    if (loadDvidSparseStack(bodyId)) {
      notifyWindowMessageUpdated("Running full split ...");
      QList<ZStackObject*> seedList = getObjectList(ZStackObjectRole::ROLE_SEED);
      if (ZStackObjectAccessor::GetLabelCount(seedList) > 1) {
        ZStackWatershedContainer container(NULL, NULL);
        foreach (ZStackObject *seed, seedList) {
          container.addSeed(seed);
        }

        container.setRangeHint(ZStackWatershedContainer::RANGE_FULL);

        ZDvidSparseStack *sparseStack =
            getDataDocument()->getDvidSparseStack(
              container.getRange(), flyem::BODY_SPLIT_ONLINE);
        container.setData(
              NULL, sparseStack->getSparseStack(container.getRange()));
        container.run();
        ZStackDocAccessor::ParseWatershedContainer(this, &container);
        notifyWindowMessageUpdated("Split finished.");
      } else {
        notifyWindowMessageUpdated("Less than 2 seed labels found. Splitting canceled.");
      }
    } else {
      notifyWindowMessageUpdated("Failed to load body data. Split aborted.");
    }
    releaseBody(bodyId);
  } else {
    notifyWindowMessageUpdated("Failed to secure body data. Split aborted.");
  }
}

void ZFlyEmBody3dDoc::runSplitFunc()
{
  notifyWindowMessageUpdated("Starting split ...");
  uint64_t bodyId = protectBodyForSplit();

  if (bodyId > 0) {
    if (loadDvidSparseStack(bodyId)) {
      notifyWindowMessageUpdated("Running split ...");
      QList<ZStackObject*> seedList = getObjectList(ZStackObjectRole::ROLE_SEED);
      if (seedList.size() > 1) {
        ZStackWatershedContainer container(NULL, NULL);
        foreach (ZStackObject *seed, seedList) {
          container.addSeed(seed);
        }

        container.setRangeHint(ZStackWatershedContainer::RANGE_SEED_ROI);

        ZDvidSparseStack *sparseStack =
            getDataDocument()->getDvidSparseStack(
              container.getRange(), flyem::BODY_SPLIT_ONLINE);
        container.setData(
              NULL, sparseStack->getSparseStack(container.getRange()));
        container.run();
        ZStackDocAccessor::ParseWatershedContainer(this, &container);
        notifyWindowMessageUpdated("Split finished.");
      } else {
        //    std::cout << "Less than 2 seeds found. Abort." << std::endl;
        notifyWindowMessageUpdated("Less than 2 seeds found. Splitting canceled.");
      }
    } else {
      notifyWindowMessageUpdated("Failed to load body data. Split aborted.");
    }
    releaseBody(bodyId);
  } else {
    notifyWindowMessageUpdated("Failed to secure body data. Split aborted.");
  }
}
#endif

void ZFlyEmBody3dDoc::runLocalSplit()
{
  ZOUT(LINFO(), 5) << "Trying local split ...";

  uint64_t bodyId = protectBodyForSplit();

  if (bodyId > 0) {
    activateSplit(bodyId, getBodyLabelType());
    if (isSplitActivated()) {
      if (!m_futureMap.isAlive(THREAD_SPLIT_KEY)) {
        removeObject(ZStackObjectRole::ROLE_SEGMENTATION, true);
        QFuture<void> future =
            QtConcurrent::run(m_splitter, &ZFlyEmBodySplitter::runLocalSplit);
        m_futureMap[THREAD_SPLIT_KEY] = future;
      }
    }
  }
}

void ZFlyEmBody3dDoc::runSplit()
{
  ZOUT(LINFO(), 5) << "Trying split ...";

  if (!m_futureMap.isAlive(THREAD_SPLIT_KEY)) {
    removeObject(ZStackObjectRole::ROLE_SEGMENTATION, true);
    QFuture<void> future =
        QtConcurrent::run(m_splitter, &ZFlyEmBodySplitter::runSplit);
    m_futureMap[THREAD_SPLIT_KEY] = future;
  }
}

void ZFlyEmBody3dDoc::runFullSplit()
{
  ZOUT(LINFO(), 5) << "Trying split ...";

  if (!m_futureMap.isAlive(THREAD_SPLIT_KEY)) {
    removeObject(ZStackObjectRole::ROLE_SEGMENTATION, true);
    QFuture<void> future =
        QtConcurrent::run(m_splitter, &ZFlyEmBodySplitter::runFullSplit);
    m_futureMap[THREAD_SPLIT_KEY] = future;
  }
}

void ZFlyEmBody3dDoc::commitSplitResult()
{
  notifyWindowMessageUpdated("Uploading splitted bodies");

  QString summary;

  uint64_t oldId = m_splitter->getBodyId();
  uint64_t remainderId = oldId;

  ZObject3dScan *remainObj = new ZObject3dScan;

  QList<ZStackObject*> objList =
      getObjectList(ZStackObjectRole::ROLE_SEGMENTATION);
  foreach (ZStackObject *obj, objList) {
    ZObject3dScan *seg = dynamic_cast<ZObject3dScan*>(obj);
    if (seg != NULL) {
      if (seg->getLabel() > 1) {
        notifyWindowMessageUpdated(QString("Uploading %1/%2"));

        uint64_t newBodyId = 0;
        if (m_splitter->getLabelType() == flyem::LABEL_BODY) {
          newBodyId = m_mainDvidWriter.writeSplit(*seg, remainderId, 0);
        } else {
          std::pair<uint64_t, uint64_t> idPair =
              m_mainDvidWriter.writeSupervoxelSplit(*seg, remainderId);
          remainderId = idPair.first;
          newBodyId = idPair.second;
        }

        notifyWindowMessageUpdated(QString("Updating mesh ..."));
        ZMesh* mesh = ZMeshFactory::MakeMesh(*seg);
        m_mainDvidWriter.writeMesh(*mesh, newBodyId, 0);
        delete mesh;
        emit addingBody(newBodyId);
//        addEvent(BodyEvent::ACTION_ADD, newBodyId);

        summary += QString("Labe %1 uploaded as %2 (%3 voxels)\n").
            arg(seg->getLabel()).arg(newBodyId).arg(seg->getVoxelNumber());
      } else {
        remainObj->unify(*seg);
      }
    }
  }

  m_mainDvidWriter.deleteMesh(oldId);
  ZMesh* mesh = ZMeshFactory::MakeMesh(*remainObj);
  m_mainDvidWriter.writeMesh(*mesh, remainderId, 0);
//  delete mesh;

  if (remainderId == oldId) { //Update the body if the main ID remain unchanged
    mesh->setLabel(oldId);
    mesh->setSource(
          ZStackObjectSourceFactory::MakeFlyEmBodySource(
            oldId, 0, flyem::BODY_MESH));
    ZStackDocAccessor::AddObjectUnique(this, mesh);
  } else {
    emit removingBody(oldId);
    emit addingBody(remainderId);
    delete mesh;
  }
//  addEvent(BodyEvent::ACTION_REMOVE, oldId);
  ZStackDocAccessor::RemoveObject(
        this, ZStackObjectRole::ROLE_SEGMENTATION, true);

  m_splitter->setBodyId(remainderId);

  ZDvidSparseStack *sparseStack = getDataDocument()->getDvidSparseStack();
  if (sparseStack != NULL) {
    sparseStack->setObjectMask(remainObj);
  }

  notifyWindowMessageUpdated(summary);
}

void ZFlyEmBody3dDoc::waitForSplitToBeDone()
{
  m_futureMap.waitForFinished(THREAD_SPLIT_KEY);
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

void ZFlyEmBody3dDoc::compareBody(ZDvidReader &diffReader)
{
  ZIntPoint pt;
  pt.invalidate();
  compareBody(diffReader, pt);
}

void ZFlyEmBody3dDoc::compareBody(ZDvidReader &diffReader, const ZIntPoint &pt)
{
  ZFlyEmProofDoc *doc = getDataDocument();

  if (doc != NULL && diffReader.isReady()) {
#ifdef _DEBUG_
    std::cout << "Diff body target: " << std::endl;
    diffReader.getDvidTarget().print();
    std::cout << diffReader.getDvidTarget().getSegmentationName() << std::endl;
#endif

#if 0
    QString threadId = QString("processEvent()");
    if (m_futureMap.isAlive(threadId)) {
      m_futureMap[threadId].waitForFinished();
    }
#endif

    dumpAllBody(false);

    std::vector<ZSwcTree*> treeArray;
    if (pt.isValid()) {
      treeArray = makeDiffBodyModel(pt, diffReader, 0, getBodyType());
    } else {
      std::set<uint64_t> bodySet = doc->getSelectedBodySet(
            neutube::BODY_LABEL_ORIGINAL);
      for (std::set<uint64_t>::const_iterator iter = bodySet.begin();
           iter != bodySet.end(); ++iter) {
        uint64_t bodyId = *iter;
        std::vector<ZSwcTree*> tmpArray = makeDiffBodyModel(
              bodyId, diffReader, 0, getBodyType());
        treeArray.insert(treeArray.end(), tmpArray.begin(), tmpArray.end());
      }
    }

    bool bodyAdded = false;
    for (std::vector<ZSwcTree*>::iterator iter = treeArray.begin();
         iter != treeArray.end(); ++iter) {
      ZSwcTree *tree = *iter;
      if (tree != NULL) {
        getDataBuffer()->addUpdate(
              tree, ZStackDocObjectUpdate::ACTION_ADD_UNIQUE);
      }
      bodyAdded = true;
    }
    if (bodyAdded) {
      getDataBuffer()->deliver();
    }
  }
}

std::vector<std::string> ZFlyEmBody3dDoc::getParentUuidList() const
{
  std::vector<std::string> versionList;

  ZFlyEmProofDoc *doc = getDataDocument();
  if (doc != NULL) {
    ZDvidTarget target = m_workDvidReader.getDvidTarget();
    const ZDvidVersionDag &dag = doc->getVersionDag();

    versionList = dag.getParentList(target.getUuid());
  }

  return versionList;
}

std::vector<std::string> ZFlyEmBody3dDoc::getAncestorUuidList() const
{
  std::vector<std::string> versionList;

  ZFlyEmProofDoc *doc = getDataDocument();
  if (doc != NULL) {
    ZDvidTarget target = m_workDvidReader.getDvidTarget();
    const ZDvidVersionDag &dag = doc->getVersionDag();

    versionList = dag.getAncestorList(target.getUuid());
  }

  return versionList;
}


void ZFlyEmBody3dDoc::compareBody()
{
  ZFlyEmProofDoc *doc = getDataDocument();

  if (doc != NULL) {
    std::set<uint64_t> bodySet = doc->getSelectedBodySet(
          neutube::BODY_LABEL_ORIGINAL);
    if (bodySet.size() == 1) {
      const ZDvidVersionDag &dag = doc->getVersionDag();

      ZDvidTarget target = getBodyReader().getDvidTarget();
      std::vector<std::string> versionList =
          dag.getParentList(target.getUuid());
      if (!versionList.empty()) {
        std::string uuid = versionList.front();
        target.setUuid(uuid);
        ZDvidReader diffReader;
        if (diffReader.open(target)) {
          compareBody(diffReader);
        }
      }
    }
  }
}

void ZFlyEmBody3dDoc::compareBody(const ZFlyEmBodyComparisonDialog *dlg)
{
  ZFlyEmProofDoc *doc = getDataDocument();

  if (doc != NULL) {
    ZDvidTarget target = getBodyReader().getDvidTarget();
    target.setUuid(dlg->getUuid());
    if (dlg->usingCustomSegmentation()) {
      target.useDefaultDataSetting(false);
      target.setSegmentationName(dlg->getSegmentation());
    } else if (dlg->usingSameSegmentation()) {
      target.useDefaultDataSetting(false);
    } else if (dlg->usingDefaultSegmentation()) {
      target.useDefaultDataSetting(true);
    }

    ZDvidReader diffReader;
    if (diffReader.open(target)) {
      if (dlg->usingCustomSegmentation()) {
        diffReader.syncBodyLabelName();
      }

      ZIntPoint pt = dlg->getPosition();
      compareBody(diffReader, pt);
    }
  }
}

void ZFlyEmBody3dDoc::compareBody(const std::string &uuid)
{
  ZFlyEmProofDoc *doc = getDataDocument();

  if (doc != NULL) {
    std::set<uint64_t> bodySet = doc->getSelectedBodySet(
          neutube::BODY_LABEL_ORIGINAL);
    if (bodySet.size() == 1) {
      ZDvidTarget target = getBodyReader().getDvidTarget();
      target.setUuid(uuid);

#ifdef _DEBUG_
    std::cout << "Diff body target: " << std::endl;
    std::cout << target.getSegmentationName() << std::endl;
#endif

      ZDvidReader diffReader;
      if (diffReader.open(target)) {
        compareBody(diffReader);
      }
    }
  }
}

#if 0
template <typename InputIterator>
void ZFlyEmBody3dDoc::dumpGarbage(
    const InputIterator &first, const InputIterator &last, bool recycable)
{
  QMutexLocker locker(&m_garbageMutex);


  for (InputIterator iter = first; iter != last; ++iter) {
    ZStackObject *obj = *iter;
    m_garbageMap[obj].setTimeStamp(m_objectTime.elapsed());
    m_garbageMap[obj].setRecycable(recycable);
    ZOUT(LTRACE(), 5) << obj << "dumped" << obj->getSource();
  }

  m_garbageJustDumped = true;
}
#endif

void ZFlyEmBody3dDoc::dumpGarbageUnsync(ZStackObject *obj, bool recycable)
{
  //Make old conflicted objects unrecyclable
  for (QMap<ZStackObject*, ObjectStatus>::iterator iter = m_garbageMap.begin();
       iter != m_garbageMap.end(); ++iter) {
    ZStackObject *oldObj = iter.key();
    ObjectStatus &status =iter.value();
    if (oldObj->getType() == obj->getType() &&
        oldObj->getSource() == obj->getSource()) {
      status.setRecycable(false);
    }
  }

  m_garbageMap[obj].setTimeStamp(m_objectTime.elapsed());
  m_garbageMap[obj].setRecycable(recycable);

  ZOUT(LTRACE(), 5) << obj << "dumped" << obj->getSource();

  m_garbageJustDumped = true;
}

void ZFlyEmBody3dDoc::dumpGarbage(ZStackObject *obj, bool recycable)
{
  QMutexLocker locker(&m_garbageMutex);

//  m_garbageList.append(obj);
//  m_garbageMap[obj] = m_objectTime.elapsed();
  dumpGarbageUnsync(obj, recycable);
}

void ZFlyEmBody3dDoc::dumpAllBody(bool recycable)
{
  cancelEventThread();

  ZOUT(LTRACE(), 5) << "Dump puncta";
  beginObjectModifiedMode(OBJECT_MODIFIED_CACHE);
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
  endObjectModifiedMode();
  processObjectModified();
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
          getDataBuffer()->addUpdate(tree, ZStackDocObjectUpdate::ACTION_KILL);
//          removeObject(tree, false);
        } else {
          targetTree->merge(tree);
          getDataBuffer()->addUpdate(tree, ZStackDocObjectUpdate::ACTION_KILL);
        }
//        dumpGarbage(tree, false);
      }
    }
    getDataBuffer()->deliver();
//    removeEmptySwcTree(false);
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

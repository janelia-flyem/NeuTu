#define _NEUTU_USE_REF_KEY_
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
#include "z3dwindow.h"
#include "zstackdocdatabuffer.h"
#include "dialogs/zflyembodycomparisondialog.h"
#include "zswcutil.h"
#include "zflyembody3ddockeyprocessor.h"
#include "zflyembody3ddoccommand.h"
#include "zmesh.h"
#include "zglobal.h"
#include "zflyemmisc.h"

const int ZFlyEmBody3dDoc::OBJECT_GARBAGE_LIFE = 30000;
const int ZFlyEmBody3dDoc::OBJECT_ACTIVE_LIFE = 15000;
const int ZFlyEmBody3dDoc::MAX_RES_LEVEL = 5;

ZFlyEmBody3dDoc::ZFlyEmBody3dDoc(QObject *parent) :
  ZStackDoc(parent)
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
  disconnectSwcNodeModelUpdate();
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

void ZFlyEmBody3dDoc::clearGarbage()
{
  QMutexLocker locker(&m_garbageMutex);

  ZOUT(LTRACE(), 5) << "Clear garbage objects ..." << m_garbageMap.size();

  int count = 0;
  int currentTime = m_objectTime.elapsed();
  QMutableMapIterator<ZStackObject*, ObjectStatus> iter(m_garbageMap);
   while (iter.hasNext()) {
     iter.next();
     int t = iter.value().getTimeStamp();
     int dt = currentTime - t;
     if (dt < 0) {
       iter.value().setTimeStamp(0);
     } else if (dt > OBJECT_GARBAGE_LIFE){
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

int ZFlyEmBody3dDoc::getMinResLevel() const
{
  int resLevel = 0;
  switch (getBodyType()) {
  case FlyEM::BODY_COARSE:
    resLevel = MAX_RES_LEVEL;
    break;
  default:
    break;
  }

  return resLevel;
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
    if (item->getAction() == ZFlyEmToDoItem::TO_DO) {
      item->setVisible(visible);
    }
  }

  emit todoVisibleChanged();
}

void ZFlyEmBody3dDoc::setSelectedTodoItemChecked(bool on)
{
  bool changed = false;

  const TStackObjectSet& objSet = getObjectGroup().getSelectedSet(
        ZStackObject::TYPE_FLYEM_TODO_ITEM);
  for (TStackObjectSet::const_iterator iter = objSet.begin();
       iter != objSet.end(); ++iter) {
    ZFlyEmToDoItem *item = dynamic_cast<ZFlyEmToDoItem*>(*iter);
    if (item != NULL) {
      if (item->isChecked() != on) {
        item->setChecked(on);
        m_dvidWriter.writeToDoItem(*item);
        changed = true;
      }
    }
  }

  if (changed) {
    notifyTodoModified();
  }
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
  ZSwc::SetType(getSelectedSwcNodeSet(), type);
  notifySwcModified();
}

void ZFlyEmBody3dDoc::setBodyModelSelected(const QSet<uint64_t> &bodySet)
{
  m_selectedBodySet = bodySet;
}

bool ZFlyEmBody3dDoc::hasTodoItemSelected() const
{
  return !getObjectGroup().getSelectedSet(
        ZStackObject::TYPE_FLYEM_TODO_ITEM).empty();
}

void ZFlyEmBody3dDoc::saveSplitTask()
{
  if (m_bodySet.size() == 1) {
    uint64_t bodyId = *m_bodySet.begin();
    if (bodyId > 0) {
      ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriterFromUrl(
            GET_FLYEM_CONFIG.getTaskServer());
      if (writer != NULL) {
        ZJsonObject task;
        ZDvidUrl dvidUrl(getDvidTarget());
        std::string bodyUrl = dvidUrl.getSparsevolUrl(bodyId);
        task.setEntry("signal", bodyUrl);
        ZJsonArray seedJson = ZFlyEmMisc::GetSeedJson(this);
        task.setEntry("seeds", seedJson);
//        ZJsonArray roiJson = getRoiJson();
//        if (roiJson.isEmpty()) {
//          ZIntCuboid range = ZFlyEmMisc::EstimateSplitRoi(getSeedBoundBox());
//          if (!range.isEmpty()) {
//            roiJson = range.toJsonArray();
//          }
//        }
//        if (!roiJson.isEmpty()) {
//          task.setEntry("range", roiJson);
//        }

        std::string location = writer->writeServiceTask("split", task);
        ZJsonObject taskJson;
        taskJson.setEntry(NeuTube::Json::REF_KEY, location);
  //      QUrl url(bodyUrl.c_str());
        QString taskKey = dvidUrl.getSplitTaskKey(bodyId).c_str();
  //      QString("task__") + QUrl::toPercentEncoding(bodyUrl.c_str());
        writer->writeSplitTask(taskKey, taskJson);

        std::cout << "Split task saved @" << taskKey.toStdString() << std::endl;
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

ZStackObject::EType ZFlyEmBody3dDoc::getBodyObjectType() const
{
  if (getBodyType() == FlyEM::BODY_MESH) {
    return ZStackObject::TYPE_MESH;
  }

  return ZStackObject::TYPE_SWC;
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

bool ZFlyEmBody3dDoc::hasBody(uint64_t bodyId)
{
  return m_bodySet.contains(bodyId);
}

void ZFlyEmBody3dDoc::addBody(uint64_t bodyId, const QColor &color)
{
  if (!hasBody(bodyId)) {
    m_bodySet.insert(bodyId);
    if (getBodyType() == FlyEM::BODY_SKELETON) {
      addBodyFunc(bodyId, color, -1);
    } else {
      addBodyFunc(bodyId, color, MAX_RES_LEVEL);
    }
  }
}

void ZFlyEmBody3dDoc::setBodyType(FlyEM::EBodyType type)
{
  m_bodyType = type;
  switch (m_bodyType) {
  case FlyEM::BODY_COARSE:
    setTag(NeuTube::Document::FLYEM_BODY_3D_COARSE);
    break;
  case FlyEM::BODY_FULL:
    setTag(NeuTube::Document::FLYEM_BODY_3D);
    break;
  case FlyEM::BODY_SKELETON:
    setTag(NeuTube::Document::FLYEM_SKELETON);
    break;
  case FlyEM::BODY_MESH:
    setTag(NeuTube::Document::FLYEM_MESH);
    break;
  case FlyEM::BODY_NULL:
    break;
  }
}

void ZFlyEmBody3dDoc::updateBody(
    uint64_t bodyId, const QColor &color)
{
  updateBody(bodyId, color, FlyEM::BODY_COARSE);
  updateBody(bodyId, color, FlyEM::BODY_FULL);
  updateBody(bodyId, color, FlyEM::BODY_SKELETON);
  updateBody(bodyId, color, FlyEM::BODY_MESH);
}

void ZFlyEmBody3dDoc::updateBody(
    uint64_t bodyId, const QColor &color, FlyEM::EBodyType type)
{
  beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  if (type != FlyEM::BODY_MESH) {
    ZSwcTree *tree = getBodyModel(bodyId, 0, type);
    if (tree != NULL) {
      if (tree->getColor() != color) {
        tree->setColor(color);
        processObjectModified(tree);
      }
    }
  } else {
    ZMesh *mesh = getBodyMesh(bodyId, 0);
    if (mesh != NULL) {
      if (mesh->getColor() != color) {
        mesh->setColor(color);
        mesh->pushObjectColor();
        processObjectModified(mesh);
      }
    }
  }
  endObjectModifiedMode();
  notifyObjectModified(true);
}

ZSwcTree* ZFlyEmBody3dDoc::getBodyModel(
    uint64_t bodyId, int zoom, FlyEM::EBodyType bodyType)
{
  return retrieveBodyModel(bodyId, zoom, bodyType);
}

ZMesh* ZFlyEmBody3dDoc::getBodyMesh(uint64_t bodyId, int zoom)
{
  return retrieveBodyMesh(bodyId, zoom);
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

      if (getBodyType() == FlyEM::BODY_FULL) { //using the original color
        color = labelSlice->getLabelColor(bodyId, NeuTube::BODY_LABEL_MAPPED);
      } else {
        color = labelSlice->getLabelColor(bodyId, NeuTube::BODY_LABEL_ORIGINAL);
      }
      color.setAlpha(255);
      event.setBodyColor(color);
    }
  } else {
    event.setBodyColor(Qt::white);
  }

  if (event.getAction() == BodyEvent::ACTION_ADD &&
      getBodyType() != FlyEM::BODY_SKELETON) {
    event.setResLevel(MAX_RES_LEVEL);
  }

  m_eventQueue.enqueue(event);
}

ZSwcTree* ZFlyEmBody3dDoc::getBodyQuickly(uint64_t bodyId)
{
  ZSwcTree *tree = NULL;

  if (getBodyType() == FlyEM::BODY_FULL) {
    tree = recoverFullBodyFromGarbage(bodyId, MAX_RES_LEVEL);
  }

  if (tree == NULL) {
    tree = makeBodyModel(bodyId, 0, FlyEM::BODY_COARSE);
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

void ZFlyEmBody3dDoc::addBodyMeshFunc(
    uint64_t bodyId, const QColor &color, int resLevel)
{
  bool loaded =
      !(getObjectGroup().findSameClass(
          ZStackObject::TYPE_MESH,
          ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId)).
        isEmpty());

  emit messageGenerated(ZWidgetMessage("Syncing 3D Body view ..."));
  ZMesh *mesh = makeBodyMeshModel(bodyId, resLevel);
  emit messageGenerated(ZWidgetMessage("3D Body view synced"));

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

    updateBodyFunc(bodyId, mesh);

    if (!loaded) {
      addSynapse(bodyId);
//      addTodo(bodyId);
      updateTodo(bodyId);
    }
  }
}


void ZFlyEmBody3dDoc::addBodyFunc(
    uint64_t bodyId, const QColor &color, int resLevel)
{
  if (getBodyType() == FlyEM::BODY_MESH) {
    addBodyMeshFunc(bodyId, color, resLevel);
  } else {
    removeDiffBody();

    bool loaded =
        !(getObjectGroup().findSameClass(
            ZStackObject::TYPE_SWC,
            ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId)).
          isEmpty());

    ZSwcTree *tree = NULL;
    if (resLevel == MAX_RES_LEVEL) {
      tree = getBodyQuickly(bodyId);
    } else {
      emit messageGenerated(ZWidgetMessage("Syncing 3D Body view ..."));
      tree = makeBodyModel(bodyId, resLevel, getBodyType());
      emit messageGenerated(ZWidgetMessage("3D Body view synced"));
    }

    if (tree != NULL) {
      if (ZStackObjectSourceFactory::ExtractBodyTypeFromFlyEmBodySource(
            tree->getSource()) == FlyEM::BODY_FULL) {
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

    if (hasBody(bodyId)) {
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

ZFlyEmToDoItem ZFlyEmBody3dDoc::makeTodoItem(
    int x, int y, int z, bool checked, uint64_t bodyId)
{
  ZFlyEmToDoItem item;

  ZIntPoint position = m_dvidReader.readPosition(bodyId, ZIntPoint(x, y, z));

  if (position.isValid()) {
    item.setPosition(position);
    item.setKind(ZFlyEmToDoItem::KIND_NOTE);
    item.setUserName(NeuTube::GetCurrentUserName());
    if (checked) {
      item.setChecked(checked);
    }
  }

  return item;
}

ZFlyEmToDoItem ZFlyEmBody3dDoc::readTodoItem(int x, int y, int z) const
{
  ZFlyEmToDoItem item = m_dvidReader.readToDoItem(x, y, z);

  return item;
}

void ZFlyEmBody3dDoc::addTodo(const ZFlyEmToDoItem &item, uint64_t bodyId)
{
  if (item.isValid()) {
    m_dvidWriter.writeToDoItem(item);
    updateTodo(bodyId);
  }
}

void ZFlyEmBody3dDoc::addTodoSliently(const ZFlyEmToDoItem &item)
{
  if (item.isValid()) {
    m_dvidWriter.writeToDoItem(item);
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
    m_dvidWriter.deleteToDoItem(item.getX(), item.getY(), item.getZ());
    updateTodo(bodyId);
  }
}

void ZFlyEmBody3dDoc::removeTodoSliently(const ZFlyEmToDoItem &item)
{
  if (item.isValid()) {
    m_dvidWriter.deleteToDoItem(item.getX(), item.getY(), item.getZ());
  }
}

void ZFlyEmBody3dDoc::removeTodo(int x, int y, int z)
{
  m_dvidWriter.deleteToDoItem(x, y, z);
}

void ZFlyEmBody3dDoc::addTodo(int x, int y, int z, bool checked, uint64_t bodyId)
{
  ZFlyEmToDoItem item = makeTodoItem(x, y, z, checked, bodyId);
  addTodo(item, bodyId);
}


void ZFlyEmBody3dDoc::removeBody(uint64_t bodyId)
{
  m_bodySet.remove(bodyId);
  removeBodyFunc(bodyId, true);
}

void ZFlyEmBody3dDoc::updateBodyFunc(uint64_t bodyId, ZStackObject *bodyObject)
{
  ZOUT(LTRACE(), 5) << "Update body: " << bodyId;

  QString threadId = QString("updateBody(%1)").arg(bodyId);
  if (!m_futureMap.isAlive(threadId)) {
    //TStackObjectList objList = getObjectGroup().findSameSource(
        //  ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId));
    TStackObjectList objList = getObjectGroup().findSameClass(
          bodyObject->getType(),
          ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId));

    for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
         ++iter) {
      getDataBuffer()->addUpdate(*iter, ZStackDocObjectUpdate::ACTION_RECYCLE);
    }

    getDataBuffer()->addUpdate(bodyObject, ZStackDocObjectUpdate::ACTION_ADD_UNIQUE);
    getDataBuffer()->deliver();
  }

  ZOUT(LTRACE(), 5) << "Body updated: " << bodyId;
}

void ZFlyEmBody3dDoc::executeAddTodoCommand(
    int x, int y, int z, bool checked, uint64_t bodyId)
{
  ZFlyEmBody3dDocCommand::AddTodo *command =
      new ZFlyEmBody3dDocCommand::AddTodo(this);
  command->setTodoItem(x, y, z, checked, bodyId);
  if (command->hasValidItem()) {
    pushUndoCommand(command);
  } else {
    delete command;
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
      getDataBuffer()->addUpdate(*iter, ZStackDocObjectUpdate::ACTION_RECYCLE);
    }

    if (!objList.isEmpty()) {
      emit bodyRemoved(bodyId);
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
    uint64_t bodyId, int zoom, FlyEM::EBodyType bodyType,
    ZStackObject::EType objType)
{
  ZStackObject *obj = getObjectGroup().findFirstSameSource(
        objType, ZStackObjectSourceFactory::MakeFlyEmBodySource(
          bodyId, zoom, bodyType));

  return obj;
}

ZStackObject* ZFlyEmBody3dDoc::retriveBodyObject(uint64_t bodyId, int zoom)
{
  return retriveBodyObject(bodyId, zoom, getBodyType(), getBodyObjectType());
}

ZSwcTree* ZFlyEmBody3dDoc::retrieveBodyModel(
    uint64_t bodyId, int zoom, FlyEM::EBodyType bodyType)
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
            bodyId, zoom, FlyEM::BODY_FULL));
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
            bodyId, zoom, FlyEM::BODY_MESH));
    if (mesh != NULL) {
      break;
    }
  }

  return mesh;
}

std::vector<ZSwcTree*> ZFlyEmBody3dDoc::makeDiffBodyModel(
    uint64_t bodyId1, ZDvidReader &diffReader, int zoom,
    FlyEM::EBodyType bodyType)
{
  if (!m_bodyReader.isReady()) {
    m_bodyReader.open(m_dvidReader.getDvidTarget());
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
    FlyEM::EBodyType bodyType)
{
  if (!m_bodyReader.isReady()) {
    m_bodyReader.open(m_dvidReader.getDvidTarget());
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

ZDvidReader& ZFlyEmBody3dDoc::getBodyReader()
{
  if (!m_bodyReader.isReady()) {
    m_bodyReader.open(m_dvidReader.getDvidTarget());
  }

  return m_bodyReader;
}

std::vector<ZSwcTree*> ZFlyEmBody3dDoc::makeDiffBodyModel(
    uint64_t bodyId1, uint64_t bodyId2, ZDvidReader &diffReader, int zoom,
    FlyEM::EBodyType bodyType)
{ 
  std::vector<ZSwcTree*> treeArray;

  if (bodyId1 > 0 && bodyId2 > 0) {
    if (bodyType == FlyEM::BODY_COARSE || bodyType == FlyEM::BODY_SKELETON) {
      zoom = 0;
    }

    if (bodyType == FlyEM::BODY_COARSE) {
      ZObject3dScan obj1 = getBodyReader().readCoarseBody(bodyId1);
      ZObject3dScan obj2 = diffReader.readCoarseBody(bodyId2);
      treeArray = ZSwcFactory::CreateDiffSurfaceSwc(obj1, obj2);
      for (std::vector<ZSwcTree*>::iterator iter = treeArray.begin();
           iter != treeArray.end(); ++iter) {
        ZSwcTree *tree = *iter;
        if (tree != NULL) {
          tree->translate(-getDvidInfo().getStartBlockIndex());
          tree->rescale(getDvidInfo().getBlockSize().getX(),
                        getDvidInfo().getBlockSize().getY(),
                        getDvidInfo().getBlockSize().getZ());
          tree->translate(getDvidInfo().getStartCoordinates() +
                          getDvidInfo().getBlockSize() / 2);
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
    uint64_t bodyId, int zoom, FlyEM::EBodyType bodyType)
{
  ZSwcTree *tree = NULL;

  if (bodyType == FlyEM::BODY_COARSE || bodyType == FlyEM::BODY_SKELETON) {
    zoom = 0;
  }

  if (bodyType == FlyEM::BODY_COARSE) {
    tree = recoverFromGarbage<ZSwcTree>(
          ZStackObjectSourceFactory::MakeFlyEmCoarseBodySource(bodyId));
  } else if (bodyType == FlyEM::BODY_SKELETON) {
    tree = recoverFromGarbage<ZSwcTree>(
          ZStackObjectSourceFactory::MakeFlyEmBodySource(
            bodyId, 0, bodyType));
  } else if (bodyType == FlyEM::BODY_FULL) {
    tree = recoverFullBodyFromGarbage(bodyId, zoom);
  }

  if (tree == NULL) {
    if (bodyId > 0) {
      int t = m_objectTime.elapsed();
      if (bodyType == FlyEM::BODY_SKELETON) {
        tree = m_dvidReader.readSwc(bodyId);
      } else if (bodyType == FlyEM::BODY_COARSE) {
        ZObject3dScan obj = m_dvidReader.readCoarseBody(bodyId);
        if (!obj.isEmpty()) {
          tree = ZSwcFactory::CreateSurfaceSwc(obj);
          tree->translate(-getDvidInfo().getStartBlockIndex());
          tree->rescale(getDvidInfo().getBlockSize().getX(),
                        getDvidInfo().getBlockSize().getY(),
                        getDvidInfo().getBlockSize().getZ());
          tree->translate(getDvidInfo().getStartCoordinates() +
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
          m_dvidReader.readMultiscaleBody(bodyId, zoom, true, &obj);
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

ZMesh* ZFlyEmBody3dDoc::makeBodyMeshModel(uint64_t bodyId, int zoom)
{
  ZMesh *mesh = recoverMeshFromGarbage(bodyId, zoom);

  if (mesh == NULL) {
    if (bodyId > 0) {
      int t = m_objectTime.elapsed();
      mesh = m_dvidReader.readMesh(bodyId, zoom);

      if (mesh != NULL) {
        mesh->setTimeStamp(t);
        mesh->setSource(
              ZStackObjectSourceFactory::MakeFlyEmBodySource(
                bodyId, zoom, FlyEM::BODY_MESH));
        mesh->setObjectClass(
              ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId));
        mesh->setLabel(bodyId);
      }
    }
  }

  return mesh;
}

const ZDvidInfo& ZFlyEmBody3dDoc::getDvidInfo() const
{
  return m_dvidInfo;
}

void ZFlyEmBody3dDoc::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
  m_dvidReader.clear();
  updateDvidInfo();
}

void ZFlyEmBody3dDoc::updateDvidInfo()
{
  m_dvidInfo.clear();

  if (!m_dvidReader.isReady()) {
    m_dvidReader.open(getDvidTarget());
    m_dvidWriter.open(m_dvidReader.getDvidTarget());
  }

  if (m_dvidReader.isReady()) {
    m_dvidInfo = m_dvidReader.readLabelInfo();
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
    std::cout << diffReader.getDvidTarget().getLabelBlockName() << std::endl;
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
            NeuTube::BODY_LABEL_ORIGINAL);
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
    ZDvidTarget target = m_dvidReader.getDvidTarget();
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
    ZDvidTarget target = m_dvidReader.getDvidTarget();
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
          NeuTube::BODY_LABEL_ORIGINAL);
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
      target.setLabelBlockName(dlg->getSegmentation());
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
          NeuTube::BODY_LABEL_ORIGINAL);
    if (bodySet.size() == 1) {
      ZDvidTarget target = getBodyReader().getDvidTarget();
      target.setUuid(uuid);

#ifdef _DEBUG_
    std::cout << "Diff body target: " << std::endl;
    std::cout << target.getLabelBlockName() << std::endl;
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
  notifyObjectModified();
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



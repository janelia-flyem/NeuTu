//#define _NEUTU_USE_REF_KEY_
#include "zflyembody3ddoc.h"

#include <algorithm>
#include <unordered_set>

#include <QtConcurrentRun>
#include <QMutexLocker>
#include <QElapsedTimer>

#include <archive.h>

#include "common/utilities.h"

#include "logging/zqslog.h"
#include "logging/zlog.h"
#include "logging/utilities.h"

#include "logging.h"
#include "zjsondef.h"

#include "dvid/zdvidinfo.h"
#include "dvid/zdvidurl.h"
#include "dvid/zdvidsparsestack.h"
#include "dvid/zdvidlabelslice.h"

#include "zswcfactory.h"
#include "zstackobjectsourcefactory.h"
#include "z3dgraphfactory.h"
#include "zflyemproofdoc.h"
#include "zwidgetmessage.h"

#include "zstring.h"
#include "zfiletype.h"
#include "neutubeconfig.h"
#include "z3dwindow.h"

#include "zswcutil.h"
#include "zflyembody3ddockeyprocessor.h"
#include "zflyembody3ddoccommand.h"
#include "zmesh.h"
#include "zglobal.h"
#include "zflyemmisc.h"
#include "zstroke2d.h"
#include "zobject3d.h"
#include "zmeshfactory.h"
#include "zpunctum.h"
#include "zstackdocaccessor.h"
#include "zstackwatershedcontainer.h"
#include "zstackobjectaccessor.h"
#include "mvc/zstackdocdatabuffer.h"

#include "zactionlibrary.h"
#include "dvid/zdvidgrayslice.h"

#include "misc/miscutility.h"
#include "dvid/zdvidbodyhelper.h"
#include "data3d/zstackobjecthelper.h"
#include "zstackdocproxy.h"
//#include "zflyembodyannotationdialog.h"
#include "zflyemtaskhelper.h"
#include "protocols/protocoltaskfactory.h"
#include "zstackdoc3dhelper.h"
#include "zstackobjectarray.h"

#include "zflyembodyenv.h"
#include "zflyembodystatus.h"
#include "flyemdatareader.h"
#include "zflyemproofdocutil.h"
#include "zflyemproofutil.h"
#include "zflyemtodoitem.h"
#include "zflyembodysplitter.h"

#include "dialogs/zflyembodycomparisondialog.h"
#include "dialogs/zflyemtodoannotationdialog.h"
#include "dialogs/zflyemtodofilterdialog.h"
#include "dialogs/flyemdialogfactory.h"
#include "dialogs/flyembodyannotationdialog.h"

const int ZFlyEmBody3dDoc::OBJECT_GARBAGE_LIFE = 30000;
const int ZFlyEmBody3dDoc::OBJECT_ACTIVE_LIFE = 15000;
//const int ZFlyEmBody3dDoc::MAX_RES_LEVEL = 5;
const char* ZFlyEmBody3dDoc::THREAD_SPLIT_KEY = "split";

/* Implementation details
 *
 * ZFlyEmBody3dDoc uses an event queue to coordiante body updates asynchronously.
 * The processEvent() function will be triggered every 200ms to process events
 * in the current event queue on background. The processing function (processEventFunc)
 * first goes through all the events available and generate a new event for each
 * body by merging multiple events of the same body. The priority of an event has
 * not been used yet.
 *
 * To allow fast toggling, a body recycled into the garbage object (m_garbageMap)
 * can be recovered later if it does not get obsolete. The garbage object was
 * also initially designed for improving cocurrency safety, but no longer
 * serving that purpose since the introduction of thread-safe object update
 * in ZStackDoc.
 *
 * ZFlyEmBody3dDoc supports splitting too with the help of ZFlyEmBodySplitter.
 * Splitting is run in its own thread managed by ZThreadFutureMap using the
 * THREAD_SPLIT_KEY key.
 *
 * A geometrical model created ZFlyEmBody3dDoc for a body can be:
 *   > Surface spheres
 *   > Mesh
 *   > Skeleton
 *
 * The choice is controlled by m_bodyType.
 */

ZFlyEmBody3dDoc::ZFlyEmBody3dDoc(QObject *parent) :
  ZStackDoc(parent),
  m_workDvidReader(NUM_WORK_DVID_READERS)
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

  m_helper = ZSharedPointer<ZStackDoc3dHelper>(new ZStackDoc3dHelper);
}

ZFlyEmBody3dDoc::~ZFlyEmBody3dDoc()
{
  m_quitting = true;

  clearToDestroy();

  QMutexLocker locker(&m_eventQueueMutex);
  m_eventQueue.clear();
  locker.unlock();

//  m_futureMap.waitForFinished();

  m_garbageJustDumped = false;
//  clearGarbage();

  QMutexLocker garbageLocker(&m_garbageMutex);
  for (QMap<ZStackObject*, ObjectStatus>::iterator iter = m_garbageMap.begin();
       iter != m_garbageMap.end(); ++iter) {
    delete iter.key();
  }

  LDEBUG() << "ZFlyEmBody3dDoc destroyed";
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

void ZFlyEmBody3dDoc::useCoarseOnly()
{
  int coarseLevel = getCoarseBodyZoom();
  setMinDsLevel(coarseLevel);
  setMaxDsLevel(coarseLevel);
}

bool ZFlyEmBody3dDoc::showingCoarseOnly() const
{
  int coarseLevel = getCoarseBodyZoom();
  return (coarseLevel == getMinDsLevel()) &&
      (coarseLevel == getMaxDsLevel()) && (coarseLevel > 0);
}

bool ZFlyEmBody3dDoc::isCoarseLevel(int level) const
{
  return (level > 0) && (level == getCoarseBodyZoom());
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

  if (!m_garbageMap.isEmpty()) {
    ZOUT(LKINFO, 5) << QString("Clear garbage objects: %1 ...").
                       arg(m_garbageMap.size());
  }

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
       if (obj->getType() == ZStackObject::EType::SWC) {
         ZOUT(LKINFO, 5) << QString("Deleting SWC object: %1 %2").
                            arg(neutu::ToString(obj).c_str()).
                            arg(obj->getSource().c_str());
       } else {
#ifdef _DEBUG_
         LKINFO << QString("Deleting %1 %2").arg(neutu::ToString(obj).c_str()).
                   arg(obj->getSource().c_str());
#endif
       }

       if (obj != iter.key()) {
         ZOUT(LKINFO, 5) << "Deleting failed";
       }

       delete iter.key();
       iter.remove();
       ++count;

       if (m_garbageMap.contains(obj)) {
         ZOUT(LKINFO, 5) << "Deleting failed";
       }
     }
   }

   if (count > 0) {
     ZOUT(LKINFO, 5) << QString("Garbage update: %1 removed; %2 left")
                        .arg(count)
                        .arg(m_garbageMap.size());
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

#if 0
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
#endif

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

/*
bool ZFlyEmBody3dDoc::isAdmin() const
{
  return getDataDocument()->isAdmin();
}
*/

const ZFlyEmBodyAnnotationProtocal& ZFlyEmBody3dDoc::getBodyStatusProtocol() const
{
  return getDataDocument()->getBodyStatusProtocol();
}

void ZFlyEmBody3dDoc::initArbGraySlice()
{
  ZDvidGraySlice *slice = new ZDvidGraySlice();
  slice->setSliceAxis(neutu::EAxis::ARB);
  slice->setTarget(ZStackObject::ETarget::CANVAS_3D);
  slice->setSource(
        ZStackObjectSourceFactory::MakeDvidGraySliceSource(neutu::EAxis::ARB));
  addObject(slice);
}

ZDvidGraySlice* ZFlyEmBody3dDoc::getArbGraySlice() const
{
  ZDvidGraySlice *slice = getObject<ZDvidGraySlice>(
        ZStackObjectSourceFactory::MakeDvidGraySliceSource(neutu::EAxis::ARB));

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
        ZStackObjectSourceFactory::MakeDvidGraySliceSource(neutu::EAxis::ARB));
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

int ZFlyEmBody3dDoc::getMinDsLevel() const
{
  return m_minDsLevel;
}

int ZFlyEmBody3dDoc::getMaxDsLevel() const
{
  return m_maxDsLevel;
}

void ZFlyEmBody3dDoc::removeDiffBody()
{
  ZStackDocObjectUpdate *u = new ZStackDocObjectUpdate(
        nullptr, ZStackDocObjectUpdate::EAction::CALLBACK);
  u->setCallback([this](ZStackObject*) {
    QMutexLocker locker(this->getObjectGroup().getMutex());
    auto pred = [](const ZStackObject *obj) {
      return ZStackObjectSourceFactory::IsBodyDiffSource(obj->getSource());
    };
    TStackObjectList objList = this->getObjectGroup().takeUnsync(
          ZStackObject::EType::SWC, pred);
    for (ZStackObject *obj : objList) {
      this->removeTakenObject(obj, true);
    }
    this->processObjectModified();
  });

  getDataBuffer()->addUpdate(u);

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

void ZFlyEmBody3dDoc::processEventFunc(const ZFlyEmBodyEvent &event)
{
  ZFlyEmBodyConfig config = event.getBodyConfig();

  switch (event.getAction()) {
  case ZFlyEmBodyEvent::EAction::REMOVE:
    removeBodyFunc(event.getBodyId(), true);
    break;
  case ZFlyEmBodyEvent::EAction::ADD:
    addBodyFunc(config);
    break;
  case ZFlyEmBodyEvent::EAction::UPDATE:
//    if (event.updating(BodyEvent::UPDATE_CHANGE_COLOR)) {
    if (event.updating(ZFlyEmBodyEvent::UPDATE_MULTIRES)) {
      addBodyFunc(config);
    } else {
      updateBody(event.getBodyId(), event.getBodyColor(), getBodyType());
    }
//    }
    if (event.updating(ZFlyEmBodyEvent::UPDATE_ADD_SYNAPSE)) {
      addSynapse(event.getBodyId());
    }
    if (event.updating(ZFlyEmBodyEvent::UPDATE_ADD_TODO_ITEM)) {
      addTodo(event.getBodyId());
    }
    if (event.updating(ZFlyEmBodyEvent::UPDATE_SEGMENTATION)) {
      updateSegmentation();
    }
    break;
  default:
    break;
  }

  QMutexLocker locker(&m_eventQueueMutex);
  if (!toBeRemoved(event.getBodyId())) {
    ZFlyEmBodyEvent nextEvent = event.makeHighResEvent(config, getMinDsLevel());
    if (nextEvent.isValid()) {
      m_eventQueue.enqueue(nextEvent);
    }
  }
}

void ZFlyEmBody3dDoc::showMoreDetail(uint64_t bodyId, const ZIntCuboid &range)
{
  QMutexLocker locker(&m_eventQueueMutex);
  if (m_bodyManager.contains(bodyId)) {
    if (!toBeRemoved(bodyId)) {
      ZFlyEmBodyEvent bodyEvent(ZFlyEmBodyEvent::EAction::UPDATE, bodyId);
      bodyEvent.addUpdateFlag(ZFlyEmBodyEvent::UPDATE_MULTIRES);
      bodyEvent.setBodyColor(getBodyColor(bodyId));
      bodyEvent.setRange(range);
      bodyEvent.setDsLevel(getMinDsLevel());
      if (getDvidTarget().hasMultiscaleSegmentation()) {
        bodyEvent.setLocalDsLevel(1); //need configuration
      } else {
        bodyEvent.setLocalDsLevel(0);
      }
      m_eventQueue.enqueue(bodyEvent);
    }
  }
}


void ZFlyEmBody3dDoc::setDoneItemVisible(bool visible)
{
  processObjectList<ZFlyEmToDoItem>([visible, this](ZFlyEmToDoItem *item) {
    if (item->isChecked()) {
      item->setVisible(visible);
      bufferObjectVisibilityChanged(item);
    }
  });
}

void ZFlyEmBody3dDoc::setNormalTodoVisible(bool visible)
{
  processObjectList<ZFlyEmToDoItem>([visible, this](ZFlyEmToDoItem *item) {
    if (item->getAction() == neutu::EToDoAction::TO_DO) {
      item->setVisible(visible);
      bufferObjectVisibilityChanged(item);
    }
  });
}

void ZFlyEmBody3dDoc::annotateTodoItem(
    std::function<void(ZFlyEmToDoItem *)> f,
    std::function<bool(const ZFlyEmToDoItem *)> pred)
{
  std::vector<ZIntPoint> ptArray;

  const TStackObjectSet& objSet = getObjectGroup().getSelectedSet(
        ZStackObject::EType::FLYEM_TODO_ITEM);
  for (TStackObjectSet::const_iterator iter = objSet.begin();
       iter != objSet.end(); ++iter) {
    ZFlyEmToDoItem *item = dynamic_cast<ZFlyEmToDoItem*>(*iter);
    if (item != NULL) {
      if (pred(item)) {
        f(item);
        m_mainDvidWriter.writeToDoItem(*item);
        ptArray.push_back(item->getPosition());
        bufferObjectModified(item);
      }
    }
  }

  getDataDocument()->downloadTodo(ptArray);

  processObjectModified();
}

void ZFlyEmBody3dDoc::setTodoItemAction(neutu::EToDoAction action, bool checked)
{
  annotateTodoItem(
        [action, checked](ZFlyEmToDoItem* item) {
            item->setAction(action);
            item->setChecked(checked); },
        [action, checked](const ZFlyEmToDoItem* item) -> bool{
            return (action != item->getAction()) ||
                (checked != item->isChecked());}
  );
}

void ZFlyEmBody3dDoc::setTodoItemAction(neutu::EToDoAction action)
{
  annotateTodoItem(
        [action](ZFlyEmToDoItem* item) { item->setAction(action); },
        [action](const ZFlyEmToDoItem* item) -> bool{
            return action != item->getAction();}
  );
}

void ZFlyEmBody3dDoc::annotateTodo(ZFlyEmTodoAnnotationDialog *dlg, ZStackObject *obj)
{
  ZFlyEmToDoItem *item = dynamic_cast<ZFlyEmToDoItem*>(obj);
  if (item) {
    dlg->init(*item);
    if (dlg->exec()) {
      dlg->annotate(item);
      m_mainDvidWriter.writeToDoItem(*item);
      bufferObjectModified(item);
    }

    getDataDocument()->downloadTodo(item->getPosition());
    processObjectModified();
  }
}

void ZFlyEmBody3dDoc::setSelectedTodoItemChecked(bool on)
{
//  bool changed = false;

  std::vector<ZIntPoint> ptArray;

  const TStackObjectSet& objSet = getObjectGroup().getSelectedSet(
        ZStackObject::EType::FLYEM_TODO_ITEM);
  for (TStackObjectSet::const_iterator iter = objSet.begin();
       iter != objSet.end(); ++iter) {
    ZFlyEmToDoItem *item = dynamic_cast<ZFlyEmToDoItem*>(*iter);
    if (item != NULL) {
      if (item->isChecked() != on) {
        item->setChecked(on);
        m_mainDvidWriter.writeToDoItem(*item);
        ptArray.push_back(item->getPosition());
//        changed = true;
        bufferObjectModified(item);
      }
    }
  }

  getDataDocument()->downloadTodo(ptArray);

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
        ZStackObject::EType::FLYEM_TODO_ITEM).empty();
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
    //Update the segmentation (splits) only when one body is selected
    //Todo: association between bodies and segmentations
    uint64_t bodyId = m_bodyManager.getSingleBodyId();
    if (bodyId > 0) {
      addEvent(ZFlyEmBodyEvent::EAction::UPDATE, bodyId,
               ZFlyEmBodyEvent::UPDATE_SEGMENTATION);
    }

    /*
    if (m_bodySet.size() == 1) {
      uint64_t bodyId = *m_bodySet.begin();
      addEvent(ZFlyEmBodyEvent::EAction::ACTION_UPDATE, bodyId, ZFlyEmBodyEvent::UPDATE_SEGMENTATION);
    }
    */
  }
}

void ZFlyEmBody3dDoc::saveSplitTask()
{
  uint64_t bodyId = getBodyManager().getSingleBodyId();
  if (bodyId > 0) {
    ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriterFromUrl(
          GET_FLYEM_CONFIG.getTaskServer());
    if (writer != NULL) {
      ZJsonArray seedJson = flyem::GetSeedJson(this);

      ZDvidUrl dvidUrl(getDvidTarget());
      QString taskKey = dvidUrl.getSplitTaskKey(bodyId).c_str();
      if (seedJson.isEmpty()) {
        if (writer->getDvidReader().hasSplitTask(taskKey)) {
          writer->deleteSplitTask(taskKey);
          std::cout << "Split task deleted: " << taskKey.toStdString() << std::endl;
        }
      } else {
        ZJsonArray roiJson;
        ZJsonObject task = flyem::MakeSplitTask(
              getDvidTarget(), bodyId, seedJson, roiJson);

        //          std::string bodyUrl = dvidUrl.getSparsevolUrl(bodyId);
        //          task.setEntry("signal", bodyUrl);

        //          task.setEntry("seeds", seedJson);

        std::string location = writer->writeServiceTask("split", task);

        //Save the entry point
        ZJsonObject taskJson;
        taskJson.setEntry(neutu::json::REF_KEY, location);
        taskJson.setEntry("user", neutu::GetCurrentUserName());
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
        ZStackObject::EType::FLYEM_TODO_ITEM);
  if (!objSet.empty()) {
    item = const_cast<ZFlyEmToDoItem*>(
          dynamic_cast<const ZFlyEmToDoItem*>(*(objSet.begin())));
  }

  return item;
}

QMap<uint64_t, ZFlyEmBodyEvent> ZFlyEmBody3dDoc::makeEventMapUnsync(
    ZFlyEmBodyManager *bm)
{
//  QSet<uint64_t> bodySet = m_bodySet;
  QMap<uint64_t, ZFlyEmBodyEvent> actionMap;
  for (QQueue<ZFlyEmBodyEvent>::const_iterator iter = m_eventQueue.begin();
       iter != m_eventQueue.end(); ++iter) {
    const ZFlyEmBodyEvent &event = *iter;
    uint64_t bodyId = event.getBodyId();
    if (actionMap.contains(bodyId)) {
      actionMap[bodyId].mergeEvent(event, neutu::EBiDirection::BACKWARD);
    } else {
      actionMap[bodyId] = event;
    }
  }

  for (QMap<uint64_t, ZFlyEmBodyEvent>::iterator iter = actionMap.begin();
       iter != actionMap.end(); ++iter) {
    ZFlyEmBodyEvent &event = iter.value();
    switch (event.getAction()) {
    case ZFlyEmBodyEvent::EAction::ADD:
      if (getBodyManager().contains(event.getBodyId())) {
        event.setAction(ZFlyEmBodyEvent::EAction::UPDATE);
      } else {
        if (bm != NULL) {
          bm->registerBody(event.getBodyId());
        }
//        bodySet.insert(event.getBodyId());
      }
      break;
    case ZFlyEmBodyEvent::EAction::FORCE_ADD:
      event.setAction(ZFlyEmBodyEvent::EAction::UPDATE);
      if (bm != NULL) {
        bm->registerBody(event.getBodyId());
      }
//      bodySet.insert(event.getBodyId());
      break;
    case ZFlyEmBodyEvent::EAction::REMOVE:
      if (getBodyManager().contains(event.getBodyId())) {
//        bodySet.remove(event.getBodyId());
        if (bm != NULL) {
          bm->deregisterBody(event.getBodyId());
        }
      } else {
        event.setAction(ZFlyEmBodyEvent::EAction::NONE);
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


QMap<uint64_t, ZFlyEmBodyEvent> ZFlyEmBody3dDoc::makeEventMap(
    bool synced, ZFlyEmBodyManager *bm)
{
  if (synced) {
    std::cout << "Locking process event" << std::endl;
    QMutexLocker locker(&m_eventQueueMutex);

    std::cout << "Making event map" << std::endl;
    return makeEventMapUnsync(bm);
  }

  return makeEventMapUnsync(bm);
}

void ZFlyEmBody3dDoc::cancelEventThread()
{
  m_quitting = true;
  m_futureMap.waitForFinished();
  m_quitting = false;
}

void ZFlyEmBody3dDoc::processEventFunc()
{
  QMap<uint64_t, ZFlyEmBodyEvent> actionMap = makeEventMap(
        true, &getBodyManager());
  if (!actionMap.isEmpty()) {
    std::cout << "====Processing Event====" << std::endl;
    for (QMap<uint64_t, ZFlyEmBodyEvent>::const_iterator iter = actionMap.begin();
         iter != actionMap.end(); ++iter) {
      const ZFlyEmBodyEvent &event = iter.value();
      event.print();
    }
  }

  //Process removing events first
  for (QMap<uint64_t, ZFlyEmBodyEvent>::const_iterator iter = actionMap.begin();
       iter != actionMap.end(); ++iter) {
    const ZFlyEmBodyEvent &event = iter.value();
    if (event.getAction() == ZFlyEmBodyEvent::EAction::REMOVE) {
      processEventFunc(event);
    }
    if (m_quitting) {
      break;
    }
  }

  //Process other events
  for (QMap<uint64_t, ZFlyEmBodyEvent>::const_iterator iter = actionMap.begin();
       iter != actionMap.end(); ++iter) {
    const ZFlyEmBodyEvent &event = iter.value();
    if (event.getAction() != ZFlyEmBodyEvent::EAction::REMOVE) {
      processEventFunc(event);
    }
    if (m_quitting) {
      break;
    }
  }


//  emit messageGenerated(ZWidgetMessage("3D Body view updated."));
  std::cout << "====Processing done====" << std::endl;
}

neutu::EBodyLabelType ZFlyEmBody3dDoc::getBodyLabelType(uint64_t bodyId) const
{
  if (getDvidTarget().hasSupervoxel() &&
      ZFlyEmBodyManager::encodingSupervoxel(bodyId)) {
    return neutu::EBodyLabelType::SUPERVOXEL;
  }

  return neutu::EBodyLabelType::BODY;
}

bool ZFlyEmBody3dDoc::IsOverSize(const ZStackObject *obj)
{
  return ZStackObjectHelper::IsOverSize(*obj);
}

int ZFlyEmBody3dDoc::getCoarseBodyZoom() const
{
  return zgeom::GetZoomLevel(getDvidInfo().getBlockSize().getX());
  /*
  int s = getDvidInfo().getBlockSize().getX();
  int zoom = 0;
  while (s /= 2) {
    ++zoom;
  }

  return zoom;
  */
}

ZStackObject::EType ZFlyEmBody3dDoc::getBodyObjectType() const
{
  if (getBodyType() == flyem::EBodyType::MESH) {
    return ZStackObject::EType::MESH;
  }

  return ZStackObject::EType::SWC;
}

const ZFlyEmBodyManager& ZFlyEmBody3dDoc::getBodyManager() const
{
  return m_bodyManager;
}

ZFlyEmBodyManager& ZFlyEmBody3dDoc::getBodyManager()
{
  return m_bodyManager;
}

uint64_t ZFlyEmBody3dDoc::getSelectedSingleNormalBodyId() const
{
  uint64_t bodyId = 0;
  auto selectedObjectList = getSelectedObjectList<ZMesh>(
        ZStackObject::EType::MESH);
  if (selectedObjectList.size() == 1) {
    ZMesh *mesh = selectedObjectList[0];
    if (!isSupervoxel(mesh)) {
      bodyId = getBodyId(mesh);
    }
  }

  return bodyId;
}

uint64_t ZFlyEmBody3dDoc::getSingleBody() const
{
  QMutexLocker locker(&m_BodySetMutex);

  return getBodyManager().getSingleBodyId();
/*
  uint64_t bodyId = 0;
  if (m_bodySet.size() == 1) {
    bodyId = *(m_bodySet.begin());
  }

  return bodyId;
  */
}

void ZFlyEmBody3dDoc::SetObjectClass(ZStackObject *obj, uint64_t bodyId)
{
  if (obj != NULL) {
    obj->setObjectClass(ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId));
  }
}

void ZFlyEmBody3dDoc::registerBody(uint64_t bodyId)
{
  getBodyManager().registerBody(bodyId);
}

void ZFlyEmBody3dDoc::deregisterBody(uint64_t bodyId)
{
  getBodyManager().deregisterBody(bodyId);
}

void ZFlyEmBody3dDoc::addBodyConfig(const ZFlyEmBodyConfig &config)
{
  getBodyManager().addBodyConfig(config);
}

neutu::EBodyLabelType ZFlyEmBody3dDoc::getLabelType(uint64_t bodyId) const
{
  if (getBodyManager().isSupervoxel(bodyId)) {
    return neutu::EBodyLabelType::SUPERVOXEL;
  }

  return neutu::EBodyLabelType::BODY;
}

void ZFlyEmBody3dDoc::showAllMesh()
{
  QList<ZMesh*> meshList = ZStackDocProxy::GetGeneralMeshList(this);
  for (ZMesh *mesh : meshList) {
    if (!mesh->isVisible()) {
      mesh->setVisible(true);
      bufferObjectVisibilityChanged(mesh);
    }
  }
  processObjectModified();
}

void ZFlyEmBody3dDoc::hideNoSplitMesh()
{
  QList<ZMesh*> meshList = ZStackDocProxy::GetGeneralMeshList(this);
  ZMesh* splitMesh = getMeshForSplit();
  for (ZMesh *mesh : meshList) {
    if (mesh != splitMesh) {
      mesh->setVisible(false);
      bufferObjectVisibilityChanged(mesh);
    }
  }
  processObjectModified();
}

ZMesh* ZFlyEmBody3dDoc::getMeshForSplit() const
{
  ZMesh *meshForSplit = nullptr;

  QList<ZMesh*> meshList = ZStackDocProxy::GetGeneralMeshList(this);
  if (!isSplitActivated()) {
    if (!meshList.isEmpty()) {
      if (meshList.size() == 1) {
        meshForSplit = meshList.front();
      } else {
        for (ZMesh *mesh : meshList) {
          if (mesh->isSelected()) {
            if (meshForSplit == nullptr) {
              meshForSplit = mesh;
            } else { //more than one selected
              meshForSplit = nullptr;
              break;
            }
          }
        }
      }
    }
  } else {
    uint64_t bodyId = m_splitter->getBodyId();
    if (m_splitter->getLabelType() == neutu::EBodyLabelType::SUPERVOXEL &&
        !m_splitter->fromTar()) {
      bodyId = ZFlyEmBodyManager::EncodeSupervoxel(bodyId);
    }
    for (ZMesh *mesh : meshList) {
      if (mesh->getLabel() == bodyId) {
        meshForSplit = mesh;
        break;
      }
    }
  }

  return meshForSplit;
}

ZMesh* ZFlyEmBody3dDoc::readSupervoxelMesh(
    const ZDvidReader &reader, uint64_t svId) const
{
  ZMesh *mesh = reader.readSupervoxelMesh(ZFlyEmBodyManager::decode(svId));

  return mesh;
}

/*
void ZFlyEmBody3dDoc::activateSplit(uint64_t bodyId, bool fromTar)
{
  activateSplit(bodyId, getLabelType(bodyId), fromTar);
}
*/

void ZFlyEmBody3dDoc::activateSplit(uint64_t bodyId)
{
  activateSplit(bodyId, getLabelType(bodyId), m_splitter->fromTar());
}

void ZFlyEmBody3dDoc::activateSplit(
    uint64_t bodyId, neutu::EBodyLabelType type, bool fromTar)
{
  if (!isSplitActivated() && isDvidMutable()) {
    bodyId = decode(bodyId);
    uint64_t parentId = bodyId;
    if (type == neutu::EBodyLabelType::SUPERVOXEL) {
      parentId = getMainDvidReader().readParentBodyId(bodyId);
    }

    if (getDataDocument()->checkOutBody(parentId, neutu::EBodySplitMode::ONLINE)) {
      m_splitter->setBody(bodyId, type, fromTar);

//      hideNoSplitMesh();

      QString msg = "Split activated for ";
      if (type == neutu::EBodyLabelType::SUPERVOXEL) {
        msg += "supervoxel ";
      }

      msg += QString("%1").arg(bodyId);

      emit messageGenerated(
            ZWidgetMessage(
              msg, neutu::EMessageType::INFORMATION,
              ZWidgetMessage::TARGET_CUSTOM_AREA |
              ZWidgetMessage::TARGET_KAFKA));

      flyem::LogBodyOperation("start split", bodyId, type);

      emit interactionStateChanged();
    } else {
      notifyWindowMessageUpdated("Failed to lock the body for split.");
    }
  }
}

void ZFlyEmBody3dDoc::activateSplitForSelected()
{
  TStackObjectSet objSet = getSelected(ZStackObject::EType::MESH);
  if (objSet.size() == 1) {
    ZStackObject *obj = *(objSet.begin());
    /*
    neutu::EBodyLabelType labelType = flyem::LABEL_BODY;
    if (getDvidTarget().hasSupervoxel()) {
      if (getMappedId(obj->getLabel()) != obj->getLabel()) {
        labelType = flyem::LABEL_SUPERVOXEL;
      }
    }
    */

    bool fromTar = false;
    if (obj->hasRole(ZStackObjectRole::ROLE_SUPERVOXEL)) {
      fromTar = true;
    }
    activateSplit(obj->getLabel(), getLabelType(obj->getLabel()), fromTar);
  }
}

void ZFlyEmBody3dDoc::deactivateSplit()
{
  if (m_splitter->getBodyId() > 0) {
    waitForSplitToBeDone();

//    if (m_taskConfig.getTaskType() == ProtocolTaskFactory::TASK_BODY_CLEAVE) {
    if (m_splitter->getLabelType() == neutu::EBodyLabelType::SUPERVOXEL) {
      removeObject(ZStackObjectRole::ROLE_SEGMENTATION, true);
      ZStackDocAccessor::RemoveSplitSeed(this);
    }
//    }

    uint64_t parentId = m_splitter->getBodyId();

    emitInfo(QString("Exit split for %1").arg(parentId));
    flyem::LogBodyOperation("exit split", parentId, m_splitter->getLabelType());

    if (m_splitter->getLabelType() == neutu::EBodyLabelType::SUPERVOXEL) {
      parentId = getMainDvidReader().readParentBodyId(m_splitter->getBodyId());
    }

    flyem::LogBodyOperation("unlock", parentId, neutu::EBodyLabelType::BODY);
    getDataDocument()->checkInBodyWithMessage(
          parentId, neutu::EBodySplitMode::ONLINE);

    m_splitter->setBodyId(0);

    QAction *action = m_actionLibrary->getAction(
          ZActionFactory::ACTION_SHOW_SPLIT_MESH_ONLY);
    if (action) {
      action->setChecked(false);
    }

    emit interactionStateChanged();
  }
}


void ZFlyEmBody3dDoc::showMeshForSplitOnly(bool on)
{
  KINFO << QString("Toggle meshes for split: %1").arg(on);

  if (on) {
    hideNoSplitMesh();
  } else {
    showAllMesh();
  }
}

ZMesh* ZFlyEmBody3dDoc::getRoiMesh(const QString &name) const
{
  return getObject<ZMesh>(
        ZStackObjectSourceFactory::MakeFlyEmRoiSource(name.toStdString()));

}

void ZFlyEmBody3dDoc::updateRoiMeshList(
    const QList<QString> &nameList, const QList<bool> &visibleList,
    const QList<QColor> &colorList)
{
  beginObjectModifiedMode(EObjectModifiedMode::CACHE);

  for (int i = 0; i < nameList.size(); ++i) {
    updateRoiMesh(nameList[i], visibleList[i], colorList[i]);
  }

  endObjectModifiedMode();

  processObjectModified();
}

void ZFlyEmBody3dDoc::updateRoiMesh(
    const QString &name, bool visible, const QColor &color)
{
  ZMesh *mesh = getRoiMesh(name);

  if (mesh == nullptr) {
    mesh = getDataDocument()->makeRoiMesh(name);
    if (mesh) {
      mesh->setVisible(visible);
      if (color != mesh->getColor()) {
        mesh->pushObjectColor(color);
      }
      addObject(mesh);
    }
  } else if (mesh) {
    mesh->setVisible(visible);
    if (color != mesh->getColor()) {
      mesh->pushObjectColor(color);
    }
    processObjectModified(mesh);
  }
}

/*
void ZFlyEmBody3dDoc::updateRoiMesh(const QString &name)
{
  ZMesh *mesh = getRoiMesh(name);

  if (mesh == nullptr) {
    mesh = getDataDocument()->makeRoiMesh(name);
    if (mesh) {
      addObject(mesh);
    }
  }
}
*/

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
      case ZActionFactory::ACTION_BODY_ANNOTATION:
        connect(action, SIGNAL(triggered()),
                this, SLOT(startBodyAnnotation()));
        break;
      case ZActionFactory::ACTION_SHOW_SPLIT_MESH_ONLY:
        connect(action, &QAction::toggled,
                this, &ZFlyEmBody3dDoc::showMeshForSplitOnly);
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
  return (m_splitter->getState() == ZFlyEmBodySplitter::EState::STATE_FULL_SPLIT) ||
      (m_splitter->getState() == ZFlyEmBodySplitter::EState::STATE_SPLIT);
}

bool ZFlyEmBody3dDoc::protectBody(uint64_t bodyId)
{
  if (bodyId > 0) {
    QMutexLocker locker(&m_eventQueueMutex);
    QMap<uint64_t, ZFlyEmBodyEvent> actionMap = makeEventMap(false, NULL);
    if (actionMap.contains(bodyId)) {
      if (actionMap[bodyId].getAction() == ZFlyEmBodyEvent::EAction::REMOVE) {
        //Cannot protected a body to be removed
        return false;
      }
    }

    m_protectedBodySet.insert(bodyId);
  }

  return true;
}

void ZFlyEmBody3dDoc::releaseBody(
    uint64_t bodyId, neutu::EBodyLabelType labelType)
{
  QMutexLocker locker(&m_eventQueueMutex);

  if (labelType == neutu::EBodyLabelType::SUPERVOXEL) {
    bodyId = ZFlyEmBodyManager::EncodeSupervoxel(bodyId);
  }
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
    bodyId = getBodyManager().getSingleBodyId();
    m_splitter->setFromTar(false);
  }

  if (!protectBody(bodyId)) {
    bodyId = 0;
  }

  return bodyId;
}

ZDvidSparseStack* ZFlyEmBody3dDoc::loadDvidSparseStack(
    uint64_t bodyId, neutu::EBodyLabelType type)
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
    body = getBodyReader().readDvidSparseStackAsync(bodyId, type);
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
#if 0
  auto updateBodyModelSelectionFunc = [this]() {
    QMutexLocker locker(this->getObjectGroup().getMutex());
    auto objList = this->getObjectGroup().getObjectListUnsync(
          ZStackObject::EType::SWC);
    auto meshList = this->getObjectGroup().getObjectListUnsync(
          ZStackObject::EType::MESH);

    objList.append(meshList);

    for (ZStackObject *obj : objList) {
      if (m_selectedBodySet.contains(obj->getLabel())) {
        if (!obj->isSelected()) {
          setSelected(obj, true);
        }
      }
    }

    foreach (ZSwcTree *tree, swcList) {
      if (m_selectedBodySet.contains(tree->getLabel())) {
        if (!tree->isSelected()) {
          getDataBuffer()->addUpdate(tree, ZStackDocObjectUpdate::EAction::SELECT);
        }
      } else if (tree->isSelected()) {
        getDataBuffer()->addUpdate(tree, ZStackDocObjectUpdate::EAction::DESELECT);
      }
    }

    QList<ZMesh*> meshList = getMeshList();
    foreach (ZMesh *mesh, meshList) {
      if (m_selectedBodySet.contains(mesh->getLabel())) {
        if (!mesh->isSelected()) {
          getDataBuffer()->addUpdate(mesh, ZStackDocObjectUpdate::EAction::SELECT);
        }
      } else if (mesh->isSelected()) {
        getDataBuffer()->addUpdate(mesh, ZStackDocObjectUpdate::EAction::DESELECT);
      }
    }
  };
#endif

  QList<ZSwcTree*> swcList = getSwcList();
  foreach (ZSwcTree *tree, swcList) {
    if (m_selectedBodySet.contains(tree->getLabel())) {
      if (!tree->isSelected()) {
        getDataBuffer()->addUpdate(tree, ZStackDocObjectUpdate::EAction::SELECT);
      }
    } else if (tree->isSelected()) {
      getDataBuffer()->addUpdate(tree, ZStackDocObjectUpdate::EAction::DESELECT);
    }
  }

  QList<ZMesh*> meshList = getMeshList();
  foreach (ZMesh *mesh, meshList) {
    if (m_selectedBodySet.contains(mesh->getLabel())) {
      if (!mesh->isSelected()) {
        getDataBuffer()->addUpdate(mesh, ZStackDocObjectUpdate::EAction::SELECT);
      }
    } else if (mesh->isSelected()) {
      getDataBuffer()->addUpdate(mesh, ZStackDocObjectUpdate::EAction::DESELECT);
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

/*
bool ZFlyEmBody3dDoc::hasBody(uint64_t bodyId, bool encoded) const
{
  QMutexLocker locker(&m_BodySetMutex);

  if (!encoded) {
    return getUnencodedBodySet().contains(bodyId);
  }

  return getBodyManager().contains(bodyId);
}
*/

void ZFlyEmBody3dDoc::addBody(const ZFlyEmBodyConfig &config)
{
  QMutexLocker locker(&m_BodySetMutex);
  uint64_t bodyId = config.getBodyId();
  if (!getBodyManager().contains(bodyId)) {
    if (!config.getAddBuffer()) {
      getBodyManager().registerBody(bodyId);
    }

    ZFlyEmBodyConfig newConfig = config;

    if (getBodyType() == flyem::EBodyType::SKELETON) {
      newConfig.setDsLevel(-1);
    } else {
      newConfig.setDsLevel(m_maxDsLevel);
    }

    addBodyFunc(newConfig);
  }
}

void ZFlyEmBody3dDoc::updateBody(ZFlyEmBodyConfig &config)
{
  bool updated = false;

  uint64_t bodyId = config.getBodyId();

  beginObjectModifiedMode(ZStackDoc::EObjectModifiedMode::CACHE);
  if (config.getBodyType() != flyem::EBodyType::MESH) {
    ZSwcTree *tree = getBodyModel(
          bodyId, config.getDsLevel(), config.getBodyType());
    if (tree != NULL) {
      if (tree->getColor() != config.getBodyColor()) {
        tree->setColor(config.getBodyColor());
        processObjectModified(tree);
        updated = true;
      }
    }
  } else {
    // A mesh from a tar archive is never needs its color updated, and checking
    // for it is a slow loop that is quadratic in the number of meshes.

    if (!fromTar(bodyId)) {
      ZMesh *mesh = getBodyMesh(bodyId, config.getDsLevel());
      if (mesh != NULL) {
        if (mesh->getColor() != config.getBodyColor()) {
          mesh->setColor(config.getBodyColor());
          mesh->pushObjectColor();
          processObjectModified(mesh);
          updated = true;
        }
      }
    }
  }
  endObjectModifiedMode();
  processObjectModified();

  if (!updated) {
    addBodyFunc(config);
  }
}

void ZFlyEmBody3dDoc::setBodyType(flyem::EBodyType type)
{
  m_bodyType = type;
  switch (m_bodyType) {
  /*
  case flyem::BODY_COARSE:
    setTag(neutube::Document::FLYEM_BODY_3D_COARSE);
    setMaxDsLevel(MAX_RES_LEVEL);
    break;
    */
  case flyem::EBodyType::SPHERE:
    if (showingCoarseOnly()) {
      setTag(neutu::Document::ETag::FLYEM_BODY_3D_COARSE);
    } else {
      setTag(neutu::Document::ETag::FLYEM_BODY_3D);
//      setMaxDsLevel(m_coarseLevel);
//      setMaxDsLevel(std::max(MAX_RES_LEVEL, getCoarseBodyZoom()));
    }
//    setMaxResLevel(MAX_RES_LEVEL);
    break;
  case flyem::EBodyType::SKELETON:
    setTag(neutu::Document::ETag::FLYEM_SKELETON);
    setMaxDsLevel(0);
    break;
  case flyem::EBodyType::MESH:
    setTag(neutu::Document::ETag::FLYEM_MESH);
//    setMaxDsLevel(std::max(MAX_RES_LEVEL, getCoarseBodyZoom()));
//    setMaxResLevel(MAX_RES_LEVEL);
    break;
  case flyem::EBodyType::DEFAULT:
    break;
  }
}

void ZFlyEmBody3dDoc::updateBody(
    uint64_t bodyId, const QColor &color)
{
//  updateBody(bodyId, color, flyem::BODY_COARSE);
  updateBody(bodyId, color, flyem::EBodyType::SPHERE);
  updateBody(bodyId, color, flyem::EBodyType::SKELETON);
  updateBody(bodyId, color, flyem::EBodyType::MESH);
}

void ZFlyEmBody3dDoc::updateBody(
    uint64_t bodyId, const QColor &color, flyem::EBodyType type)
{
  beginObjectModifiedMode(ZStackDoc::EObjectModifiedMode::CACHE);
  if (type != flyem::EBodyType::MESH) {
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

void ZFlyEmBody3dDoc::addEvent(const ZFlyEmBodyEvent &event, QMutex *mutex)
{
  if (event.isValid()) {
    QMutexLocker locker(mutex);

    if (event.getAction() == ZFlyEmBodyEvent::EAction::REMOVE) {
      //When a body is removed, its associated objects will be removed as well.
      //Clear the undo queue to avoid potential crash
      undoStack()->clear();
    }

#ifdef _DEBUG_
    std::cout << "+++++Adding event" << std::endl;
    event.print();
#endif

    m_eventQueue.enqueue(event);
  }
}

void ZFlyEmBody3dDoc::addEvent(ZFlyEmBodyEvent::EAction action, uint64_t bodyId,
                               ZFlyEmBodyEvent::TUpdateFlag flag, QMutex *mutex)
{
  QMutexLocker locker(mutex);

  ZFlyEmBodyEvent event(action, bodyId);
  event.addUpdateFlag(flag);
  ZFlyEmBodyConfig config = getBodyManager().getBodyConfig(bodyId);
  if (config.getBodyId() > 0) {
    event.setBodyConfig(config);
  } else {
    if (event.getAction() == ZFlyEmBodyEvent::EAction::ADD &&
        getBodyType() != flyem::EBodyType::SKELETON) {
      if (ZFlyEmBodyManager::encodesTar(bodyId)) {
        event.setDsLevel(0);
      } else {
        event.setDsLevel(getMaxDsLevel());
      }
    }
  }

  event.setBodyColor(getBodyColor(bodyId));

  addEvent(event);
//  m_eventQueue.enqueue(event);
}

ZSwcTree* ZFlyEmBody3dDoc::getBodyQuickly(uint64_t bodyId)
{
  ZSwcTree *tree = NULL;

  if (getBodyType() == flyem::EBodyType::SPHERE) {
    tree = recoverFullBodyFromGarbage(bodyId, getMaxDsLevel());
  }

  if (tree == NULL) {
    tree = makeBodyModel(bodyId, getCoarseBodyZoom(), getBodyType());
  }

  return tree;
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

void ZFlyEmBody3dDoc::addBodyMeshFunc(ZFlyEmBodyConfig &config)
{
  uint64_t bodyId = config.getBodyId();

  notifyBodyUpdate(bodyId, config.getDsLevel());

//  std::map<uint64_t, ZMesh*> meshes;
  std::vector<ZMesh *> meshes = makeBodyMeshModels(config);

//  bool loaded =
//      !(getObjectGroup().findSameClass(
//          ZStackObject::EType::TYPE_SWC,
//          ZStackObjectSourceFactory::MakeFlyEmBodySource(config.getBodyId())).
//        isEmpty());

  if (config.getLabelType() == neutu::EBodyLabelType::BODY) {
    loadSynapseFresh(bodyId);
    loadTodoFresh(bodyId);
  }
  /*
  if (!loaded) {
    addSynapse(ZFlyEmBodyManager::decode(id));
    updateTodo(ZFlyEmBodyManager::decode(id));
  }
  */

  for (ZMesh *mesh : meshes) {
    mesh->setColor(config.getBodyColor());
    mesh->pushObjectColor();
  }

  updateMeshFunc(config, meshes);

  if (config.isTar()) {
    QSet<uint64_t> subbodySet;
    for (const ZMesh *mesh : meshes) {
//      if (ZFlyEmBodyManager::decode(mesh->getLabel()) !=
//          ZFlyEmBodyManager::decode(config.getBodyId())) {
      if (mesh->hasRole(ZStackObjectRole::ROLE_SUPERVOXEL)) {
        subbodySet.insert(mesh->getLabel());
      }
//      }
    }
    if (!config.getAddBuffer()) {
      getBodyManager().registerBody(config.getBodyId(), subbodySet);
    } else {
      getBodyManager().registerBufferedBody(config.getBodyId(), subbodySet);
    }
  } else {
    getBodyManager().registerBody(config.getBodyId());
  }

  notifyBodyUpdated(config.getBodyId(), config.getDsLevel());

  if (config.isTar() && !config.getAddBuffer()) {
    // Meshes loaded from an archive are ready at this point, so emit a signal, which
    // can be used by code that needs to know the IDs of the loaded meshes (instead of
    // the ID of the archive). But only if the meshes are not being buffered (for
    // prefetching).
    LDEBUG() << "Emitting bodyMeshesAdded";
    emit bodyMeshesAdded(meshes.size());
  }

  if (config.getAddBuffer()) {
    QString msg = QString("Done prefetching body %1").arg(config.getBodyId());
    emit messageGenerated(ZWidgetMessage(msg));
  }

#if 0
  for (auto it : meshes) {
//    emit messageGenerated(ZWidgetMessage("3D Body view synced"));

    uint64_t bodyId = it.first;
    ZMesh *mesh = it.second;

    if (mesh != NULL) {
      resLevel = ZStackObjectSourceFactory::ExtractZoomFromFlyEmBodySource(
            mesh->getSource());
      config.setDsLevel(resLevel);
    }

    if (mesh != NULL) {
  #ifdef _DEBUG_
      std::cout << "Adding object: " << dynamic_cast<ZStackObject*>(mesh) << std::endl;
      std::cout << "Color count: " << mesh->colors().size() << std::endl;
      std::cout << "Vertex count: " << mesh->vertices().size() << std::endl;
  #endif
      mesh->setColor(config.getBodyColor());
      mesh->pushObjectColor();

      // The findSameClass() function has performance that iis quadratic in the number of meshes,
      // and is unnecessary for meshes from a tar archive.

      bool loaded = fromTar(id);
      if (!loaded) {
        loaded =
          !(getObjectGroup().findSameClass(
              ZStackObject::EType::TYPE_MESH,
              ZStackObjectSourceFactory::MakeFlyEmBodySource(mesh->getLabel())).
            isEmpty());
      }

//      updateBodyFunc(bodyId, mesh);

#if defined(_NEU3_)
      if (!loaded) {
        // TODO: As of December, 2017, the following is slow due to access of a desktop server,
        // http://zhaot-ws1:9000.  This server should be replaced with a faster one.
        // The problem is most noticeable for the functionality of taskbodyhistory.cpp.
        loadSplitTask(bodyId);
      }
#endif

      // If the argument ID loads an archive, then makeBodyMeshModels() can create
      // multiple meshes whose IDs need to be recorded, to make operations like
      // selection work correctly.

      getBodyManager().registerBody(mesh->getLabel());
    }
  }

  notifyBodyUpdated(id, resLevel);

  if (ZFlyEmBodyManager::encodesTar(id)) {

    // Meshes loaded from an archive are ready at this point, so emit a signal, which
    // can be used by code that needs to know the IDs of the loaded meshes (instead of
    // the ID of the archive).
    LDEBUG() << "Emitting bodyMeshesAdded";
    emit bodyMeshesAdded(meshes.size());
  }
#endif
}

bool ZFlyEmBody3dDoc::toBeRemoved(uint64_t bodyId) const
{
  bool removing = false;

  for (QQueue<ZFlyEmBodyEvent>::const_iterator iter = m_eventQueue.begin();
       iter != m_eventQueue.end(); ++iter) {
    const ZFlyEmBodyEvent &event = *iter;
    if (event.getBodyId() == bodyId) {
      if (event.getAction() == ZFlyEmBodyEvent::EAction::REMOVE) {
        removing = true;
      } else {
        removing = false;
      }
    }
  }

  return removing;
}

bool ZFlyEmBody3dDoc::isSupervoxel(const ZStackObject *obj) const
{
  if (obj != NULL) {
    if (getBodyManager().isSupervoxel(getBodyId(obj))) {
      return true;
    }
  }

  return false;
}

bool ZFlyEmBody3dDoc::isSupervoxel(uint64_t bodyId)
{
  return getBodyManager().isSupervoxel(bodyId);
}

void ZFlyEmBody3dDoc::cacheSupervoxelSize(std::vector<uint64_t> svIdArray) const
{
  QMutexLocker locker(&m_supervoxelSizeCacheMutex);
  std::vector<uint64_t> sizeToUpdate;
  for (uint64_t bodyId : svIdArray) {
    if (!m_supervoxelSizeCache.contains(bodyId)) {
      sizeToUpdate.push_back(bodyId);
    }
  }

  if (!sizeToUpdate.empty()) {
    std::vector<size_t> bodySize = getMainDvidReader().readBodySize(
          sizeToUpdate, neutu::EBodyLabelType::SUPERVOXEL);
    if (sizeToUpdate.size() == bodySize.size()) {
      for (size_t i = 0; i < sizeToUpdate.size(); ++i) {
        m_supervoxelSizeCache[sizeToUpdate[i]] = bodySize[i];
      }
    }
  }
}

size_t ZFlyEmBody3dDoc::getSupervoxelSize(uint64_t svId) const
{
  QMutexLocker locker(&m_supervoxelSizeCacheMutex);
  auto iter = m_supervoxelSizeCache.find(svId);
  size_t svSize = 0;
  if (iter == m_supervoxelSizeCache.end()) {
    svSize = getMainDvidReader().readBodySize(
          svId, neutu::EBodyLabelType::SUPERVOXEL);
    m_supervoxelSizeCache[svId] = svSize;
  } else {
    svSize = iter.value();
  }

  return svSize;
}

void ZFlyEmBody3dDoc::invalidateSupervoxelCache(uint64_t svId)
{
  QMutexLocker locker(&m_supervoxelSizeCacheMutex);
  m_supervoxelSizeCache.remove(svId);
}

void ZFlyEmBody3dDoc::invalidateBodyCache(uint64_t bodyId)
{
  setUnrecycable(QSet<uint64_t>({bodyId}));
}

template<typename InputIterator>
void ZFlyEmBody3dDoc::invalidateBodyCache(InputIterator first, InputIterator last)
{
  for (InputIterator iter = first; iter != last; ++iter) {
    invalidateBodyCache(*iter);
  }
}

void ZFlyEmBody3dDoc::invalidateSelectedBodyCache()
{
  std::set<uint64_t> bodySet =
      getDataDocument()->getSelectedBodySet(neutu::ELabelSource::ORIGINAL);
  invalidateBodyCache(bodySet.begin(), bodySet.end());
}

uint64_t ZFlyEmBody3dDoc::getBodyId(const ZStackObject *obj) const
{
  if (obj->getType() == ZStackObject::EType::MESH ||
      obj->getType() == ZStackObject::EType::SWC ||
      obj->getType() == ZStackObject::EType::OBJECT3D_SCAN) {
    return obj->getLabel();
  }

  return 0;
}

void ZFlyEmBody3dDoc::loadSynapseFresh(uint64_t bodyId)
{
  if (!getBodyManager().isSynapseLoaded(bodyId)) {
    addSynapse(bodyId);
    getBodyManager().setSynapseLoaded(bodyId);
  }
}

void ZFlyEmBody3dDoc::loadTodoFresh(uint64_t bodyId)
{
  if (!getBodyManager().isTodoLoaded(bodyId)) {
    updateTodo(bodyId);
    getBodyManager().setTodoLoaded(bodyId);
  }
}

FlyEmBodyAnnotationDialog *ZFlyEmBody3dDoc::getBodyAnnotationDlg()
{
  if (m_annotationDlg == nullptr) {
    m_annotationDlg = FlyEmDialogFactory::MakeBodyAnnotationDialog(
          getDataDocument(), getParent3DWindow());
#if 0
    m_annotationDlg = new ZFlyEmBodyAnnotationDialog(getParent3DWindow());
    /*
    ZJsonArray statusJson = getMainDvidReader().readBodyStatusList();
    QList<QString> statusList;
    for (size_t i = 0; i < statusJson.size(); ++i) {
      std::string status = ZJsonParser::stringValue(statusJson.at(i));
      if (!status.empty() && ZFlyEmBodyStatus::IsAccessible(status)) {
        statusList.append(status.c_str());
      }
    }
    */
    QList<QString> statusList = getDataDocument()->getBodyStatusList();
    if (!statusList.empty()) {
      m_annotationDlg->setDefaultStatusList(statusList);
    } else {
      m_annotationDlg->setDefaultStatusList(flyem::GetDefaultBodyStatus());
    }

    for (const QString &status : getDataDocument()->getAdminBodyStatusList()) {
      m_annotationDlg->addAdminStatus(status);
    }
#endif
  }

  return m_annotationDlg;
}

void ZFlyEmBody3dDoc::addBodyFunc(ZFlyEmBodyConfig &config)
{
  flyem::EBodyType bodyType = config.getBodyType();
  if (bodyType == flyem::EBodyType::DEFAULT) {
    bodyType = getBodyType();
  }

  if (bodyType == flyem::EBodyType::MESH) {
    addBodyMeshFunc(config);
  } else {
    removeDiffBody();

    uint64_t bodyId = config.getBodyId();

//    bool loaded =
//        !(getObjectGroup().findSameClass(
//            ZStackObject::EType::TYPE_SWC,
//            ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId)).
//          isEmpty());

    ZSwcTree *tree = NULL;
    if (isCoarseLevel(config.getDsLevel())) { //Coarse body
      notifyBodyUpdate(bodyId, getMaxDsLevel());
      tree = getBodyQuickly(bodyId);
      config.setDsLevel(getMaxDsLevel());
//      notifyBodyUpdated(bodyId, getMaxDsLevel());
    } else {
      notifyBodyUpdate(bodyId, config.getDsLevel());
      tree = makeBodyModel(bodyId, config.getDsLevel(), bodyType);
    }

    if (tree != NULL) {
      if (ZStackObjectSourceFactory::ExtractBodyTypeFromFlyEmBodySource(
            tree->getSource()) == flyem::EBodyType::SPHERE) {
        int resLevel = ZStackObjectSourceFactory::ExtractZoomFromFlyEmBodySource(
              tree->getSource());
        config.setDsLevel(resLevel);
        if (ZStackObjectHelper::IsOverSize(*tree)) {
          config.disableNextDsLevel();
        }
      }
      notifyBodyUpdated(bodyId, config.getDsLevel());
    } else {
      notifyWindowMessageUpdated(
            QString("Failed to generate 3D model for %1 (scale=%2)")
            .arg(bodyId)
            .arg(config.getDsLevel()));
    }

#if 0
    if (resLevel > getMinDsLevel()) {
      QMutexLocker locker(&m_eventQueueMutex);
      if (!toBeRemoved(bodyId)) {
        ZFlyEmBodyEvent bodyEvent = makeHighResBodyEvent(config);
        m_eventQueue.enqueue(bodyEvent);
      }
    }
#endif

    if (tree != NULL) {
      tree->setStructrualMode(ZSwcTree::STRUCT_POINT_CLOUD);
      if (m_nodeSeeding) {
        tree->setType(0);
      }

#ifdef _DEBUG_
      std::cout << "Adding object: " << dynamic_cast<ZStackObject*>(tree) << std::endl;
#endif
      tree->setColor(config.getBodyColor());

      updateBodyFunc(bodyId, tree);

      loadSynapseFresh(bodyId);
      loadTodoFresh(bodyId);
//      if (!loaded) {
//        addSynapse(bodyId);
//        //      addTodo(bodyId);
//        updateTodo(bodyId);
//      }
    }
  }
}

void ZFlyEmBody3dDoc::addSynapse(bool on)
{
  if (on) {
    for (uint64_t bodyId : getNormalBodySet()) {
      addEvent(ZFlyEmBodyEvent::EAction::UPDATE, bodyId, ZFlyEmBodyEvent::UPDATE_ADD_SYNAPSE);
    }
  }
}

void ZFlyEmBody3dDoc::addTodo(bool on)
{
  if (on) {
    for (uint64_t bodyId : getNormalBodySet()) {
      addEvent(ZFlyEmBodyEvent::EAction::UPDATE, bodyId, ZFlyEmBodyEvent::UPDATE_ADD_TODO_ITEM);
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
  return getBodyManager().isSynapseLoaded(bodyId);
  /*
  return getObjectGroup().findFirstSameSource(
        ZStackObject::EType::TYPE_PUNCTUM,
        ZStackObjectSourceFactory::MakeFlyEmTBarSource(bodyId)) != NULL;
        */
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
    getDataBuffer()->addUpdate(
          punctum, ZStackDocObjectUpdate::EAction::ADD_NONUNIQUE);
  }
  if (!puncta.empty()) {
    getDataBuffer()->deliver();
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

    getBodyManager().setSynapseLoaded(bodyId);
  }
}

void ZFlyEmBody3dDoc::makeKeyProcessor()
{
  m_keyProcessor = new ZFlyEmBody3dDocKeyProcessor(this);
}

void ZFlyEmBody3dDoc::updateTodo(uint64_t bodyId)
{
  if (m_showingTodo) {
#ifdef _DEBUG_
    std::cout << "Updating todo\n";
#endif

    bodyId = decode(bodyId);

    std::string source = ZStackObjectSourceFactory::MakeTodoPunctaSource(bodyId);

    ZStackDocObjectUpdate *u = new ZStackDocObjectUpdate(
          nullptr, ZStackDocObjectUpdate::EAction::CALLBACK);
    u->setCallback([source, this](ZStackObject*) {
      this->removeObject(source, true);
    });
    getDataBuffer()->addUpdate(u);
#if 0
    getDataBuffer()->removeObjectUpdate([&](ZStackDocObjectUpdate *u) {
      if (u->getObject()) {
        return ((u->getAction() == ZStackDocObjectUpdate::EAction::ADD_NONUNIQUE) ||
            (u->getAction() == ZStackDocObjectUpdate::EAction::ADD_UNIQUE)) &&
            (u->getObject()->getSource() == source);
      }

      return true;
    });
    TStackObjectList objList = getObjectGroup().findSameSource(source);
    for (TStackObjectList::iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
#ifdef _DEBUG_2
        std::cout << "Todo to remove: " << *iter << std::endl;
#endif
      getDataBuffer()->addUpdate(*iter, ZStackDocObjectUpdate::EAction::KILL);
    }
#endif

    if (getBodyManager().contains(bodyId)) {
      std::vector<ZFlyEmToDoItem*> itemList =
          getDataDocument()->getTodoItem(bodyId);

      for (std::vector<ZFlyEmToDoItem*>::const_iterator iter = itemList.begin();
           iter != itemList.end(); ++iter) {
        ZFlyEmToDoItem *item = *iter;
        item->setRadius(30);
        item->setSource(source);
#ifdef _DEBUG_2
        std::cout << "Todo to add: " << item << item->getPosition().toString()
                  << " " << item->isChecked() << std::endl;
#endif
        getDataBuffer()->addUpdate(
              item, ZStackDocObjectUpdate::EAction::ADD_NONUNIQUE);
      }
    }

    getDataBuffer()->deliver();
  }
}

void ZFlyEmBody3dDoc::updateSynapse(uint64_t bodyId)
{
  if (m_showingSynapse) {
    ZOUT(LTRACE(), 5) << "Update todo";

    bodyId = decode(bodyId);

    getDataBuffer()->addUpdate([bodyId, this]() {
      this->removeObject(ZStackObjectSourceFactory::MakeFlyEmTBarSource(bodyId));
    });

    getDataBuffer()->addUpdate([bodyId, this]() {
      this->removeObject(ZStackObjectSourceFactory::MakeFlyEmPsdSource(bodyId));
    });

/*
    {
      TStackObjectList objList = getObjectGroup().findSameSource(
            ZStackObjectSourceFactory::MakeFlyEmTBarSource(bodyId));
      for (TStackObjectList::iterator iter = objList.begin();
           iter != objList.end(); ++iter) {
        getDataBuffer()->addUpdate(*iter, ZStackDocObjectUpdate::EAction::KILL);
      }
    }

    {
      TStackObjectList objList = getObjectGroup().findSameSource(
            ZStackObjectSourceFactory::MakeFlyEmPsdSource(bodyId));
      for (TStackObjectList::iterator iter = objList.begin();
           iter != objList.end(); ++iter) {
        getDataBuffer()->addUpdate(*iter, ZStackDocObjectUpdate::EAction::KILL);
      }
    }
    */

    getBodyManager().setSynapseLoaded(bodyId, false);
    getDataBuffer()->deliver();

    addSynapse(bodyId);
  }
}

void ZFlyEmBody3dDoc::addTodo(uint64_t bodyId)
{
  if (m_showingTodo) {
    ZOUT(LTRACE(), 5) << "Add todo items";

    std::string source = ZStackObjectSourceFactory::MakeTodoPunctaSource(bodyId);
    if (getObjectGroup().findFirstSameSource(
          ZStackObject::EType::FLYEM_TODO_ITEM, source) == NULL) {
      std::vector<ZFlyEmToDoItem*> itemList =
          getDataDocument()->getTodoItem(bodyId);

      for (std::vector<ZFlyEmToDoItem*>::const_iterator iter = itemList.begin();
           iter != itemList.end(); ++iter) {
        ZFlyEmToDoItem *item = *iter;
        item->setRadius(30);
        item->setSource(source);
        getDataBuffer()->addUpdate(
              item, ZStackDocObjectUpdate::EAction::ADD_NONUNIQUE);
      }
    }
    getDataBuffer()->deliver();
  }
}

void ZFlyEmBody3dDoc::updateSegmentation()
{
  if (getBodyType() == flyem::EBodyType::MESH) {
    ZOUT(LTRACE(), 5) << "Update segmentation";
    QList<ZStackObject*> oldObjList =
        getObjectList(ZStackObjectRole::ROLE_TMP_RESULT);
    getDataBuffer()->addUpdate(oldObjList, ZStackDocObjectUpdate::EAction::KILL);


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
                mesh, ZStackDocObjectUpdate::EAction::ADD_NONUNIQUE);
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
      flyem::LoadSplitTask(getDvidTarget(), bodyId);

  if (!seedList.isEmpty()) {
    getDataBuffer()->addUpdate(
          seedList, ZStackDocObjectUpdate::EAction::ADD_NONUNIQUE);
    getDataBuffer()->deliver();

//    ZStackDocAccessor::RemoveObject(
//          getDataDocument(), ZStackObjectRole::ROLE_SEED, false);
    //A dangerous way to share objects. Need object clone or shared pointer.
//    ZStackDocAccessor::AddObject(getDataDocument(), seedList);
  }
}

void ZFlyEmBody3dDoc::removeSplitTask(uint64_t bodyId)
{
  flyem::RemoveSplitTask(getDvidTarget(), bodyId);
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
    item.setKind(ZFlyEmToDoItem::EKind::KIND_NOTE);
    item.setUserName(neutu::GetCurrentUserName());
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
  ZFlyEmToDoItem item = FlyEmDataReader::ReadToDoItem(
        getMainDvidReader(), x, y, z);

  return item;
}

bool ZFlyEmBody3dDoc::addTodo(const ZFlyEmToDoItem &item, uint64_t bodyId)
{
  bool succ = false;
  if (item.isValid()) {
    m_mainDvidWriter.writeToDoItem(item);
    if (m_mainDvidWriter.isStatusOk()) {
      emitInfo(QString("Todo added at %1").
               arg(item.getPosition().toString().c_str()));
//      emit messageGenerated(ZWidgetMessage(
//                              ));
      if (m_mainDvidWriter.getDvidTarget().hasSupervoxel()) {
        uint64_t svId = m_mainDvidWriter.getDvidReader().readSupervoxelIdAt(
              item.getPosition());
        emitInfo(QString("Supervoxel ID: %1").arg(svId));
//        emit messageGenerated(ZWidgetMessage(
//                               QString("Supervoxel ID: %1").arg(svId)));
      }

      if (getBodyManager().contains(bodyId)) {
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
  std::vector<ZIntPoint> ptArray;

  QSet<uint64_t> bodySet;
  foreach (const ZFlyEmToDoItem &item, itemList) {
    addTodoSliently(item);
    if (item.getBodyId() > 0) {
      bodySet.insert(item.getBodyId());
    }
    ptArray.push_back(item.getPosition());
  }

  foreach (uint64_t bodyId, bodySet) {
    updateTodo(bodyId);
  }

  getDataDocument()->downloadTodo(ptArray);
}

void ZFlyEmBody3dDoc::removeTodo(const QList<ZFlyEmToDoItem> &itemList)
{
  std::vector<ZIntPoint> ptArray;

  QSet<uint64_t> bodySet;
  foreach (const ZFlyEmToDoItem &item, itemList) {
    removeTodoSliently(item);
    if (item.getBodyId() > 0) {
      bodySet.insert(item.getBodyId());
    }
    ptArray.push_back(item.getPosition());
  }

  foreach (uint64_t bodyId, bodySet) {
    updateTodo(bodyId);
  }

  getDataDocument()->downloadTodo(ptArray);
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
  item.setAction(neutu::EToDoAction::TO_SPLIT);
  addTodo(item, bodyId);
}

void ZFlyEmBody3dDoc::removeBody(uint64_t bodyId)
{
  QMutexLocker locker(&m_BodySetMutex);
  getBodyManager().deregisterBody(bodyId);
  removeBodyFunc(bodyId, true);
}

void ZFlyEmBody3dDoc::updateMeshFunc(
    ZFlyEmBodyConfig &config, const std::vector<ZMesh *> meshes)
{
  int numMeshes = 0;
  if (config.isTar()) {
    for (ZMesh *mesh : meshes) {
      ZStackDocObjectUpdate::EAction action =
          config.getAddBuffer() ? ZStackDocObjectUpdate::EAction::ADD_BUFFER :
                                  ZStackDocObjectUpdate::EAction::ADD_NONUNIQUE;
      getDataBuffer()->addUpdate(mesh, action);
    }
    config.setDsLevel(0);
    numMeshes = meshes.size();
  } else {
    if (meshes.size() == 1) {
      ZMesh *mesh = meshes[0];
      if (config.getAddBuffer()) {
        getDataBuffer()->addUpdate(mesh, ZStackDocObjectUpdate::EAction::ADD_BUFFER);
      } else {
        TStackObjectList objList = getObjectGroup().findSameClass(
              mesh->getType(),
              ZStackObjectSourceFactory::MakeFlyEmBodySource(config.getBodyId()));

        if (!objList.isEmpty()) {
          ZStackObject *obj = objList.front();
          if (obj) {
            mesh->setColor(obj->getColor());
          }
          mesh->pushObjectColor();
        }

        for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
             ++iter) {
          getDataBuffer()->addUpdate(*iter, ZStackDocObjectUpdate::EAction::RECYCLE);
        }

        getDataBuffer()->addUpdate(mesh, ZStackDocObjectUpdate::EAction::ADD_UNIQUE);
        int resLevel = ZStackObjectSourceFactory::ExtractZoomFromFlyEmBodySource(
              mesh->getSource());
        config.setDsLevel(resLevel);

        if (objList.isEmpty()) {
          numMeshes = 1;
        }
      }
    }
  }
  getDataBuffer()->deliver();

  if (!config.getAddBuffer()) {
    emit bodyMeshLoaded(numMeshes);
  }
}

void ZFlyEmBody3dDoc::updateBodyFunc(uint64_t bodyId, ZStackObject *bodyObject)
{
  ZOUT(LTRACE(), 5) << "Update body: " << bodyId;

//  QString threadId = QString("updateBody(%1)").arg(bodyId);
//  if (!m_futureMap.isAlive(threadId)) {

  // The findSameClass() function has performance that is quadratic in the number of meshes,
  // and is unnecessary for meshes from a tar archive.

  bool replacing = false;

  if (!fromTar(bodyId)) {
    TStackObjectList objList = getObjectGroup().findSameClass(
          bodyObject->getType(),
          ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId));

    for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
         ++iter) {
      getDataBuffer()->addUpdate(*iter, ZStackDocObjectUpdate::EAction::RECYCLE);
    }

    if (!objList.isEmpty()) {
      replacing = true;
    }
    getDataBuffer()->addUpdate(bodyObject, ZStackDocObjectUpdate::EAction::ADD_UNIQUE);
  } else {
    // The event name is a bit confusing, but "NONUNIQUE" means that ZStackDoc::addObject()
    // won't get into a quadratic loop checking that this body is unique, at test that is
    // not necessary for bodies from tar archives.

    getDataBuffer()->addUpdate(bodyObject, ZStackDocObjectUpdate::EAction::ADD_NONUNIQUE);
  }
  getDataBuffer()->deliver();

//  if (!replacing) {
//    emit bodyMeshLoaded();
//  }
//  }

  ZOUT(LTRACE(), 5) << "Body updated: " << bodyId;
}

bool ZFlyEmBody3dDoc::executeDeleteSwcNodeCommand()
{
  if (getTag() == neutu::Document::ETag::FLYEM_SKELETON) {
    return ZStackDoc::executeDeleteSwcNodeCommand();
  }

  return false;
}

bool ZFlyEmBody3dDoc::executeConnectSwcNodeCommand()
{
  if (getTag() == neutu::Document::ETag::FLYEM_SKELETON) {
    return ZStackDoc::executeConnectSwcNodeCommand();
  }

  return false;
}

bool ZFlyEmBody3dDoc::executeBreakSwcConnectionCommand()
{
  if (getTag() == neutu::Document::ETag::FLYEM_SKELETON) {
    return ZStackDoc::executeBreakSwcConnectionCommand();
  }

  return false;
}


void ZFlyEmBody3dDoc::executeAddTodoCommand(
    int x, int y, int z, bool checked, uint64_t bodyId)
{
  if (isDvidMutable()) {
    neutu::EToDoAction action = m_taskConfig.getDefaultTodo();
    if (action == neutu::EToDoAction::TO_SPLIT) {
      if (isSupervoxel(bodyId)) {
        action = neutu::EToDoAction::TO_SUPERVOXEL_SPLIT;
      }
    }

    executeAddTodoCommand( x, y, z, checked, action, bodyId);
  }
}

void ZFlyEmBody3dDoc::executeAddTodoCommand(
    int x, int y, int z, bool checked, neutu::EToDoAction action,
    uint64_t bodyId)
{
  if (isDvidMutable()) {
    ZFlyEmBody3dDocCommand::AddTodo *command =
        new ZFlyEmBody3dDocCommand::AddTodo(this);

    if (bodyId == 0) {
      bodyId = getMainDvidReader().readBodyIdAt(x, y, z);
    }

    if (getBodyManager().contains(bodyId)) {
      command->setTodoItem(x, y, z, checked, action, bodyId);
      if (command->hasValidItem()) {
        pushUndoCommand(command);
      } else {
        delete command;
      }
    } else {
      std::ostringstream stream;
      int count = 0;
      for (uint64_t bodyId : getNormalBodySet()) {
        if (count < 3) {
          stream << bodyId << ", ";
        } else {
          stream << "...";
          break;
        }
      }
      LDEBUG() << "Cannot add todo:" << bodyId << "not in" << stream.str();
    }
  }
}

void ZFlyEmBody3dDoc::removeTodo(ZFlyEmTodoFilterDialog *dlg)
{
  if (isDvidMutable()) {
    if (dlg->exec()) {
      ZFlyEmBody3dDocCommand::RemoveTodo *command =
          new ZFlyEmBody3dDocCommand::RemoveTodo(this);

      QList<ZFlyEmToDoItem*> todoList = getObjectList<ZFlyEmToDoItem>();
      for (const ZFlyEmToDoItem *item : todoList) {
        if (dlg->passed(item)) {
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
  }
}

void ZFlyEmBody3dDoc::executeRemoveTodoCommand()
{
  if (isDvidMutable()) {
    ZFlyEmBody3dDocCommand::RemoveTodo *command =
        new ZFlyEmBody3dDocCommand::RemoveTodo(this);
    const TStackObjectSet& objSet = getObjectGroup().getSelectedSet(
          ZStackObject::EType::FLYEM_TODO_ITEM);
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
}

namespace {

bool is_recycable(ZStackObject::EType type)
{
  return (type == ZStackObject::EType::MESH) ||
      (type == ZStackObject::EType::SWC);
}

bool is_recycable(const ZStackObject *obj)
{
  if (obj) {
    return is_recycable(obj->getType());
  }

  return false;
}

}

void ZFlyEmBody3dDoc::dumpObject(ZStackObject *obj, bool recycling)
{
  if (removeObject(obj, false)) { //Called to first to make sure that the object exists
    if (is_recycable(obj)) {
      dumpGarbage(obj, recycling);
      emit bodyRecycled(obj->getLabel());
    } else {
//      dumpGarbage(obj, false);
      delete obj;
    }
  }
}

void ZFlyEmBody3dDoc::recycleObject(ZStackObject *obj)
{
  dumpObject(obj, true);
}

void ZFlyEmBody3dDoc::killObject(ZStackObject *obj)
{
  dumpObject(obj, false);
}

void ZFlyEmBody3dDoc::removeBodyFunc(uint64_t bodyId, bool removingAnnotation)
{
  KINFO << QString("Remove body: %1").arg(bodyId);

  QString threadId = QString("removeBody(%1)").arg(bodyId);
  if (!m_futureMap.isAlive(threadId)) {
    //TStackObjectList objList = getObjectGroup().findSameSource(
        //  ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId));
    TStackObjectList objList = getObjectGroup().findSameClass(
          getBodyObjectType(),
          ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId));

//    QMutexLocker locker(&m_garbageMutex);
//    beginObjectModifiedMode(EObjectModifiedMode::OBJECT_MODIFIED_CACHE);
//    blockSignals(true);
    for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
         ++iter) {
//      removeObject(*iter, false);
//      dumpGarbageUnsync(*iter, true);
      KINFO << "Put " + (*iter)->getSource() + " in recycle";
      getDataBuffer()->addUpdate(*iter, ZStackDocObjectUpdate::EAction::RECYCLE);
    }

    if (!objList.isEmpty()) {
      emit bodyRemoved(bodyId);
    } else {
      KWARN << QString("No object found for %1").arg(bodyId);
#ifdef _DEBUG_
      KINFO << "Current sources:";

      TStackObjectList objList = getObjectGroup().getObjectList(
            getBodyObjectType());
      for (const ZStackObject *obj : objList) {
        KINFO << obj->getObjectClass() + " " + obj->getSource();
      }
#endif
    }

    if (removingAnnotation) {
      //Use RECYCLE instead of KILL until premature deletion is fixeed
      //for selection signals.
      objList = getObjectGroup().findSameSource(
            ZStackObjectSourceFactory::MakeTodoPunctaSource(bodyId));
      getDataBuffer()->addUpdate(objList, ZStackDocObjectUpdate::EAction::KILL);
      /*
      for (TStackObjectList::iterator iter = objList.begin();
           iter != objList.end(); ++iter) {
//        removeObject(*iter, false);
//        dumpGarbageUnsync(*iter, true);
        getDataBuffer()->addUpdate(*iter, ZStackDocObjectUpdate::EAction::ACTION_KILL);
      }
      */

      objList = getObjectGroup().findSameSource(
            ZStackObjectSourceFactory::MakeFlyEmTBarSource(bodyId));
      for (TStackObjectList::iterator iter = objList.begin();
           iter != objList.end(); ++iter) {
//        removeObject(*iter, false);
//        dumpGarbageUnsync(*iter, true);
        getDataBuffer()->addUpdate(*iter, ZStackDocObjectUpdate::EAction::KILL);
      }

      objList = getObjectGroup().findSameSource(
            ZStackObjectSourceFactory::MakeFlyEmPsdSource(bodyId));
      for (TStackObjectList::iterator iter = objList.begin();
           iter != objList.end(); ++iter) {
//        removeObject(*iter, false);
//        dumpGarbageUnsync(*iter, true);
        getDataBuffer()->addUpdate(*iter, ZStackDocObjectUpdate::EAction::KILL);
      }

      objList = getObjectGroup().findSameClass(
            ZStackObjectRole::ROLE_SEED,
            ZStackObjectSourceFactory::MakeFlyEmSeedSource(bodyId));
      /*
      objList = getObjectGroup().findSameSource(
            ZStackObjectSourceFactory::MakeFlyEmSeedSource(bodyId));
      getDataBuffer()->addUpdate(objList, ZStackDocObjectUpdate::EAction::ACTION_KILL);
      */

      objList = getObjectList(ZStackObjectRole::ROLE_TMP_RESULT);
      getDataBuffer()->addUpdate(objList, ZStackDocObjectUpdate::EAction::KILL);
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

  KINFO << QString("Body removed: %1").arg(bodyId);
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
      retriveBodyObject(bodyId, zoom, bodyType, ZStackObject::EType::SWC);

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
  for (int zoom = m_minDsLevel; zoom <= resLevel; ++zoom) {
    tree = recoverFromGarbage<ZSwcTree>(
          ZStackObjectSourceFactory::MakeFlyEmBodySource(
            bodyId, zoom, flyem::EBodyType::SPHERE));
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
            bodyId, zoom, flyem::EBodyType::MESH));
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
//  if (!m_bodyReader.isReady()) {
//    m_bodyReader.open(m_workDvidReader.getDvidTarget());
//  }

  ZIntPoint pt = getBodyReader().readBodyPosition(bodyId1);
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
//  if (!m_bodyReader.isReady()) {
//    m_bodyReader.open(m_workDvidReader.getDvidTarget());
//  }

  uint64_t bodyId1 = getBodyReader().readBodyIdAt(pt);
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
  return getBodyManager().getAggloId(bodyId);
  /*
  for (const auto &m : m_tarIdToMeshIds) {
    if (m.second.count(bodyId) > 0) {
      bodyId = m.first;
      break;
    }
  }

  return decode(bodyId);
  */
}

bool ZFlyEmBody3dDoc::isAgglo(uint64_t bodyId) const
{
  return getBodyManager().hasMapping(bodyId);
}

QSet<uint64_t> ZFlyEmBody3dDoc::getMappedSet(uint64_t bodyId) const
{
  return getBodyManager().getMappedSet(bodyId);
}

/*
QSet<uint64_t> ZFlyEmBody3dDoc::getUnencodedBodySet() const
{
  QSet<uint64_t> bodySet;
  foreach (uint64_t bodyId, m_bodySet) {
    bodySet.insert(decode(bodyId));
  }
  return bodySet;
}
*/

QSet<uint64_t> ZFlyEmBody3dDoc::getNormalBodySet() const
{
#ifdef _DEBUG_2
  getBodyManager().print();
#endif
  return getBodyManager().getNormalBodySet();
  /*
  if (isTarMode()) {
    QSet<uint64_t> bodySet;
    for (const auto &m : m_tarIdToMeshIds) {
      bodySet.insert(decode(m.first));
    }

    return bodySet;
  }

  return getUnencodedBodySet();
  */
}

QSet<uint64_t> ZFlyEmBody3dDoc::getInvolvedNormalBodySet() const
{
  QSet<uint64_t> bodySet = getBodyManager().getNormalBodySet();

  QSet<uint64_t> orphanSet = getBodyManager().getOrphanSupervoxelSet(false);
  for (uint64_t spId : orphanSet) {
    uint64_t bodyId = getMainDvidReader().readParentBodyId(spId);
    if (bodyId > 0) {
      bodySet.insert(bodyId);
    }
  }

  return bodySet;
}

ZDvidReader& ZFlyEmBody3dDoc::getBodyReader()
{
  if (!m_bodyReader.isReady()) {
    m_bodyReader.open(getMainDvidReader().getDvidTarget());
  }

  return m_bodyReader;
}

std::vector<ZSwcTree*> ZFlyEmBody3dDoc::makeDiffBodyModel(
    uint64_t bodyId1, uint64_t bodyId2, ZDvidReader &diffReader, int zoom,
    flyem::EBodyType bodyType)
{
  std::vector<ZSwcTree*> treeArray;

  if (bodyId1 > 0 && bodyId2 > 0) {
    if (bodyType == flyem::EBodyType::SKELETON) {
      zoom = 0;
    }

    if (bodyType == flyem::EBodyType::SPHERE && isCoarseLevel(zoom)) {
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

  if (bodyType == flyem::EBodyType::SKELETON) {
    zoom = 0;
  }

  if (bodyType == flyem::EBodyType::SKELETON) {
    tree = recoverFromGarbage<ZSwcTree>(
          ZStackObjectSourceFactory::MakeFlyEmBodySource(
            bodyId, 0, bodyType));
  } else if (bodyType == flyem::EBodyType::SPHERE) {
    tree = recoverFullBodyFromGarbage(bodyId, zoom);
  }

  if (tree == NULL) {
    const ZDvidReader &reader = getWorkDvidReader();
    if (bodyId > 0) {
      int t = m_objectTime.elapsed();
      if (bodyType == flyem::EBodyType::SKELETON) {
        tree = reader.readSwc(bodyId);
      } else if (isCoarseLevel(zoom)) {
        ZObject3dScan obj = reader.readCoarseBody(bodyId);
        if (m_quitting) {
          return NULL;
        }

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
          reader.readMultiscaleBody(bodyId, zoom, true, &obj);
          if (m_quitting) {
            return NULL;
          }

          if (!obj.isEmpty()) {
            tree = ZSwcFactory::CreateSurfaceSwc(obj, 3);
          }
        } else {
          tree = ZSwcFactory::CreateSurfaceSwc(*cachedBody, 3);
          zoom = 0;
        }
      }

      if (tree != NULL) {
        tree->setTimeStamp(t);
        if (IsOverSize(tree) && zoom <= 2) {
          zoom = 0;
        }
        tree->setSource(
              ZStackObjectSourceFactory::MakeFlyEmBodySource(
                bodyId, zoom, bodyType));
        SetObjectClass(tree, bodyId);
        tree->setLabel(bodyId);
      }
    }
  }

  return tree;
}

uint64_t ZFlyEmBody3dDoc::decode(uint64_t encodedId)
{
  return ZFlyEmBodyManager::decode(encodedId);
}

bool ZFlyEmBody3dDoc::usingOldMeshesTars() const
{
  bool result = false;
  if (const char* setting = std::getenv("NEU3_USE_TARSUPERVOXELS")) {
    result = (std::string(setting) == "no");
  }
  return result;
}

#if 0
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

bool ZFlyEmBody3dDoc::isTarMode() const
{
  return !m_tarIdToMeshIds.empty();
}
#endif

bool ZFlyEmBody3dDoc::fromTar(uint64_t id) const
{
  if (ZFlyEmBodyManager::encodesTar(id)) {
    return true;
  } else {
    return getBodyManager().isSupervoxel(id);
  }
//  return getBodyManager().hasMapping(id);
}

std::vector<ZMesh*> ZFlyEmBody3dDoc::getTarCachedMeshes(uint64_t bodyId)
{
  std::vector<ZMesh*> recoveredMeshes;

  QSet<uint64_t> meshIds = getBodyManager().getMappedSet(bodyId);
  if (!meshIds.isEmpty()) {
//      auto& meshIds = it->second;
//    std::vector<ZMesh *> recoveredMeshes;

    for (uint64_t meshId : meshIds) {
      if (ZMesh *mesh = recoverMeshFromGarbage(meshId, 0)) {
        recoveredMeshes.push_back(mesh);
      }
    }

    if ((int) recoveredMeshes.size() != meshIds.size()) {
      recoveredMeshes.clear();
    }
  } else {
    // Check for buffered (prefetched) meshes.

    meshIds = getBodyManager().getBufferedMappedSet(bodyId);
    if (!meshIds.empty()) {
      for (uint64_t meshId : meshIds) {
        ZStackObject *obj = takeObjectFromBuffer(
              ZStackObject::EType::MESH,
              ZStackObjectSourceFactory::MakeFlyEmBodySource(
                meshId, 0, flyem::EBodyType::MESH));
        if (ZMesh *mesh = dynamic_cast<ZMesh*>(obj)) {
          recoveredMeshes.push_back(mesh);
        }
      }
      if ((int) recoveredMeshes.size() != meshIds.size()) {
        recoveredMeshes.clear();
      }
      getBodyManager().deregisterBufferedBody(bodyId);
    }
  }

  return recoveredMeshes;
}

std::vector<ZMesh*>  ZFlyEmBody3dDoc::getCachedMeshes(uint64_t bodyId, int zoom)
{
  std::vector<ZMesh *> recoveredMeshes;

  if (ZFlyEmBodyManager::encodesTar(bodyId)) {
    recoveredMeshes = getTarCachedMeshes(bodyId);
#if 0
//    auto it = m_tarIdToMeshIds.find(bodyId);
//    if (it != m_tarIdToMeshIds.end()) {
    QSet<uint64_t> meshIds = getBodyManager().getMappedSet(bodyId);
    if (!meshIds.isEmpty()) {
//      auto& meshIds = it->second;
      std::vector<ZMesh *> recoveredMeshes;

      for (uint64_t meshId : meshIds) {
        if (ZMesh *mesh = recoverMeshFromGarbage(meshId, 0)) {
          recoveredMeshes.push_back(mesh);
        }
      }

      if ((int) recoveredMeshes.size() == meshIds.size()) {
        for (ZMesh *mesh : recoveredMeshes) {
          result[mesh->getLabel()] = mesh;
        }
        return true;
      }
    }
    return false;
#endif
  } else {
    ZMesh *mesh = recoverMeshFromGarbage(bodyId, zoom);
  #if 0 //todo
    if (mesh == NULL) {
      std::string source = ZStackObjectSourceFactory::MakeFlyEmBodySource(
            bodyId, zoom, flyem::BODY_MESH);
      mesh = dynamic_cast<ZMesh*>(
            takeObjectFromCache(ZStackObject::EType::TYPE_MESH, source));
    }
  #endif
    if (mesh) {
      recoveredMeshes.push_back(mesh);
    }
  }

  return recoveredMeshes;
//  if (!recoveredMeshes.empty()) {
//    result[bodyId] = recoveredMeshes;
//    return true;
//  }

//  return false;
}

#if 0
ZMesh* ZFlyEmBody3dDoc::readSupervoxelMesh(
    const ZDvidReader &reader, uint64_t bodyId, int zoom)
{
  ZMesh *mesh = NULL;
  if (getDvidTarget().hasSupervoxel()) {
    bool loaded = false;
    if (zoom > getMaxDsLevel()) {
      //For zooming level beyond the range, ignore it if the mesh at any level exists.
      loaded = !(getObjectGroup().findSameClass(
                   ZStackObject::EType::TYPE_MESH,
                   ZStackObjectSourceFactory::MakeFlyEmBodySource(bodyId)).
                 isEmpty());
    }

    if (!loaded) { //Now make mesh from sparse vol
      ZObject3dScan obj;
      if (isCoarseLevel(zoom) && !config.isHybrid()){
        reader.readCoarseBody(config.getBodyId(), &obj);
        obj.setDsIntv(getDvidInfo().getBlockSize() - 1);
        mesh = ZMeshFactory::MakeMesh(obj);
      } else {
        ZDvidBodyHelper helper(&reader);
        helper.setLabelType(flyem::LABEL_SUPERVOXEL);

        if (isCoarseLevel(zoom)) {
          helper.setCoarse(true);
        }
         helper.setZoom(config.getDsLevel());
        if (config.isHybrid()) {
          helper.setRange(config.getRange());
          helper.setLowresZoom(config.getDsLevel());
          helper.setZoom(config.getLocalDsLevel());
        }
//        helper.setLowresZoom(config.getLocalDsLevel());
//        helper.setCoarse(true);
        ZObject3dScanArray objArray = helper.readHybridBody(config.getBodyId());
        ZMeshFactory mf;
        mesh = mf.makeMesh(objArray);
//        reader.readMultiscaleBody(config, zoom, true, &obj);
      }

      if (mesh) {
        mesh->setLabel(bodyId);
      }
    }
  }

  return mesh;
}
#endif

ZMesh *ZFlyEmBody3dDoc::readMesh(
    const ZDvidReader &reader, ZFlyEmBodyConfig &config)
{
  int zoom = config.getDsLevel();
//  *acturalMeshZoom = zoom;

  ZMesh *mesh = NULL;

  if (ZFlyEmBodyManager::encodesTar(config.getBodyId())) {
    if (ZFlyEmBodyManager::encodedLevel(config.getBodyId()) == 1) {
      bool showProgress = !config.getAddBuffer();
      std::vector<ZMesh*> meshArray = makeTarMeshModels(
            reader, config.getBodyId(), m_objectTime.elapsed(), showProgress);
      if (meshArray.size() == 1) {
        mesh = meshArray[0];
      } else {
        for (ZMesh *m : meshArray) {
          delete m;
        }
      }
    }
    config.setDsLevel(0);
//    *acturalMeshZoom = 0;

    return mesh;
  }

  if (config.getLabelType() == neutu::EBodyLabelType::SUPERVOXEL) {
    if (!isCoarseLevel(zoom)) {
      mesh = readSupervoxelMesh(reader, config.getBodyId());
      if (mesh != NULL) {
        config.setDsLevel(0);
//        *acturalMeshZoom = 0;
      }
    }
  } else {
    if (!isCoarseLevel(zoom)) { //Skip coarse Level
      //Checking order: merged mesh -> ngmesh -> normal mesh
      std::string mergeKey =
          ZDvidUrl::GetMeshKey(config.getBodyId(), ZDvidUrl::EMeshType::MERGED);
      mesh = reader.readMesh(mergeKey);
      if (mesh) {
        config.setDsLevel(0);
//        *acturalMeshZoom = 0;
      } else {
        //Skip ngmesh if the merge key exists but is broken
        if (!reader.hasKey(getDvidTarget().getMeshName().c_str(), mergeKey.c_str())) {
          mesh = reader.readMesh(
                ZDvidUrl::GetMeshKey(config.getBodyId(), ZDvidUrl::EMeshType::NG));
          if (mesh) {
            config.setDsLevel(0);
//            *acturalMeshZoom = 0;
          } else {
            mesh = reader.readMesh(
                  ZDvidUrl::GetMeshKey(
                    config.getBodyId(), zoom, ZDvidUrl::EMeshType::NG));
          }
        }
      }

      if (mesh == nullptr) {
        mesh = reader.readMesh(config.getBodyId(), zoom);
        /*
        if (mesh != NULL) {
          *acturalMeshZoom = zoom;
        }
        */
      }
    }
  }

  if (mesh && config.getDsLevel() > 0) {
    ZIntCuboid box = reader.readBodyBoundBox(config.getBodyId());
    if (ZStackObjectHelper::IsOverSize(box, zoom)) {
      ZStackObjectHelper::SetOverSize(mesh);
//      config.disableNextDsLevel();
    }
  }

  if (mesh == NULL) {
    bool loaded = false;
    if (zoom > getMaxDsLevel()) {
      //For zooming level beyond the range, ignore it if the mesh at any level exists.
      loaded = !(getObjectGroup().findSameClass(
                   ZStackObject::EType::MESH,
                   ZStackObjectSourceFactory::MakeFlyEmBodySource(config.getBodyId())).
                 isEmpty());
    }

    if (!loaded) { //Now make mesh from sparse vol
      ZObject3dScan obj;
      if (isCoarseLevel(zoom) && !config.isHybrid()){
        reader.readCoarseBody(config.getDecodedBodyId(),
              config.getLabelType(), &obj);
        obj.setDsIntv(getDvidInfo().getBlockSize() - 1);
        mesh = ZMeshFactory::MakeMesh(obj);
      } else {
        ZDvidBodyHelper helper(&reader);
        helper.setLabelType(config.getLabelType());

        if (isCoarseLevel(zoom)) {
          helper.setCoarse(true);
        }
        helper.setZoom(config.getDsLevel());
        if (config.isHybrid()) {
          helper.setRange(config.getRange());
          helper.setLowresZoom(config.getDsLevel());
          helper.setZoom(config.getLocalDsLevel());
        }
//        helper.setLowresZoom(config.getLocalDsLevel());
//        helper.setCoarse(true);
        ZObject3dScanArray objArray = helper.readHybridBody(
              config.getDecodedBodyId());
        ZMeshFactory mf;
        QElapsedTimer timer;
        timer.start();
        mesh = mf.makeMesh(objArray);

        if (mesh && !config.isHybrid() &&
            config.getLabelType() == neutu::EBodyLabelType::BODY) {
          std::shared_ptr<ZMesh> meshClone(mesh->clone());
          getDataDocument()->addUploadTask([=](ZDvidWriter &writer) {
            {
              QByteArray data = meshClone->writeToMemory("obj");
              writer.writeDataToKeyValue(
                    writer.getDvidTarget().getMeshName(),
                    ZDvidUrl::GetMeshKey(config.getBodyId(), zoom), data);
#ifdef _DEBUG_
              std::cout << ZDvidUrl::GetMeshKey(config.getBodyId(), zoom)
                        << " uploaded" << std::endl;
#endif
            }

            {
              ZDvidInfo info = writer.getDvidReader().readLabelInfo();
              ZResolution res = info.getVoxelResolution();
              double sx = res.getVoxelSize(neutu::EAxis::X, res.getUnit());
              double sy = res.getVoxelSize(neutu::EAxis::Y, res.getUnit());
              double sz = res.getVoxelSize(neutu::EAxis::Z, res.getUnit());
              meshClone->scale(sx, sy, sz);
              QByteArray data = meshClone->writeToMemory("ngmesh");
              writer.writeDataToKeyValue(
                    writer.getDvidTarget().getMeshName(),
                    ZDvidUrl::GetMeshKey(
                      config.getBodyId(), zoom, ZDvidUrl::EMeshType::NG), data);
#ifdef _DEBUG_
              std::cout << ZDvidUrl::GetMeshKey(
                             config.getBodyId(), zoom, ZDvidUrl::EMeshType::NG)
                        << " uploaded" << std::endl;
#endif
            }
          });
          /*
          QByteArray data = mesh->writeToMemory("obj");
          getDataDocument()->addUploadTask(
                getWorkDvidReader().getDvidTarget().getMeshName(),
                ZDvidUrl::GetMeshKey(config.getBodyId(), zoom), data);
                */
        }

        neutu::LogProfileInfo(
              timer.elapsed(),
              QString("Mesh generating time for %1 with zoom %2~%3").
              arg(config.getBodyId()).arg(config.getDsLevel()).
              arg(config.getLocalDsLevel()).toStdString());

//        LINFO() << "Mesh generating time:" << timer.elapsed() << "ms";
//        reader.readMultiscaleBody(config, zoom, true, &obj);
      }

      if (mesh) {
        mesh->setLabel(config.getBodyId());
      }
    }
  }

  if (mesh) {
    if (IsOverSize(mesh) && config.getDsLevel() <= 2) {
      config.disableNextDsLevel();
    }
  }

  return mesh;
}

ZMesh *ZFlyEmBody3dDoc::readMesh(ZFlyEmBodyConfig &config)
{
  return readMesh(getWorkDvidReader(), config);
}

ZMesh *ZFlyEmBody3dDoc::readMesh(const ZDvidReader &reader, uint64_t bodyId, int zoom)
{
  ZFlyEmBodyConfig config(bodyId);
  config.setDsLevel(zoom);
  return readMesh(reader, config);
}

namespace {
  void finalizeMesh(ZMesh *mesh, uint64_t parentId, int zoom, int t)
  {
    if (mesh) {
      uint64_t id = mesh->getLabel();
      mesh->prepareNormals();
      mesh->setTimeStamp(t);
      auto source = ZStackObjectSourceFactory::MakeFlyEmBodySource(
            id, zoom, flyem::EBodyType::MESH);
      mesh->setSource(source);
      ZFlyEmBody3dDoc::SetObjectClass(mesh, parentId);
    }
  }
}

std::vector<ZMesh*> ZFlyEmBody3dDoc::makeTarMeshModels(
    const ZDvidReader &reader, uint64_t bodyId, int t, bool showProgress)
{
  std::vector<ZMesh*> resultVec;

  if (showProgress) {
    emit meshArchiveLoadingStarted();
  }

  // It is challenging to emit progress updates as m_dvidReader reads the data for the
  // tar archive, so just initialize the progress meter to show ani ntermediate status.

  const float PROGRESS_FRACTION_START = 1.0f / 3.0f;
  if (showProgress) {
    emit meshArchiveLoadingProgress(PROGRESS_FRACTION_START);
  }

  bool isSupervoxelTar = ZFlyEmBodyManager::encodingSupervoxelTar(bodyId);

  bool useOldMeshesTars = usingOldMeshesTars();
  if (!useOldMeshesTars) {
    bodyId = decode(bodyId);
  }

  if (struct archive *arc = reader.readMeshArchiveStart(bodyId, useOldMeshesTars)) {

    // When reading the meshes, decompress them in parallel to improve performance.
    // The following lambda function updates the progress dialog during the decompression.

    auto progress = [=](size_t i, size_t n) {
      if (showProgress) {
        float fraction = float(i) / n;
        float progressFraction = PROGRESS_FRACTION_START + (1 - PROGRESS_FRACTION_START) * fraction;
        emit meshArchiveLoadingProgress(progressFraction);
      }
    };

    reader.readMeshArchiveAsync(arc, resultVec, progress);
    //      QSet<uint64_t> mappedSet;
    uint64_t decodedBodyId = decode(bodyId);
    for (ZMesh *mesh : resultVec) {
      finalizeMesh(mesh, decodedBodyId, 0, t);
      if (isSupervoxelTar) {
        mesh->addRole(ZStackObjectRole::ROLE_SUPERVOXEL);
      }
      //        result[mesh->getLabel()] = mesh;
      //        mappedSet.insert(mesh->getLabel());
    }
//    result[bodyId] = resultVec;
    //      getBodyManager().registerBody(id, mappedSet);

    reader.readMeshArchiveEnd(arc);

    if (showProgress) {
      emit meshArchiveLoadingEnded();
    }
  } else {
    QString title = "Mesh Loading Failed";
    uint64_t idUnencoded = ZFlyEmBodyManager::decode(bodyId);
    QString text = "DVID mesh archive does not contain ID " +
        QString::number(idUnencoded) + " (encoded as " + QString::number(bodyId) + ")";
    ZWidgetMessage msg(title, text, neutu::EMessageType::ERROR, ZWidgetMessage::TARGET_DIALOG);
    emit messageGenerated(msg);
    emit meshArchiveLoadingEnded();
  }

  return resultVec;
}

std::vector<ZMesh*> ZFlyEmBody3dDoc::makeTarMeshModels(
    uint64_t bodyId, int t, bool showProgress)
{
  return makeTarMeshModels(getWorkDvidReader(), bodyId, t, showProgress);
}

std::vector<ZMesh*> ZFlyEmBody3dDoc::makeBodyMeshModels(
    ZFlyEmBodyConfig &config)
{
  std::vector<ZMesh*> result;

//  int zoom = config.getDsLevel();

  if (!config.isHybrid() && config.getBodyId() > 0) {
    result = getCachedMeshes(config.getBodyId(), config.getDsLevel());
  }

  if (result.empty()) {
    int t = m_objectTime.elapsed();
    if (config.isTar()) {
      bool showProgress = !config.getAddBuffer();
      result = makeTarMeshModels(config.getBodyId(), t, showProgress);
    } else {
      ZMesh *mesh = NULL;

      if (!config.isHybrid()) {
        ZStackObject *obj = takeObjectFromBuffer(
              ZStackObject::EType::MESH,
              ZStackObjectSourceFactory::MakeFlyEmBodySource(
                config.getBodyId(), 0, flyem::EBodyType::MESH));
        mesh = dynamic_cast<ZMesh*>(obj);
        if (mesh) {
          config.setDsLevel(0);
        }
      }
      if (mesh == NULL) {
        mesh = readMesh(config);
        if (mesh != NULL) {
          mesh->setLabel(config.getBodyId());
        }
      }
      if (mesh != NULL) {
        uint64_t parentId = config.getBodyId();
        if (config.getLabelType() != neutu::EBodyLabelType::SUPERVOXEL) {
          parentId = decode(parentId);
        }
        finalizeMesh(mesh, parentId, config.getDsLevel(), t);
        result.push_back(mesh);
      }
    }
  }

  return result;

#if 0
  if (ZFlyEmBodyManager::encodesTar(id)) {
    std::vector<ZMesh*> resultVec = makeTarMeshModels(id, t);
    result[id] = resultVec;

#if 0
//    getBodyManager().registerBody(id, QSet<uint64_t>());
//    m_tarIdToMeshIds[id].clear();

    emit meshArchiveLoadingStarted();

    // It is challenging to emit progress updates as m_dvidReader reads the data for the
    // tar archive, so just initialize the progress meter to show ani ntermediate status.

    const float PROGRESS_FRACTION_START = 1 / 3.0;
    emit meshArchiveLoadingProgress(PROGRESS_FRACTION_START);

    // For now, use the new "tarsupervoxels" data instance for tar archives of meshes
    // only if an environment variable is set,  When testing is complete, this approach
    // will become the default.

    bool useOldMeshesTars = usingOldMeshesTars();
    if (!useOldMeshesTars) {
      id = decode(id);
    }

    if (struct archive *arc = m_workDvidReader.readMeshArchiveStart(id, useOldMeshesTars)) {
      std::vector<ZMesh*> resultVec;

      // When reading the meshes, decompress them in parallel to improve performance.
      // The following lambda function updates the progress dialog during the decompression.

      auto progress = [=](size_t i, size_t n) {
        float fraction = float(i) / n;
        float progressFraction = PROGRESS_FRACTION_START + (1 - PROGRESS_FRACTION_START) * fraction;
        emit meshArchiveLoadingProgress(progressFraction);
      };

      m_workDvidReader.readMeshArchiveAsync(arc, resultVec, progress);
//      QSet<uint64_t> mappedSet;
      for (ZMesh *mesh : resultVec) {
        finalizeMesh(mesh, 0, t);
//        result[mesh->getLabel()] = mesh;
//        mappedSet.insert(mesh->getLabel());
      }
      result[id] = resultVec;
//      getBodyManager().registerBody(id, mappedSet);

      m_workDvidReader.readMeshArchiveEnd(arc);

      emit meshArchiveLoadingEnded();

    } else {
      QString title = "Mesh Loading Failed";
      uint64_t idUnencoded = ZFlyEmBodyManager::decode(id);
      QString text = "DVID mesh archive does not contain ID " +
          QString::number(idUnencoded) + " (encoded as " + QString::number(id) + ")";
      ZWidgetMessage msg(title, text, neutube::EMessageType::MSG_ERROR, ZWidgetMessage::TARGET_DIALOG);
      emit messageGenerated(msg);
      emit meshArchiveLoadingEnded();
    }
#endif
  } else {
    ZMesh *mesh = NULL;

    if (!config.isHybrid()) {
      ZStackObject *obj = takeObjectFromBuffer(
            ZStackObject::EType::TYPE_MESH,
            ZStackObjectSourceFactory::MakeFlyEmBodySource(id, 0, flyem::BODY_MESH));
      mesh = dynamic_cast<ZMesh*>(obj);
    }
    if (mesh == NULL) {
      mesh = readMesh(config);
      if (mesh != NULL) {
        if (IsOverSize(mesh) && zoom <= 2) {
          zoom = 0;
        }
      }
    } else {
      zoom = 0;
    }
    finalizeMesh(mesh, zoom, t);
    result[id] = mesh;
  }
#endif
}

const ZDvidInfo& ZFlyEmBody3dDoc::getDvidInfo() const
{
  return m_dvidInfo;
}

void ZFlyEmBody3dDoc::setDvidTarget(const ZDvidTarget &target)
{
//  m_dvidTarget = target;
  for (ZDvidReader& reader : m_workDvidReader) {
    reader.clear();
    reader.open(target);
  }
  m_mainDvidWriter.clear();
  m_bodyReader.clear();

  m_mainDvidWriter.open(target);

  updateDvidInfo();

  m_splitter->setDvidTarget(target);
}

bool ZFlyEmBody3dDoc::isDvidMutable() const
{
  return (getDvidTarget().readOnly() == false);
}

const ZDvidReader& ZFlyEmBody3dDoc::getMainDvidReader() const
{
  return m_mainDvidWriter.getDvidReader();
}

ZDvidWriter& ZFlyEmBody3dDoc::getMainDvidWriter()
{
  return m_mainDvidWriter;
}

const ZDvidReader& ZFlyEmBody3dDoc::getWorkDvidReader() const
{
  if (!m_workDvidReaderIndices.hasLocalData()) {
    if (m_workDvidReaderNextIndex < NUM_WORK_DVID_READERS) {
      QMutexLocker locker(&m_workDvidReaderNextIndexMutex);
      m_workDvidReaderIndices.setLocalData(m_workDvidReaderNextIndex++);
    } else {
      ZWidgetMessage msg("ZFlyEmBody3dDoc::getWorkDvidReader(): more threads than expected, "
                         "corruption is likely", neutu::EMessageType::WARNING,
                         ZWidgetMessage::TARGET_TEXT_APPENDING | ZWidgetMessage::TARGET_KAFKA);
      return m_workDvidReader.front();
    }
  }
  return m_workDvidReader[m_workDvidReaderIndices.localData()];
}

void ZFlyEmBody3dDoc::updateDvidInfo()
{
  m_dvidInfo.clear();

  if (getMainDvidReader().isReady()) {
    m_dvidInfo = getMainDvidReader().readLabelInfo();
    setMaxDsLevel(zgeom::GetZoomLevel(m_dvidInfo.getBlockSize().getX()));
    ZDvidGraySlice *slice = getArbGraySlice();
    if (slice != NULL) {
      slice->setDvidTarget(getMainDvidReader().getDvidTarget().getGrayScaleTarget());
    }
  }
}

void ZFlyEmBody3dDoc::processBodySelectionChange()
{
  std::set<uint64_t> bodySet =
      getDataDocument()->getSelectedBodySet(neutu::ELabelSource::ORIGINAL);

  addBodyChangeEvent(bodySet.begin(), bodySet.end());
}

QColor ZFlyEmBody3dDoc::getBodyColor(uint64_t bodyId)
{
  QColor color(Qt::white);

#ifndef _NEU3_
  if (getDataDocument() != NULL) {
    ZDvidLabelSlice *labelSlice =
        ZFlyEmProofDocUtil::GetActiveLabelSlice(getDataDocument(), neutu::EAxis::Z);

    if (labelSlice != NULL) {
      if (getBodyType() == flyem::EBodyType::SPHERE) { //using the original color
        color = labelSlice->getLabelColor(bodyId, neutu::ELabelSource::MAPPED);
      } else {
        color = labelSlice->getLabelColor(bodyId, neutu::ELabelSource::ORIGINAL);
      }
      color.setAlpha(255);
    }
  }
#else
  Q_UNUSED(bodyId);
#endif

  return color;
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
              container.getRange(), neutu::EBodySplitMode::BODY_SPLIT_ONLINE);
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
              container.getRange(), neutu::EBodySplitMode::BODY_SPLIT_ONLINE);
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
              container.getRange(), neutu::EBodySplitMode::BODY_SPLIT_ONLINE);
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
  LKINFO << "Trying local split ...";

  uint64_t bodyId = protectBodyForSplit();

  if (bodyId > 0) {
    activateSplit(bodyId);
    if (isSplitActivated()) {
      if (!m_futureMap.isAlive(THREAD_SPLIT_KEY)) {
        removeObject(ZStackObjectRole::ROLE_SEGMENTATION, true);
        QFuture<void> future =
            QtConcurrent::run(m_splitter, &ZFlyEmBodySplitter::runLocalSplit);
        m_futureMap[THREAD_SPLIT_KEY] = future;
      } else {
        notifyWindowMessageUpdated("Split is still running ...");
      }
    }
  }
}

void ZFlyEmBody3dDoc::runSplit()
{
  LKINFO << "Trying split ...";

  if (!m_futureMap.isAlive(THREAD_SPLIT_KEY)) {
    removeObject(ZStackObjectRole::ROLE_SEGMENTATION, true);
    QFuture<void> future =
        QtConcurrent::run(m_splitter, &ZFlyEmBodySplitter::runSplit);
    m_futureMap[THREAD_SPLIT_KEY] = future;
  } else {
    notifyWindowMessageUpdated("Split is still running ...");
  }
}

void ZFlyEmBody3dDoc::runFullSplit()
{
  LKINFO << "Trying full split ...";

  if (!m_futureMap.isAlive(THREAD_SPLIT_KEY)) {
    removeObject(ZStackObjectRole::ROLE_SEGMENTATION, true);
    QFuture<void> future =
        QtConcurrent::run(m_splitter, &ZFlyEmBodySplitter::runFullSplit);
    m_futureMap[THREAD_SPLIT_KEY] = future;
  }
}

void ZFlyEmBody3dDoc::constructBodyMesh(ZMesh *mesh, uint64_t bodyId, bool fromTar)
{
  mesh->setLabel(bodyId);
  mesh->setSource(
        ZStackObjectSourceFactory::MakeFlyEmBodySource(
          bodyId, 0, flyem::EBodyType::MESH));
  if (fromTar) {
    uint64_t parentId = getBodyManager().getAggloId(bodyId);
    SetObjectClass(mesh, parentId);
    mesh->addRole(ZStackObjectRole::ROLE_SUPERVOXEL);
  } else {
    SetObjectClass(mesh, bodyId);
  }
  mesh->setColor(Qt::white);
  mesh->pushObjectColor();
  mesh->removeRole(ZStackObjectRole::ROLE_SEGMENTATION);
}

void ZFlyEmBody3dDoc::retrieveSegmentationMesh(QMap<std::string, ZMesh *> *meshMap)
{
  QList<ZStackObject*> decorList =
      m_helper->getObjectList(neutu3d::ERendererLayer::DECORATION);
  for (auto &obj : decorList) {
    ZMesh *mesh = dynamic_cast<ZMesh*>(obj);
    if (mesh) {
      if (mesh->hasRole(ZStackObjectRole::ROLE_SEGMENTATION) &&
          !obj->getObjectId().empty()) {
        if (meshMap->contains(obj->getObjectId())) {
          LKWARN << QString("Unexpected object ID detected: %1").
                    arg(obj->getObjectId().c_str());
        } else {
          (*meshMap)[obj->getObjectId()] = mesh;
        }
      }
    }
  }
}

#define FLYEM_ADD_PROFILE_TIME(call, time) \
{\
  QElapsedTimer timer;\
  timer.start();\
  call;\
  time += timer.elapsed();\
}

void ZFlyEmBody3dDoc::commitSplitResult()
{
  QElapsedTimer timer;
  timer.start();

  QAction *action = getAction(ZActionFactory::ACTION_COMMIT_SPLIT);
  if (action != NULL) {
    action->setVisible(false);
  }

  notifyWindowMessageUpdated("Uploading splitted bodies");

  QString summary;

  uint64_t oldId = m_splitter->getBodyId();
  uint64_t remainderId = oldId;

  uint64_t parentId = getBodyManager().getAggloId(oldId); //for tar

  ZObject3dScan *remainObj = new ZObject3dScan;

  *remainObj = *(m_splitter->getBodyForSplit()->getObjectMask());

  QList<ZStackObject*> objList =
      getObjectList(ZStackObjectRole::ROLE_SEGMENTATION);


  ZStackObjectArray objToDelete;

  QMap<std::string, ZMesh*> meshMap;
  retrieveSegmentationMesh(&meshMap);

  QList<ZMesh*> mainMeshList;
  bool uploadingMesh =
      (m_splitter->getLabelType() == neutu::EBodyLabelType::SUPERVOXEL);

  int64_t uploadingTime = 0;
  int64_t subtractingTime = 0;
  int64_t meshProcessingTime = 0;
  int64_t meshUploadingTime = 0;

  for (ZStackObject *obj : objList) {
    ZObject3dScan *seg = dynamic_cast<ZObject3dScan*>(obj);
    if (seg != NULL) {
      ZMesh *mesh = NULL;
      if (meshMap.contains(obj->getObjectId())) {
        mesh = meshMap[obj->getObjectId()];
      } else if (uploadingMesh) {
        QElapsedTimer timer;
        timer.start();
        mesh = ZMeshFactory::MakeMesh(*seg);
        meshProcessingTime += timer.elapsed();
//        FLYEM_ADD_PROFILE_TIME(ZMeshFactory::MakeMesh(*seg), meshProcessingTime);
        objToDelete.push_back(mesh);
      }

      if (seg->getLabel() > 1) {
        notifyWindowMessageUpdated(
              QString("Uploading %1").arg(seg->getLabel()));
        uint64_t newBodyId = 0;
        if (m_splitter->getLabelType() == neutu::EBodyLabelType::BODY) {
          QElapsedTimer timer;
          timer.start();
          newBodyId = m_mainDvidWriter.writeSplit(*seg, remainderId, 0);
          uploadingTime += timer.elapsed();
        } else {
          QElapsedTimer timer;
          timer.start();
          invalidateSupervoxelCache(m_splitter->getBodyId());
          std::pair<uint64_t, uint64_t> idPair =
              m_mainDvidWriter.writeSupervoxelSplit(*seg, remainderId);
          uploadingTime += timer.elapsed();

          remainderId = idPair.first;
          newBodyId = idPair.second;

          notifyWindowMessageUpdated(QString("Updating mesh ..."));

          if (mesh != nullptr) {
//            FLYEM_ADD_PROFILE_TIME(
//                  m_mainDvidWriter.writeSupervoxelMesh(*mesh, newBodyId),
//                  meshUploadingTime);
            QElapsedTimer timer;
            timer.start();
            m_mainDvidWriter.writeSupervoxelMesh(*mesh, newBodyId);
            meshUploadingTime += timer.elapsed();
          }

          if (m_splitter->fromTar()) {
            getBodyManager().registerBody(parentId, newBodyId);
            m_helper->releaseObject(neutu3d::ERendererLayer::DECORATION, mesh);
            ZStackDocAccessor::AddObjectUnique(this, mesh);
          }
          constructBodyMesh(mesh, newBodyId, m_splitter->fromTar());

//          ZMesh* mesh = ZMeshFactory::MakeMesh(*seg);
//          m_mainDvidWriter.writeMesh(*mesh, newBodyId, 0);
//          delete mesh;

//          emit addingBody(newBodyId);
        }
//        addEvent(BodyEvent::ACTION_ADD, newBodyId);

        summary += QString("Labe %1 uploaded as %2 (%3 voxels)\n").
            arg(seg->getLabel()).arg(newBodyId).arg(seg->getVoxelNumber());

        QElapsedTimer timer;
        timer.start();
        remainObj->subtractSliently(*seg);
        subtractingTime += timer.elapsed();
      }/* else {
        remainObj->unify(*seg);
        if (mesh) {
          mainMeshList.append(mesh);
        }
      }*/
    }
  }


  ZMesh *mainMesh = nullptr;

  if (m_splitter->getLabelType() == neutu::EBodyLabelType::SUPERVOXEL) {
    if (m_splitter->fromTar()) {
      getBodyManager().registerBody(parentId, remainderId);
    } else {
      if (remainderId > 0) {
        remainderId = ZFlyEmBodyManager::EncodeSupervoxel(remainderId);
      }

      oldId = ZFlyEmBodyManager::EncodeSupervoxel(oldId);
    }

    if (mainMeshList.size() == 1) {
      mainMesh = mainMeshList[0];
    } else if (!mainMeshList.isEmpty()) {
      mainMesh = new ZMesh(*mainMeshList[0]);
      for (int i = 1; i < mainMeshList.size(); ++i) {
        mainMesh->append(*mainMeshList[i]);
      }
    }
  }

  if (mainMesh == nullptr) {
    QElapsedTimer timer;
    timer.start();
    mainMesh = ZMeshFactory::MakeMesh(*remainObj);
    meshProcessingTime += timer.elapsed();
  }

#ifdef _DEBUG_
  uploadingMesh = true;
#endif

  if (uploadingMesh) {
    if (m_splitter->getLabelType() == neutu::EBodyLabelType::SUPERVOXEL) {
      QElapsedTimer timer;
      timer.start();
      m_mainDvidWriter.writeSupervoxelMesh(*mainMesh, decode(remainderId));
      meshUploadingTime += timer.elapsed();
    } else {
      QElapsedTimer timer;
      timer.start();
      m_mainDvidWriter.writeMesh(*mainMesh, remainderId, 0);
      meshUploadingTime += timer.elapsed();
    }
  }

//  m_mainDvidWriter.deleteMesh(oldId);
//  ZMesh* mesh = ZMeshFactory::MakeMesh(*remainObj);
//  m_mainDvidWriter.writeMesh(*mesh, remainderId, 0);
//  delete mesh;

  constructBodyMesh(mainMesh, remainderId, m_splitter->fromTar());

  if (m_splitter->fromTar()) {
    ZStackDocAccessor::AddObjectUnique(this, mainMesh);
    if (remainderId  != oldId) {
      ZStackDocAccessor::RemoveObject(
            this, ZStackObject::EType::MESH,
            ZStackObjectSourceFactory::MakeFlyEmBodySource(
              oldId, 0, flyem::EBodyType::MESH), true);
      deregisterBody(oldId);
    }
  } else {
    if (remainderId == oldId) { //Update the body if the main ID remain unchanged
      ZStackDocAccessor::AddObjectUnique(this, mainMesh);
      if (m_splitter->getLabelType() == neutu::EBodyLabelType::BODY) {
        updateTodo(oldId);
        updateSynapse(oldId);
      }
    } else { //Replace the old ID with the remainderID
      dumpGarbage(mainMesh, true); //recycled later

      emit removingBody(oldId);
      emit addingBody(remainderId);
    }
  }

 m_helper->releaseObject(neutu3d::ERendererLayer::DECORATION, mainMesh);

//  addEvent(BodyEvent::ACTION_REMOVE, oldId);
  ZStackDocAccessor::RemoveObject(
        this, ZStackObjectRole::ROLE_SEGMENTATION, true);
  ZStackDocAccessor::RemoveSideSplitSeed(this);

  m_splitter->setBodyId(decode(remainderId));

  m_splitter->updateCachedMask(remainObj);
  /*
  ZDvidSparseStack *sparseStack = getDataDocument()->getDvidSparseStack();
  if (sparseStack != NULL) {
    sparseStack->setObjectMask(remainObj);
  }
  */
//  LKINFO << summary;

  notifyWindowMessageUpdated(summary);

  undoStack()->clear();

  neutu::LogProfileInfo(
        timer.elapsed(),
        QString("commit split (uploading: %1ms; subtraction: %2ms; "
                "mesh processing: %3ms; mesh uploading: %4ms.)")
        .arg(uploadingTime).arg(subtractingTime).arg(meshProcessingTime)
        .arg(meshUploadingTime).toStdString());
}

void ZFlyEmBody3dDoc::waitForSplitToBeDone()
{
  m_futureMap.waitForFinished(THREAD_SPLIT_KEY);
}

void ZFlyEmBody3dDoc::startBodyAnnotation()
{

//  ZFlyEmBodyAnnotationDialog *dlg =
//      new ZFlyEmBodyAnnotationDialog(getParent3DWindow());
  startBodyAnnotation(getBodyAnnotationDlg());
}

void ZFlyEmBody3dDoc::startBodyAnnotation(FlyEmBodyAnnotationDialog *dlg)
{
  if (isDvidMutable()) {
    dlg->updateStatusBox();
    dlg->updatePropertyBox();

    uint64_t bodyId = getSelectedSingleNormalBodyId();
    if (bodyId > 0 && getDataDocument() != NULL) {
      if (getDataDocument()->checkOutBody(bodyId, neutu::EBodySplitMode::NONE)) {
        dlg->setBodyId(bodyId);
        const ZDvidReader &reader = getMainDvidReader();
        if (reader.isReady()) {
          ZFlyEmBodyAnnotation annotation =
              FlyEmDataReader::ReadBodyAnnotation(reader, bodyId);

          if (!annotation.isEmpty()) {
            dlg->loadBodyAnnotation(annotation);
          }

          if (dlg->exec() && dlg->getBodyId() == bodyId) {
            ZFlyEmProofUtil::AnnotateBody(
                            bodyId, dlg->getBodyAnnotation(), annotation,
                            getDataDocument(), getParent3DWindow());
//            getDataDocument()->annotateBody(bodyId, dlg->getBodyAnnotation());
          }
          getDataDocument()->checkInBodyWithMessage(
                bodyId, neutu::EBodySplitMode::NONE);
        } else {
          emit messageGenerated(
                getDataDocument()->getAnnotationFailureMessage(bodyId));
        }
      } else {
        emit messageGenerated(
              ZWidgetMessage(
                getDataDocument()->getBodyLockFailMessage(bodyId),
                neutu::EMessageType::INFORMATION,
                ZWidgetMessage::ETarget::TARGET_CUSTOM_AREA));
      }
    }
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
  for (QQueue<ZFlyEmBodyEvent>::const_iterator iter = m_eventQueue.begin();
       iter != m_eventQueue.end(); ++iter) {
    const ZFlyEmBodyEvent &event  = *iter;
    event.print();
  }
}

void ZFlyEmBody3dDoc::forceBodyUpdate()
{
  QSet<uint64_t> bodySet = getNormalBodySet();
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
            neutu::ELabelSource::ORIGINAL);
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
              tree, ZStackDocObjectUpdate::EAction::ADD_UNIQUE);
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
    ZDvidTarget target = getMainDvidReader().getDvidTarget();
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
    ZDvidTarget target = getMainDvidReader().getDvidTarget();
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
          neutu::ELabelSource::ORIGINAL);
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
          neutu::ELabelSource::ORIGINAL);
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
  QElapsedTimer timer;
  timer.start();
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

  //Update data structures used by isTarMode().
//  for (auto& it : m_tarIdToMeshIds) {
//    auto& meshIds = it.second;
//    meshIds.erase(obj->getLabel());
//    if (meshIds.empty()) {
//      m_tarIdToMeshIds.erase(it.first);
//      break;
//    }
//  }

  if (obj->hasRole(ZStackObjectRole::ROLE_SUPERVOXEL)) {
    getBodyManager().eraseSupervoxel(obj->getLabel());
  }

//  ZOUT(LTRACE(), 5) << obj << "dumped" << obj->getSource();

  m_garbageJustDumped = true;

  ZOUT(KLOG, 5) << ZLog::Profile()
       << ZLog::Description(QString("Object (%1) dump time").
                            arg(obj->getSource().c_str()).toStdString())
       << ZLog::Duration(timer.elapsed());
}



void ZFlyEmBody3dDoc::diagnose() const
{
  ZStackDoc::diagnose();

  QList<ZMesh*> meshList = ZStackDocProxy::GetGeneralMeshList(this);
  LDEBUG() << "#General meshes" << meshList.size();

  if (getDataDocument() != NULL) {
    getDataDocument()->diagnose();
  }
}

ZStackDoc3dHelper* ZFlyEmBody3dDoc::getHelper() const
{
  return m_helper.get();
}

struct SynapseConnFilter {
  std::unordered_set<uint64_t> m_input;
  std::unordered_set<uint64_t> m_output;
  neutu::EBiDirection m_direction = neutu::EBiDirection::FORWARD;

  static std::unordered_set<uint64_t> CreateBodySet(const QString &str) {
    std::unordered_set<uint64_t> bodySet;
    auto bodyArray = ZString(str.toStdString()).toUint64Array();
    bodySet.insert(bodyArray.begin(), bodyArray.end());
    return bodySet;
  }

  void parse(const QString &str) {
//    m_input = 0;
//    m_output = 0;
    if (str.contains("->")) {
      m_direction = neutu::EBiDirection::FORWARD;
      QStringList tokens = str.split("->", QString::SkipEmptyParts);
      if (tokens.size() > 1) {
        m_input = CreateBodySet(tokens[0]);
        m_output = CreateBodySet(tokens[1]);
      }
    } else if (str.contains("<-")) {
      m_direction = neutu::EBiDirection::BACKWARD;
      QStringList tokens = str.split("<-", QString::SkipEmptyParts);
      if (tokens.size() > 1) {
        m_input = CreateBodySet(tokens[1]);
        m_output = CreateBodySet(tokens[0]);
      }
    }
  }
};

void ZFlyEmBody3dDoc::addSynapseSelection(const QString &filter)
{
  SynapseConnFilter sf;
  sf.parse(filter);

  if (!sf.m_input.empty() && !sf.m_output.empty()) {
    ZStackDocAccessor::SetObjectSelection(
          this, ZStackObject::EType::PUNCTUM, [&](const ZStackObject *obj) {
      const ZPunctum *p = dynamic_cast<const ZPunctum*>(obj);
      if (p) {
        return flyem::HasConnecion(
              p->name(), sf.m_input, sf.m_output, sf.m_direction);
      }
      return false;
    }, true);
  }
}

void ZFlyEmBody3dDoc::addSynapseSelection(const QStringList &filter)
{
  for (const QString &f : filter) {
    addSynapseSelection(f);
  }
}

bool ZFlyEmBody3dDoc::_loadFile(const QString &filePath)
{
  bool succ = false;

  switch (ZFileType::FileType(filePath.toStdString())) {
  case ZFileType::EFileType::JSON: {
    ZObject3dScan *obj = flyem::LoadRoiFromJson(filePath.toStdString());
    ZMesh *mesh = ZMeshFactory::MakeMesh(*obj);
    mesh->setColor(128, 128, 128);
    mesh->pushObjectColor();
    mesh->setSource(obj->getSource());
//    mesh->addRole(ZStackObjectRole::ROLE_ROI);
    getDataBuffer()->addUpdate(mesh, ZStackDocObjectUpdate::EAction::ADD_UNIQUE);
    getDataBuffer()->deliver();
    delete obj;
  }
    break;
  default:
    ZStackDoc::_loadFile(filePath);
    break;
  }

  return succ;
}

/*
bool ZFlyEmBody3dDoc::allowingSplit(uint64_t bodyId) const
{
  if (getBodyEnv()) {
    return getBodyEnv()->allowingSplit(bodyId);
  }

  return true;
}
*/

void ZFlyEmBody3dDoc::dumpGarbage(ZStackObject *obj, bool recycable)
{
  QMutexLocker locker(&m_garbageMutex);

//  m_garbageList.append(obj);
//  m_garbageMap[obj] = m_objectTime.elapsed();
  dumpGarbageUnsync(obj, recycable);
}

template <typename T>
void ZFlyEmBody3dDoc::removeBodyObject(bool recycling)
{
  if (is_recycable(T::GetType()) && recycling) {
    QList<T*> objList = getObjectList<T>();
    for (T* p : objList) {
      removeObject(p, false);
      dumpGarbage(p, true);
    }
  } else {
    removeObject(T::GetType(), true);
  }
}

void ZFlyEmBody3dDoc::dumpAllBody(bool recycling)
{
  cancelEventThread();

  beginObjectModifiedMode(EObjectModifiedMode::CACHE);

#ifdef _DEBUG_
  LKINFO << "Dump puncta";
#endif
//  ZOUT(LTRACE(), 5) << "Dump puncta";
  removeBodyObject<ZPunctum>(recycling);

#ifdef _DEBUG_
  LKINFO << "Dump todo list";
#endif

  removeBodyObject<ZFlyEmToDoItem>(recycling);

#ifdef _DEBUG_
  LKINFO << "Dump swc";
#endif
  removeBodyObject<ZSwcTree>(recycling);
//  QList<ZSwcTree*> treeList = getSwcList();
//  for (QList<ZSwcTree*>::const_iterator iter = treeList.begin();
//       iter != treeList.end(); ++iter) {
//    ZSwcTree *tree = *iter;
//    removeObject(tree, false);
//    dumpGarbage(tree, recycling);
//  }

#ifdef _DEBUG_
  LKINFO << "Dump meshes";
#endif
  QList<ZMesh*> meshList = ZStackDocProxy::GetNonRoiMeshList(this);
  for (ZMesh *mesh : meshList) {
    removeObject(mesh, false);
    dumpGarbage(mesh, recycling);
  }

  getBodyManager().clear();
//  m_bodySet.clear();
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
          getDataBuffer()->addUpdate(tree, ZStackDocObjectUpdate::EAction::KILL);
//          removeObject(tree, false);
        } else {
          targetTree->merge(tree);
          getDataBuffer()->addUpdate(tree, ZStackDocObjectUpdate::EAction::KILL);
        }
//        dumpGarbage(tree, false);
      }
    }
    getDataBuffer()->deliver();
//    removeEmptySwcTree(false);
  }
}


void ZFlyEmBody3dDoc::configure(const ProtocolTaskConfig &config)
{
  m_taskConfig = config;
}

/** Update bodies
 *
 * Currently there are three types of body IDs: normal body ID, tar body ID
 * and supervoxel ID. Any added ID will be stored in the document as its decoded
 * format, and the only way to tell its type is through how its mapped in the
 * body manager. One limitation in the body manager is that it can not store both
 * the normal form and the tar form of a body ID. Storing a supervoxel and a body
 * with the same decoded ID can also cause some confusion. So do NOT add different
 * encodings of the same ID together.
 */
template <typename InputIterator>
void ZFlyEmBody3dDoc::addBodyChangeEvent(
    const InputIterator &first, const InputIterator &last)
{
  std::cout << "Locking mutex ..." << std::endl;
  QMutexLocker locker(&m_eventQueueMutex);

  //Update event map and body set; the current event queue is cleared
  QMap<uint64_t, ZFlyEmBodyEvent> actionMap = makeEventMap(false, NULL);
  QSet<uint64_t> oldBodySet = getNormalBodySet();

//  m_eventQueue.clear();

  QSet<uint64_t> newBodySet;
  QSet<uint64_t> tarSet;
  QSet<uint64_t> decodedTarSet;
  QSet<uint64_t> supervoxelSet;
//  QMap<uint64_t, uint64_t> bodyEncodeMap;

#ifdef _DEBUG_
  std::string bodyStr;
  for (InputIterator iter = first; iter != last; ++iter) {
    bodyStr += std::to_string(*iter) + " ";
  }
  LDEBUG() << "Selection recieved:" << bodyStr;
#endif

  for (InputIterator iter = first; iter != last; ++iter) {
    uint64_t bodyId = *iter;
    if (ZFlyEmBodyManager::encodesTar(bodyId)) {
      tarSet.insert(bodyId);
      uint64_t decodedId = decode(bodyId);
      decodedTarSet.insert(decodedId);
//      newBodySet.insert(decodedId);
    } else if (ZFlyEmBodyManager::encodingSupervoxel(bodyId)) {
      supervoxelSet.insert(bodyId);
    } else {
      newBodySet.insert(decode(bodyId)); //The ID is always decoded while being added
    }
//    bodyEncodeMap[decode(bodyId)] = bodyId;
  }

  QSet<uint64_t> addedBodySet = newBodySet - oldBodySet;
  QSet<uint64_t> removedBodySet = oldBodySet - newBodySet - decodedTarSet;
  QSet<uint64_t> commonBodySet = newBodySet.intersect(oldBodySet);

  //Remove bodies not in the current set
  foreach (uint64_t bodyId, removedBodySet) {
    addEvent(ZFlyEmBodyEvent::EAction::REMOVE, bodyId, 0, NULL);
  }

  //Keep the event of common bodies if it's not removing the body
  foreach (uint64_t bodyId, commonBodySet) {
    if (actionMap.contains(bodyId)) {
      const ZFlyEmBodyEvent &bodyEvent = actionMap[bodyId];
      if (bodyEvent.getAction() != ZFlyEmBodyEvent::EAction::REMOVE) {
        addEvent(bodyEvent);
      }
    }
  }

  //Add new normal bodies
  foreach (uint64_t bodyId, addedBodySet) {
    if (!getBodyManager().isSupervoxel(bodyId)) {
      addEvent(ZFlyEmBodyEvent::EAction::ADD, bodyId, 0, NULL);
    } else {
      emit messageGenerated(
            ZWidgetMessage(
              QString("Adding a normal body ID that exists as a supervoxel: %1").arg(bodyId),
              neutu::EMessageType::WARNING,
              ZWidgetMessage::TARGET_TEXT_APPENDING |
              ZWidgetMessage::TARGET_KAFKA));
    }
  }

  //Process tar set
  foreach (uint64_t bodyId, tarSet) {
    if (ZFlyEmBodyManager::encodedLevel(bodyId) == 0) { //supervoxel tar
      if (!getBodyManager().hasMapping(bodyId)) {
        addEvent(ZFlyEmBodyEvent::EAction::ADD, bodyId, 0, NULL);
      }
    } else { //normal body
      if (!getBodyManager().contains(bodyId)) {
        addEvent(ZFlyEmBodyEvent::EAction::ADD, bodyId, 0, NULL);
      }
    }
  }

  /*
  foreach (uint64_t bodyId, tarSet) {
    if (!getBodyManager().contains(bodyId)) {
      addEvent(ZFlyEmBodyEvent::EAction::ACTION_ADD, bodyId, 0, NULL);
    }
  }
  */

  //Process supervoxel set
  QSet<uint64_t> addedSupervoxelSet =
      getBodyManager().getSupervoxelToAdd(supervoxelSet, true);
  foreach (uint64_t bodyId, addedSupervoxelSet) {
    addEvent(ZFlyEmBodyEvent::EAction::ADD, bodyId, 0, NULL);
  }

  QSet<uint64_t> removedSupervoxelSet =
      getBodyManager().getSupervoxelToRemove(supervoxelSet, true);
  foreach (uint64_t bodyId, removedSupervoxelSet) {
    addEvent(ZFlyEmBodyEvent::EAction::REMOVE, bodyId, 0, NULL);
  }

  QSet<uint64_t> commonSupervoxelSet = supervoxelSet - addedSupervoxelSet;
  foreach (uint64_t bodyId, commonSupervoxelSet) {
    if (actionMap.contains(bodyId)) {
      const ZFlyEmBodyEvent &bodyEvent = actionMap[bodyId];
      if (bodyEvent.getAction() != ZFlyEmBodyEvent::EAction::REMOVE) {
        addEvent(bodyEvent);
      }
    }
  }



#if 0
  for (InputIterator iter = first; iter != last; ++iter) {
    uint64_t bodyId = *iter;
    if (!actionMap.contains(bodyId)) { //If the action map has no such body id
      addEvent(ZFlyEmBodyEvent::EAction::ACTION_ADD, bodyId, 0, NULL);
    }
  }
#endif
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

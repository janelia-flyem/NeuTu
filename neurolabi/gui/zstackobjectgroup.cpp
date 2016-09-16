#include "zstackobjectgroup.h"

#include <QMutexLocker>
#include "QsLog/QsLog.h"
#include "neutubeconfig.h"

ZStackObjectGroup::ZStackObjectGroup() : m_currentZOrder(0)
{
}

ZStackObjectGroup::ZStackObjectGroup(const ZStackObjectGroup &group)
{
  m_objectList = group.m_objectList;
  m_sortedGroup = group.m_sortedGroup;
  m_selectedSet = group.m_selectedSet;
  m_currentZOrder = group.m_currentZOrder;
}

ZStackObjectGroup& ZStackObjectGroup::operator= (const ZStackObjectGroup &group)
{
  m_objectList = group.m_objectList;
  m_sortedGroup = group.m_sortedGroup;
  m_selectedSet = group.m_selectedSet;
  m_currentZOrder = group.m_currentZOrder;

  return *this;
}

void ZStackObjectGroup::moveTo(ZStackObjectGroup &group)
{
  QMutexLocker locker(&m_mutex);
  QMutexLocker locker2(group.getMutex());

  group.m_objectList.append(m_objectList);
  for (TObjectListMap::iterator iter = m_sortedGroup.begin();
       iter != m_sortedGroup.end(); ++iter) {
    group.m_sortedGroup[iter.key()].append(iter.value());
  }
//  group.m_sortedGroup = m_sortedGroup;

  for (TObjectSetMap::iterator iter = m_selectedSet.begin();
       iter != m_selectedSet.end(); ++iter) {
    group.m_selectedSet[iter.key()].unite(iter.value());
  }
//  group.m_selectedSet = m_selectedSet;
  group.m_currentZOrder = std::max(group.m_currentZOrder, m_currentZOrder);

  m_objectList.clear();
  m_sortedGroup.clear();
  m_selectedSet.clear();
  m_currentZOrder = 0;
}

ZStackObjectGroup::~ZStackObjectGroup()
{
  removeAllObject(true);
}

int ZStackObjectGroup::getMaxZOrderUnsync() const
{
  int maxZOrder = 0;

  for (QList<ZStackObject*>::const_iterator iter = m_objectList.begin();
       iter != m_objectList.end(); ++iter)
  {
    const ZStackObject *obj = *iter;
    if (maxZOrder == 0) {
      maxZOrder = obj->getZOrder();
    } else {
      if (obj->getZOrder() > maxZOrder) {
        maxZOrder = obj->getZOrder();
      }
    }
  }

  return maxZOrder;
}

int ZStackObjectGroup::getMaxZOrder() const
{
  QMutexLocker locker(&m_mutex);

  return getMaxZOrderUnsync();
}

const ZStackObject* ZStackObjectGroup::getLastObject(
    ZStackObject::EType type) const
{
  QMutexLocker locker(&m_mutex);

  return getLastObjectUnsync(type);
}

const ZStackObject* ZStackObjectGroup::getLastObjectUnsync(
    ZStackObject::EType type) const
{
  if (NeutubeConfig::GetVerboseLevel() >= 5) {
    LINFO() << "Getting last object";
  }

  ZStackObject *obj = NULL;
  const TStackObjectList &objList = getObjectListUnsync(type);
  if (!objList.isEmpty()) {
    obj = objList.back();
  }

  /*
  if (!getObjectList(type).empty()) {
    obj = getObjectList(type).back();
  }
  */

  return obj;
}


void ZStackObjectGroup::setSelected(ZStackObject *obj, bool selected)
{
  if (NeutubeConfig::GetVerboseLevel() >= 5) {
    LINFO() << "Getting selected";
  }

  if (obj != NULL) {
    //obj->setSelected(selected);
    getSelector()->setSelection(obj, selected);
    if (selected) {
      m_selectedSet[obj->getType()].insert(obj);
    } else {
      m_selectedSet[obj->getType()].remove(obj);
    }
  }
}

void ZStackObjectGroup::setSelected(bool selected)
{
  QMutexLocker locker(&m_mutex);

  setSelectedUnsync(selected);
}

void ZStackObjectGroup::setSelectedUnsync(bool selected)
{
  ZOUT(LTRACE(), 5) << "Select object";

  for (QList<ZStackObject*> ::iterator iter = m_objectList.begin();
       iter != m_objectList.end(); ++iter) {
    ZStackObject *obj = *iter;
    getSelector()->setSelection(obj, selected);
    //obj->setSelected(selected);
    getSelectedSetUnsync(obj->getType()).insert(obj);
  }

  if (selected == false) {
    for (TObjectSetMap::iterator iter = m_selectedSet.begin();
         iter != m_selectedSet.end(); ++iter) {
      TStackObjectSet &subset = *iter;
      subset.clear();
    }
  }
}

void ZStackObjectGroup::setSelectedUnsync(ZStackObject::EType type, bool selected)
{
  ZOUT(LTRACE(), 5) << "Select object by type";

  TStackObjectList &objList = getObjectListUnsync(type);
  TStackObjectSet &selectedSet = getSelectedSetUnsync(type);

  for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
       ++iter) {
    ZStackObject *obj = *iter;
    if (obj->isSelected() != selected) {
      getSelector()->setSelection(obj, selected);
      //obj->setSelected(selected);
      if (selected) {
        selectedSet.insert(obj);
      }
    }
  }

  if (!selected) {
    selectedSet.clear();
  }
}

void ZStackObjectGroup::setSelected(ZStackObject::EType type, bool selected)
{
  QMutexLocker locker(&m_mutex);

  setSelectedUnsync(type, selected);
}

void ZStackObjectGroup::deselectAll()
{
  QMutexLocker locker(&m_mutex);
  deselectAllUnsync();
}

void ZStackObjectGroup::deselectAllUnsync()
{
  ZOUT(LTRACE(), 5) << "Deselect all objects";

  for (TObjectSetMap::iterator iter = m_selectedSet.begin();
       iter != m_selectedSet.end(); ++iter) {
    TStackObjectSet &selectedSet = iter.value();
    for (TStackObjectSet::iterator objIter = selectedSet.begin();
         objIter != selectedSet.end(); ++objIter) {
      ZStackObject *obj = *objIter;
      obj->setSelected(false);
    }
  }
  m_selectedSet.clear();
  m_selector.deselectAll();
}

ZStackObject* ZStackObjectGroup::takeUnsync(ZStackObject *obj)
{
  ZOUT(LTRACE(), 5) << "Taking object";

  ZStackObject *found = NULL;
  if (m_objectList.removeOne(obj)) {
    found = obj;

    //Process subset
    getObjectListUnsync(obj->getType()).removeOne(obj);
    //remove_p(getSet(obj->getType()), obj);

    getSelectedSetUnsync(obj->getType()).remove(obj);
  }

  return found;
}

ZStackObject* ZStackObjectGroup::take(ZStackObject *obj)
{
  QMutexLocker locker(&m_mutex);

  return takeUnsync(obj);
}

TStackObjectList ZStackObjectGroup::takeUnsync(TObjectTest testFunc)
{
  ZOUT(LTRACE(), 5) << "Taking object";

  if (testFunc == NULL) { //Shouldn't happen. Just let it crash.
    LERROR() << "Null testing function";
  }

  TStackObjectList objSet;

  for (QList<ZStackObject*>::iterator iter = m_objectList.begin();
       iter != m_objectList.end(); ++iter) {
    ZStackObject *obj = *iter;
    if (testFunc(obj)) {
      objSet.append(obj);
    }
  }

  removeObjectUnsync(objSet.begin(), objSet.end(), false);

  return objSet;
}

TStackObjectList ZStackObjectGroup::take(TObjectTest testFunc)
{
  QMutexLocker locker(&m_mutex);

  return takeUnsync(testFunc);
}

TStackObjectList ZStackObjectGroup::takeUnsync(
    ZStackObject::EType type, TObjectTest testFunc)
{
  ZOUT(LTRACE(), 5) << "Taking object by type";

  if (testFunc == NULL) { //Shouldn't happen. Just let it crash.
    LERROR() << "Null testing function";
  }

  TStackObjectList objSet;

  TStackObjectList &objList = getObjectListUnsync(type);
  for (TStackObjectList::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZStackObject *obj = *iter;
    if (testFunc(obj)) {
      objSet.append(obj);
    }
  }

  removeObjectUnsync(objSet.begin(), objSet.end(), false);

  return objSet;
}

TStackObjectList ZStackObjectGroup::take(
    ZStackObject::EType type, TObjectTest testFunc)
{
  QMutexLocker locker(&m_mutex);
  return takeUnsync(type, testFunc);
}

TStackObjectList ZStackObjectGroup::takeSameSourceUnsync(
    ZStackObject::EType type, const std::string &source)
{
  ZOUT(LTRACE(), 5) << "Taking object by source";

  TStackObjectList objList;

  if (!source.empty()) {
    objList = findSameSourceUnsync(type, source);
    removeObjectUnsync(objList.begin(), objList.end(), false);
  }

  return objList;
}

TStackObjectList ZStackObjectGroup::takeSameSource(
    ZStackObject::EType type, const std::string &source)
{
  QMutexLocker locker(&m_mutex);

  return takeSameSourceUnsync(type, source);
}

TStackObjectList ZStackObjectGroup::takeUnsync(ZStackObject::EType type)
{
  ZOUT(LTRACE(), 5) << "Taking object by type";

  TStackObjectList objSet = getObjectListUnsync(type);
  if (!objSet.empty()) {
    QMutableListIterator<ZStackObject*> miter(m_objectList);
    while (miter.hasNext()) {
      if (objSet.contains(miter.next())) {
        miter.remove();
      }
    }
  }

  getObjectListUnsync(type).clear();

  return objSet;
}

TStackObjectList ZStackObjectGroup::take(ZStackObject::EType type)
{
  QMutexLocker locker(&m_mutex);

  return takeUnsync(type);
}

bool ZStackObjectGroup::remove_p(TStackObjectSet &objSet, ZStackObject *obj)
{
  TStackObjectSet::iterator iter = objSet.find(obj);
  if (iter != objSet.end()) {
    objSet.erase(iter);

    return true;
  }

  return false;
}

bool ZStackObjectGroup::removeObjectUnsync(ZStackObject *obj, bool isDeleting)
{
  ZOUT(LTRACE(), 5) << "Removing object. Deleting:" << isDeleting;

  ZStackObject *found = takeUnsync(obj);

  if (isDeleting) {
    delete found;
  }

  return found != NULL;
}

bool ZStackObjectGroup::removeObject(ZStackObject *obj, bool isDeleting)
{
  QMutexLocker locker(&m_mutex);

  return removeObjectUnsync(obj, isDeleting);
}

bool ZStackObjectGroup::removeObjectUnsync(ZStackObject::EType type, bool deleting)
{
  ZOUT(LTRACE(), 5) << "Removing object by type. Deleting" << deleting;

  TStackObjectList objSet = takeUnsync(type);
  if (deleting) {
    for (TStackObjectList::iterator iter = objSet.begin(); iter != objSet.end();
         ++iter) {
      delete *iter;
    }
  }

  return !objSet.empty();
}

bool ZStackObjectGroup::removeObject(ZStackObject::EType type, bool deleting)
{
  QMutexLocker locker(&m_mutex);

  return removeObjectUnsync(type, deleting);
}

bool ZStackObjectGroup::removeObjectUnsync(
    const TStackObjectSet &objSet, bool deleting)
{
  ZOUT(LTRACE(), 5) << "Removing object set. Deleting" << deleting;

  bool removed = false;

  if (!objSet.empty()) {
    QMutableListIterator<ZStackObject*> miter(m_objectList);
    while (miter.hasNext()) {
      ZStackObject *obj = miter.next();
      if (objSet.contains(obj)) {
        miter.remove();
        getObjectListUnsync(obj->getType()).removeOne(obj);
        if (deleting) {
          delete obj;
        }
        removed = true;
      }
    }
  }

  return removed;
}

bool ZStackObjectGroup::removeObject(
    const TStackObjectSet &objSet, bool deleting)
{
  QMutexLocker locker(&m_mutex);

  return removeObjectUnsync(objSet, deleting);
}

bool ZStackObjectGroup::removeSelectedUnsync(bool deleting)
{
  ZOUT(LTRACE(), 5) << "Removing seleted objects. Deleting" << deleting;

  TStackObjectList objSet = takeSelectedUnsync();
  if (deleting) {
    for (TStackObjectList::iterator iter = objSet.begin(); iter != objSet.end();
         ++iter) {
      delete *iter;
    }
  }

  return !objSet.empty();
}

bool ZStackObjectGroup::removeSelected(bool deleting)
{
  QMutexLocker locker(&m_mutex);

  return removeSelectedUnsync(deleting);
}

bool ZStackObjectGroup::removeSelectedUnsync(
    ZStackObject::EType type, bool deleting)
{
  ZOUT(LTRACE(), 5) << "Removing seleted objects by type. Deleting" << deleting;

  TStackObjectList objSet = takeSelectedUnsync(type);
  if (deleting) {
    for (TStackObjectList::iterator iter = objSet.begin(); iter != objSet.end();
         ++iter) {
      delete *iter;
    }
  }

  return !objSet.empty();
}


bool ZStackObjectGroup::removeSelected(ZStackObject::EType type, bool deleting)
{
  QMutexLocker locker(&m_mutex);

  return removeSelectedUnsync(type, deleting);
}

void ZStackObjectGroup::removeAllObjectUnsync(bool deleting)
{
  ZOUT(LTRACE(), 5) << "Removing all objects. Deleting" << deleting;

  if (deleting) {
    for (QList<ZStackObject*>::iterator iter = m_objectList.begin();
         iter != m_objectList.end(); ++iter) {
      delete *iter;
    }
  }

  for (TObjectListMap::iterator iter = m_sortedGroup.begin();
       iter != m_sortedGroup.end(); ++iter) {
    TStackObjectList &subset = *iter;
    subset.clear();
  }

  m_objectList.clear();
}

void ZStackObjectGroup::removeAllObject(bool deleting)
{
  QMutexLocker locker(&m_mutex);

  removeAllObjectUnsync(deleting);
}

TStackObjectList& ZStackObjectGroup::getObjectListUnsync(
    ZStackObject::EType type)
{
  ZOUT(LTRACE(), 6) << "Getting object list:" << "type" << type;

  if (!m_sortedGroup.contains(type)) {
    m_sortedGroup[type] = TStackObjectList();
  }

  return  m_sortedGroup[type];
}

QList<ZStackObject*> ZStackObjectGroup::getObjectList(
    ZStackObjectRole::TRole role) const
{
  QMutexLocker locker(&m_mutex);

  return getObjectListUnsync(role);
}

QList<ZStackObject*> ZStackObjectGroup::getObjectListUnsync(
    ZStackObjectRole::TRole role) const
{
  QList<ZStackObject*> objList;

  for (QList<ZStackObject*>::const_iterator iter = m_objectList.begin();
       iter != m_objectList.end(); ++iter) {
    ZStackObject *obj = const_cast<ZStackObject*>(*iter);
    if (obj->hasRole(role)) {
      objList.append(obj);
    }
  }

  return objList;
}

TStackObjectList& ZStackObjectGroup::getObjectList(ZStackObject::EType type)
{
  QMutexLocker locker(&m_mutex);

  return getObjectListUnsync(type);
}

const TStackObjectList& ZStackObjectGroup::getObjectListUnsync(
    ZStackObject::EType type) const
{
  return dynamic_cast<const TStackObjectList&>(
        const_cast<ZStackObjectGroup&>(*this).getObjectListUnsync(type));
}

const TStackObjectList& ZStackObjectGroup::getObjectList(ZStackObject::EType type) const
{
  return dynamic_cast<const TStackObjectList&>(
        const_cast<ZStackObjectGroup&>(*this).getObjectList(type));
}

TStackObjectSet& ZStackObjectGroup::getSelectedSet(ZStackObject::EType type)
{
  if (!m_selectedSet.contains(type)) {
    m_selectedSet[type] = TStackObjectSet();
  }

  return  m_selectedSet[type];
}

TStackObjectSet& ZStackObjectGroup::getSelectedSetUnsync(
    ZStackObject::EType type)
{
  if (!m_selectedSet.contains(type)) {
    m_selectedSet[type] = TStackObjectSet();
  }

  return  m_selectedSet[type];
}

const TStackObjectSet& ZStackObjectGroup::getSelectedSet(
    ZStackObject::EType type) const
{
  return dynamic_cast<const TStackObjectSet&>(
        const_cast<ZStackObjectGroup&>(*this).getSelectedSet(type));
}

const TStackObjectSet& ZStackObjectGroup::getSelectedSetUnsync(
    ZStackObject::EType type) const
{
  return dynamic_cast<const TStackObjectSet&>(
        const_cast<ZStackObjectGroup&>(*this).getSelectedSetUnsync(type));
}

TStackObjectSet ZStackObjectGroup::getObjectSetUnsync(ZStackObject::EType type) const
{
  TStackObjectSet objSet;
  const TStackObjectList &objList = getObjectListUnsync(type);
  objSet.fromList(objList);

  return objSet;
}

TStackObjectSet ZStackObjectGroup::getObjectSet(ZStackObject::EType type) const
{
  QMutexLocker locker(&m_mutex);

  return getObjectSetUnsync(type);
}

ZStackObject* ZStackObjectGroup::findFirstSameSourceUnsync(
    const ZStackObject *obj) const
{
  const TStackObjectList &objList = getObjectListUnsync(obj->getType());
  for (TStackObjectList::const_iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    const ZStackObject *checkObj = *iter;
    if (checkObj->fromSameSource(obj)) {
      return const_cast<ZStackObject*>(checkObj);
    }
  }

  return NULL;
}

ZStackObject* ZStackObjectGroup::findFirstSameSource(
    const ZStackObject *obj) const
{
  QMutexLocker locker(&m_mutex);

  return findFirstSameSourceUnsync(obj);
}

ZStackObject* ZStackObjectGroup::findFirstSameSourceUnsync(
    ZStackObject::EType type, const std::string &source) const
{
  const TStackObjectList &objList = getObjectListUnsync(type);
  for (TStackObjectList::const_iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    const ZStackObject *checkObj = *iter;
    if (checkObj->isSameSource(checkObj->getSource(), source)) {
      return const_cast<ZStackObject*>(checkObj);
    }
  }

  return NULL;
}

ZStackObject* ZStackObjectGroup::findFirstSameSource(
    ZStackObject::EType type, const std::string &source) const
{
  QMutexLocker locker(&m_mutex);

  return findFirstSameSourceUnsync(type, source);
}

TStackObjectList ZStackObjectGroup::findSameSourceUnsync(
    const ZStackObject *obj) const
{
  QList<ZStackObject*> objList;
  const TStackObjectList &fullObjList = getObjectListUnsync(obj->getType());
  for (TStackObjectList::const_iterator iter = fullObjList.begin();
       iter != fullObjList.end(); ++iter) {
    const ZStackObject *checkObj = *iter;
    if (checkObj->fromSameSource(obj)) {
      objList.append(const_cast<ZStackObject*>(checkObj));
    }
  }

  return objList;
}

TStackObjectList ZStackObjectGroup::findSameSource(
    const ZStackObject *obj) const
{
  QMutexLocker locker(&m_mutex);

  return findSameSourceUnsync(obj);
}

TStackObjectList ZStackObjectGroup::findSameSourceUnsync(
    const std::string &source) const
{
  QList<ZStackObject*> objList;
  if (!source.empty()) {
//    const TStackObjectList &fullObjList = getObjectList(obj->getType());
    for (QList<ZStackObject*>::const_iterator iter = m_objectList.begin();
         iter != m_objectList.end(); ++iter) {
      const ZStackObject *checkObj = *iter;
      if (checkObj->getSource() == source) {
        objList.append(const_cast<ZStackObject*>(checkObj));
      }
    }
  }

  return objList;
}

TStackObjectList ZStackObjectGroup::findSameSource(
    const std::string &source) const
{
  QMutexLocker locker(&m_mutex);

  return findSameSourceUnsync(source);
}

TStackObjectList ZStackObjectGroup::findSameSourceUnsync(
    ZStackObject::EType type, const std::string &source) const
{
  QList<ZStackObject*> objList;
  const TStackObjectList &fullObjList = getObjectListUnsync(type);
  for (TStackObjectList::const_iterator iter = fullObjList.begin();
       iter != fullObjList.end(); ++iter) {
    const ZStackObject *checkObj = *iter;
    if (ZStackObject::isSameSource(checkObj->getSource(), source)) {
      objList.append(const_cast<ZStackObject*>(checkObj));
    }
  }

  return objList;
}

TStackObjectList ZStackObjectGroup::findSameSource(
    ZStackObject::EType type, const std::string &source) const
{
  QMutexLocker locker(&m_mutex);

  return findSameSourceUnsync(type, source);
}

ZStackObject* ZStackObjectGroup::replaceFirstSameSourceUnsync(ZStackObject *obj)
{
  TStackObjectList &objList = getObjectListUnsync(obj->getType());
  for (TStackObjectList::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZStackObject *checkObj = *iter;
    if (checkObj->fromSameSource(obj)) {
      *iter = obj;
      return checkObj;
    }
  }

  return NULL;
}

ZStackObject* ZStackObjectGroup::replaceFirstSameSource(ZStackObject *obj)
{
  QMutexLocker locker(&m_mutex);

  return replaceFirstSameSourceUnsync(obj);
}

TStackObjectList ZStackObjectGroup::findSameClassUnsync(
    ZStackObject::EType type, const std::string &objClass)
{
  QList<ZStackObject*> objList;
  const TStackObjectList &fullObjList = getObjectListUnsync(type);
  for (TStackObjectList::const_iterator iter = fullObjList.begin();
       iter != fullObjList.end(); ++iter) {
    const ZStackObject *checkObj = *iter;
    if (ZStackObject::isSameClass(checkObj->getObjectClass(), objClass)) {
      objList.append(const_cast<ZStackObject*>(checkObj));
    }
  }

  return objList;
}

bool ZStackObjectGroup::isEmpty() const
{
  return m_objectList.isEmpty();
}

TStackObjectList ZStackObjectGroup::findSameClass(
    ZStackObject::EType type, const std::string &objClass)
{
  QMutexLocker locker(&m_mutex);

  return findSameClassUnsync(type, objClass);
}

void ZStackObjectGroup::addUnsync(const ZStackObject *obj, bool uniqueSource)
{
  addUnsync(const_cast<ZStackObject*>(obj), uniqueSource);
}

void ZStackObjectGroup::add(const ZStackObject *obj, bool uniqueSource)
{
  add(const_cast<ZStackObject*>(obj), uniqueSource);
}

#define ZSTACKOBECTGROUP_MAX_ZORDER INT_MAX

void ZStackObjectGroup::addUnsync(ZStackObject *obj, bool uniqueSource)
{
  if (obj != NULL && !containsUnsync(obj)) {
    if (m_currentZOrder >= ZSTACKOBECTGROUP_MAX_ZORDER) {
      compressZOrderUnsync();
    }
    int zOrder = obj->getZOrder();
    if (uniqueSource) {
      QList<ZStackObject*> objList = findSameSourceUnsync(obj);
      if (!objList.isEmpty()) {
        zOrder = objList.front()->getZOrder();
        removeObjectUnsync(objList.begin(), objList.end(), true);
      } else {
        zOrder = ++m_currentZOrder;
      }
    } else {
      zOrder = ++m_currentZOrder;
    }
    obj->setZOrder(zOrder);
    m_objectList.append(obj);
    if (obj->isSelected()) {
      m_selectedSet[obj->getType()].insert(obj);
    }
    getObjectListUnsync(obj->getType()).append(const_cast<ZStackObject*>(obj));
  }
}

void ZStackObjectGroup::add(ZStackObject *obj, bool uniqueSource)
{
  QMutexLocker locker(&m_mutex);

  addUnsync(obj, uniqueSource);
}

void ZStackObjectGroup::addUnsync(ZStackObject *obj, int zOrder, bool uniqueSource)
{
  if (obj != NULL) {
    obj->setZOrder(zOrder);
    if (zOrder > m_currentZOrder) {
      m_currentZOrder = zOrder;
    }

    if (!containsUnsync(obj)) {
      if (uniqueSource) {
        QList<ZStackObject*> objList = findSameSourceUnsync(obj);
        if (!objList.isEmpty()) {
          removeObjectUnsync(objList.begin(), objList.end(), true);
        }
      }
      m_objectList.append(obj);
      if (obj->isSelected()) {
        m_selectedSet[obj->getType()].insert(obj);
      }
      getObjectListUnsync(obj->getType()).append(const_cast<ZStackObject*>(obj));
    }
  }
}

void ZStackObjectGroup::add(ZStackObject *obj, int zOrder, bool uniqueSource)
{
  QMutexLocker locker(&m_mutex);

  addUnsync(obj, zOrder, uniqueSource);
}

bool ZStackObjectGroup::containsUnsync(const ZStackObject *obj) const
{
  if (obj == NULL) {
    return false;
  }

  if (m_sortedGroup.contains(obj->getType())) {
    return m_sortedGroup[obj->getType()].contains(
        const_cast<ZStackObject*>(obj));
  }

  return false;
}

bool ZStackObjectGroup::hasObjectUnsync(ZStackObject::EType type) const
{
  if (m_sortedGroup.contains(type)) {
    return !m_sortedGroup[type].empty();
  }

  return false;
}

bool ZStackObjectGroup::hasObject(ZStackObject::EType type) const
{
  QMutexLocker locker(&m_mutex);

  return hasObjectUnsync(type);
}

bool ZStackObjectGroup::hasObjectUnsync(ZStackObject::ETarget target) const
{
  for (QList<ZStackObject*>::const_iterator iter = m_objectList.begin();
       iter != m_objectList.end(); ++iter) {
    const ZStackObject *obj = *iter;
    if (obj->getTarget() == target) {
      return true;
    }
  }

  return false;
}

bool ZStackObjectGroup::hasObject(ZStackObject::ETarget target) const
{
  QMutexLocker locker(&m_mutex);

  return hasObjectUnsync(target);
}

bool ZStackObjectGroup::hasSelectedUnsync(ZStackObject::EType type) const
{
  return !getSelectedSetUnsync(type).empty();
}


bool ZStackObjectGroup::hasSelected(ZStackObject::EType type) const
{
  QMutexLocker locker(&m_mutex);

  return hasSelectedUnsync(type);
}

bool ZStackObjectGroup::hasSelected() const
{
  for (TObjectSetMap::const_iterator
       iter = m_selectedSet.begin(); iter != m_selectedSet.end();
       ++iter) {
    const TStackObjectSet &objSet = iter.value();
    if (!objSet.empty()) {
      return true;
    }
  }

  return false;
}

TStackObjectList ZStackObjectGroup::takeSelectedUnsync()
{
  for (TObjectSetMap::iterator iter = m_selectedSet.begin();
       iter != m_selectedSet.end(); ++iter) {
    TStackObjectSet &subset = *iter;
    subset.clear();
  }

  return takeUnsync(ZStackObject::isSelected);
}

TStackObjectList ZStackObjectGroup::takeSelectedUnsync(ZStackObject::EType type)
{
  TStackObjectList objSet;

  QMutableListIterator<ZStackObject*> miter(m_objectList);
  while (miter.hasNext()) {
    ZStackObject *obj = miter.next();
    if (obj->getType() == type && obj->isSelected()) {
      objSet.append(obj);
      miter.remove();
      //getObjectList(type).removeOne(obj);
    }
  }

  getObjectListUnsync(type).clear();
  getSelectedSetUnsync(type).clear();

  return objSet;
}

TStackObjectList ZStackObjectGroup::takeSelected()
{
  QMutexLocker locker(&m_mutex);

  return takeSelectedUnsync();
}

TStackObjectList ZStackObjectGroup::takeSelected(ZStackObject::EType type)
{
  QMutexLocker locker(&m_mutex);

  return takeSelectedUnsync(type);
}

TStackObjectList ZStackObjectGroup::getObjectListUnsync(
    ZStackObject::EType type, TObjectTest testFunc) const
{
  TStackObjectList objSet;
  const TStackObjectList &objList = getObjectListUnsync(type);
  for (TStackObjectList::const_iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZStackObject *obj = const_cast<ZStackObject*>(*iter);
    if (testFunc(obj)) {
      objSet.append(obj);
    }
  }

  return objSet;
}

TStackObjectList ZStackObjectGroup::getObjectList(
    ZStackObject::EType type, TObjectTest testFunc) const
{
  QMutexLocker locker(&m_mutex);

  return getObjectListUnsync(type, testFunc);
}

void ZStackObjectGroup::resetSelection()
{
  m_selector.reset();
}

QList<ZStackObject::EType> ZStackObjectGroup::getAllTypeUnsync() const
{
  QList<ZStackObject::EType> typeList;
  for (TObjectListMap::const_iterator iter = m_sortedGroup.begin();
       iter != m_sortedGroup.end(); ++iter) {
    if (!iter.value().isEmpty()) {
      typeList.append(iter.key());
    }
  }

  return typeList;
}


QList<ZStackObject::EType> ZStackObjectGroup::getAllType() const
{
  QMutexLocker locker(&m_mutex);

  return getAllTypeUnsync();
}

void ZStackObjectGroup::compressZOrderUnsync()
{
  std::set<int> zOrderSet;
  for (QList<ZStackObject*>::const_iterator iter = m_objectList.begin();
       iter != m_objectList.end(); ++iter) {
    ZStackObject *obj = *iter;
    zOrderSet.insert(obj->getZOrder());
  }

  QMap<int, int> zOrderMap;
  int index = 1;
  for (std::set<int>::const_iterator iter = zOrderSet.begin();
       iter != zOrderSet.end(); ++iter, ++index) {
    zOrderMap[*iter] = index;
    m_currentZOrder = index;
  }

  for (QList<ZStackObject*>::const_iterator iter = m_objectList.begin();
       iter != m_objectList.end(); ++iter) {
    ZStackObject *obj = *iter;
    obj->setZOrder(zOrderMap[obj->getZOrder()]);
  }
}

void ZStackObjectGroup::compressZOrder()
{
  QMutexLocker locker(&m_mutex);

  compressZOrderUnsync();
}

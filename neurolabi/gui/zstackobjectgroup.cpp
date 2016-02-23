#include "zstackobjectgroup.h"

#include <QMutexLocker>

ZStackObjectGroup::ZStackObjectGroup() : m_currentZOrder(0)
{
}

ZStackObjectGroup::~ZStackObjectGroup()
{
  removeAllObject(true);
}

int ZStackObjectGroup::getMaxZOrder() const
{
  int maxZOrder = 0;

  for (ZStackObjectGroup::const_iterator iter = begin(); iter != end(); ++iter)
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

const ZStackObject* ZStackObjectGroup::getLastObject(
    ZStackObject::EType type) const
{
  ZStackObject *obj = NULL;
  if (!getObjectList(type).empty()) {
    obj = getObjectList(type).back();
  }

  return obj;
}

void ZStackObjectGroup::setSelected(ZStackObject *obj, bool selected)
{
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
  for (ZStackObjectGroup::iterator iter = begin(); iter != end(); ++iter) {
    ZStackObject *obj = *iter;
    getSelector()->setSelection(obj, selected);
    //obj->setSelected(selected);
    getSelectedSet(obj->getType()).insert(obj);
  }

  if (selected == false) {
    for (TObjectSetMap::iterator iter = m_selectedSet.begin();
         iter != m_selectedSet.end(); ++iter) {
      TStackObjectSet &subset = *iter;
      subset.clear();
    }
  }
}

void ZStackObjectGroup::setSelected(ZStackObject::EType type, bool selected)
{
  TStackObjectList &objList = getObjectList(type);
  TStackObjectSet &selectedSet = getSelectedSet(type);

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

ZStackObject* ZStackObjectGroup::take(ZStackObject *obj, QMutex *mutex)
{
  QMutexLocker locker(mutex);

  ZStackObject *found = NULL;
  if (removeOne(obj)) {
    found = obj;

    //Process subset
    getObjectList(obj->getType()).removeOne(obj);
    //remove_p(getSet(obj->getType()), obj);

    getSelectedSet(obj->getType()).remove(obj);
  }

  return found;
}

TStackObjectList ZStackObjectGroup::take(TObjectTest testFunc, QMutex *mutex)
{
  QMutexLocker locker(mutex);

  TStackObjectList objSet;
  for (ZStackObjectGroup::iterator iter = begin(); iter != end();
       ++iter) {
    ZStackObject *obj = *iter;
    if (testFunc(obj)) {
      objSet.append(obj);
    }
  }

  removeObject(objSet.begin(), objSet.end(), false);

  return objSet;
}

TStackObjectList ZStackObjectGroup::take(
    ZStackObject::EType type, TObjectTest testFunc)
{
  TStackObjectList objSet;
  TStackObjectList &objList = getObjectList(type);
  for (TStackObjectList::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZStackObject *obj = *iter;
    if (testFunc(obj)) {
      objSet.append(obj);
    }
  }

  removeObject(objSet.begin(), objSet.end(), false);

  return objSet;
}

TStackObjectList ZStackObjectGroup::takeSameSource(
    ZStackObject::EType type, const std::string &source)
{
  TStackObjectList objList;

  if (!source.empty()) {
    objList = findSameSource(type, source);
    removeObject(objList.begin(), objList.end(), false);
  }

  return objList;
}

TStackObjectList ZStackObjectGroup::take(ZStackObject::EType type)
{
  TStackObjectList objSet = getObjectList(type);
  if (!objSet.empty()) {
    QMutableListIterator<ZStackObject*> miter(*this);
    while (miter.hasNext()) {
      if (objSet.contains(miter.next())) {
        miter.remove();
      }
    }
  }

  getObjectList(type).clear();

  return objSet;
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

bool ZStackObjectGroup::removeObject(ZStackObject *obj, bool isDeleting)
{
  ZStackObject *found = take(obj);

  if (isDeleting) {
    delete found;
  }

  return found != NULL;
}

bool ZStackObjectGroup::removeObject(ZStackObject::EType type, bool deleting)
{
  TStackObjectList objSet = take(type);
  if (deleting) {
    for (TStackObjectList::iterator iter = objSet.begin(); iter != objSet.end();
         ++iter) {
      delete *iter;
    }
  }

  return !objSet.empty();
}

bool ZStackObjectGroup::removeObject(
    const TStackObjectSet &objSet, bool deleting)
{
  bool removed = false;

  if (!objSet.empty()) {
    QMutableListIterator<ZStackObject*> miter(*this);
    while (miter.hasNext()) {
      ZStackObject *obj = miter.next();
      if (objSet.contains(obj)) {
        miter.remove();
        getObjectList(obj->getType()).removeOne(obj);
        if (deleting) {
          delete obj;
        }
        removed = true;
      }
    }
  }

  return removed;
}

bool ZStackObjectGroup::removeSelected(bool deleting)
{
  TStackObjectList objSet = takeSelected();
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
  TStackObjectList objSet = takeSelected(type);
  if (deleting) {
    for (TStackObjectList::iterator iter = objSet.begin(); iter != objSet.end();
         ++iter) {
      delete *iter;
    }
  }

  return !objSet.empty();
}

void ZStackObjectGroup::removeAllObject(bool deleting)
{
  if (deleting) {
    for (ZStackObjectGroup::iterator iter = begin(); iter != end(); ++iter) {
      delete *iter;
    }
  }

  for (TObjectListMap::iterator iter = m_sortedGroup.begin();
       iter != m_sortedGroup.end(); ++iter) {
    TStackObjectList &subset = *iter;
    subset.clear();
  }

  clear();
}

TStackObjectList& ZStackObjectGroup::getObjectList(ZStackObject::EType type)
{
  if (!m_sortedGroup.contains(type)) {
    m_sortedGroup[type] = TStackObjectList();
  }

  return  m_sortedGroup[type];
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

const TStackObjectSet& ZStackObjectGroup::getSelectedSet(
    ZStackObject::EType type) const
{
  return dynamic_cast<const TStackObjectSet&>(
        const_cast<ZStackObjectGroup&>(*this).getSelectedSet(type));
}

TStackObjectSet ZStackObjectGroup::getObjectSet(ZStackObject::EType type) const
{
  TStackObjectSet objSet;
  const TStackObjectList &objList = getObjectList(type);
  objSet.fromList(objList);

  return objSet;
}

ZStackObject* ZStackObjectGroup::findFirstSameSource(
    const ZStackObject *obj) const
{
  const TStackObjectList &objList = getObjectList(obj->getType());
  for (ZStackObjectGroup::const_iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    const ZStackObject *checkObj = *iter;
    if (checkObj->fromSameSource(obj)) {
      return const_cast<ZStackObject*>(checkObj);
    }
  }

  return NULL;
}

ZStackObject* ZStackObjectGroup::findFirstSameSource(
    ZStackObject::EType type, const std::string &source) const
{
  const TStackObjectList &objList = getObjectList(type);
  for (ZStackObjectGroup::const_iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    const ZStackObject *checkObj = *iter;
    if (checkObj->isSameSource(checkObj->getSource(), source)) {
      return const_cast<ZStackObject*>(checkObj);
    }
  }

  return NULL;
}

TStackObjectList ZStackObjectGroup::findSameSource(
    const ZStackObject *obj) const
{
  QList<ZStackObject*> objList;
  const TStackObjectList &fullObjList = getObjectList(obj->getType());
  for (ZStackObjectGroup::const_iterator iter = fullObjList.begin();
       iter != fullObjList.end(); ++iter) {
    const ZStackObject *checkObj = *iter;
    if (checkObj->fromSameSource(obj)) {
      objList.append(const_cast<ZStackObject*>(checkObj));
    }
  }

  return objList;
}


TStackObjectList ZStackObjectGroup::findSameSource(
    const std::string &source) const
{
  QList<ZStackObject*> objList;
  if (!source.empty()) {
//    const TStackObjectList &fullObjList = getObjectList(obj->getType());
    for (ZStackObjectGroup::const_iterator iter = this->begin();
         iter != this->end(); ++iter) {
      const ZStackObject *checkObj = *iter;
      if (checkObj->getSource() == source) {
        objList.append(const_cast<ZStackObject*>(checkObj));
      }
    }
  }

  return objList;
}

TStackObjectList ZStackObjectGroup::findSameSource(
    ZStackObject::EType type, const std::string &source) const
{
  QList<ZStackObject*> objList;
  const TStackObjectList &fullObjList = getObjectList(type);
  for (ZStackObjectGroup::const_iterator iter = fullObjList.begin();
       iter != fullObjList.end(); ++iter) {
    const ZStackObject *checkObj = *iter;
    if (ZStackObject::isSameSource(checkObj->getSource(), source)) {
      objList.append(const_cast<ZStackObject*>(checkObj));
    }
  }

  return objList;
}

ZStackObject* ZStackObjectGroup::replaceFirstSameSource(ZStackObject *obj)
{
  TStackObjectList &objList = getObjectList(obj->getType());
  for (ZStackObjectGroup::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZStackObject *checkObj = *iter;
    if (checkObj->fromSameSource(obj)) {
      *iter = obj;
      return checkObj;
    }
  }

  return NULL;
}

TStackObjectList ZStackObjectGroup::findSameClass(
    ZStackObject::EType type, const std::string &objClass)
{
  QList<ZStackObject*> objList;
  const TStackObjectList &fullObjList = getObjectList(type);
  for (ZStackObjectGroup::const_iterator iter = fullObjList.begin();
       iter != fullObjList.end(); ++iter) {
    const ZStackObject *checkObj = *iter;
    if (ZStackObject::isSameClass(checkObj->getObjectClass(), objClass)) {
      objList.append(const_cast<ZStackObject*>(checkObj));
    }
  }

  return objList;
}

void ZStackObjectGroup::add(const ZStackObject *obj, bool uniqueSource)
{
  add(const_cast<ZStackObject*>(obj), uniqueSource);
}

#define ZSTACKOBECTGROUP_MAX_ZORDER INT_MAX

void ZStackObjectGroup::add(ZStackObject *obj, bool uniqueSource)
{
  if (obj != NULL) {
    if (m_currentZOrder >= ZSTACKOBECTGROUP_MAX_ZORDER) {
      compressZOrder();
    }
    int zOrder = obj->getZOrder();
    if (uniqueSource) {
      QList<ZStackObject*> objList = findSameSource(obj);
      if (!objList.isEmpty()) {
        zOrder = objList.front()->getZOrder();
        removeObject(objList.begin(), objList.end(), true);
      } else {
        zOrder = ++m_currentZOrder;
      }
    } else {
      zOrder = ++m_currentZOrder;
    }
    obj->setZOrder(zOrder);
    append(obj);
    getObjectList(obj->getType()).append(const_cast<ZStackObject*>(obj));
  }
}

void ZStackObjectGroup::add(ZStackObject *obj, int zOrder, bool uniqueSource)
{
  if (obj != NULL) {
    if (uniqueSource) {
      QList<ZStackObject*> objList = findSameSource(obj);
      if (!objList.isEmpty()) {
        removeObject(objList.begin(), objList.end(), true);
      }
    }
    obj->setZOrder(zOrder);
    if (zOrder > m_currentZOrder) {
      m_currentZOrder = zOrder;
    }
    append(obj);
    getObjectList(obj->getType()).append(const_cast<ZStackObject*>(obj));
  }
}

#if 0
QList<ZStackObject*> ZStackObjectGroup::addU(const ZStackObject *obj)
{
  QList<ZStackObject*> objList;
  if (obj != NULL) {
    obj = const_cast<ZStackObject*>(obj);
    objList = findSameSource(obj);
    if (!objList.empty()) {
      obj->setZOrder(objList.front()->getZOrder());
    }
    removeObject(objList.begin(), objList.end(), false);
    append(obj);
    getObjectList(obj->getType()).append(const_cast<ZStackObject*>(obj));
  }

  return objList;
}
#endif

#if 0
void ZStackObjectGroup::addInFront(
    ZStackObject *obj, bool uniqueSource, QMutex *mutex)
{
  QMutexLocker locker(mutex);

  if (obj != NULL) {
    m_currentZOrder = getMaxZOrder();
    obj->setZOrder(maxOrder + 1);
    add(obj, uniqueSource);
  }
}
#endif

bool ZStackObjectGroup::hasObject(ZStackObject::EType type) const
{
  return !getObjectList(type).empty();
}

bool ZStackObjectGroup::hasObject(ZStackObject::ETarget target) const
{
  for (ZStackObjectGroup::const_iterator iter = begin(); iter != end();
       ++iter) {
    const ZStackObject *obj = *iter;
    if (obj->getTarget() == target) {
      return true;
    }
  }

  return false;
}

bool ZStackObjectGroup::hasSelected(ZStackObject::EType type) const
{
  return !getSelectedSet(type).empty();
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

TStackObjectList ZStackObjectGroup::takeSelected()
{
  for (TObjectSetMap::iterator iter = m_selectedSet.begin();
       iter != m_selectedSet.end(); ++iter) {
    TStackObjectSet &subset = *iter;
    subset.clear();
  }

  return take(ZStackObject::isSelected);
}

TStackObjectList ZStackObjectGroup::takeSelected(ZStackObject::EType type)
{
  TStackObjectList objSet;

  QMutableListIterator<ZStackObject*> miter(*this);
  while (miter.hasNext()) {
    ZStackObject *obj = miter.next();
    if (obj->getType() == type && obj->isSelected()) {
      objSet.append(obj);
      miter.remove();
      //getObjectList(type).removeOne(obj);
    }
  }

  getObjectList(type).clear();
  getSelectedSet(type).clear();

  return objSet;
}

TStackObjectList ZStackObjectGroup::getObjectList(
    ZStackObject::EType type, TObjectTest testFunc) const
{
  TStackObjectList objSet;
  const TStackObjectList &objList = getObjectList(type);
  for (TStackObjectList::const_iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZStackObject *obj = const_cast<ZStackObject*>(*iter);
    if (testFunc(obj)) {
      objSet.append(obj);
    }
  }

  return objSet;
}

void ZStackObjectGroup::resetSelection()
{
  m_selector.reset();
}

QList<ZStackObject::EType> ZStackObjectGroup::getAllType() const
{
  return m_sortedGroup.keys();
}

void ZStackObjectGroup::compressZOrder()
{
  std::set<int> zOrderSet;
  for (const_iterator iter = begin(); iter != end(); ++iter) {
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

  for (const_iterator iter = begin(); iter != end(); ++iter) {
    ZStackObject *obj = *iter;
    obj->setZOrder(zOrderMap[obj->getZOrder()]);
  }
}

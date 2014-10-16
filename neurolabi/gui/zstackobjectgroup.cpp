#include "zstackobjectgroup.h"

ZStackObjectGroup::ZStackObjectGroup()
{
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

ZStackObject* ZStackObjectGroup::take(ZStackObject *obj)
{
  ZStackObject *found = NULL;
  if (remove(obj)) {
    found = obj;

    //Process subset
    remove_p(getSet(obj->getType()), obj);
  }

  return found;
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

void ZStackObjectGroup::removeAll(bool isDeleting)
{
  if (isDeleting) {
    for (ZStackObjectGroup::iterator iter = begin(); iter != end(); ++iter) {
      delete *iter;
    }
  }

  for (TObjectMap::iterator iter = m_sortedGroup.begin();
       iter != m_sortedGroup.end(); ++iter) {
    TStackObjectSet &subset = *iter;
    subset.clear();
  }

  clear();
}

TStackObjectSet& ZStackObjectGroup::getSet(ZStackObject::EType type)
{
  if (!m_sortedGroup.contains(type)) {
    m_sortedGroup[type] = TStackObjectSet();
  }

  return  m_sortedGroup[type];
}

const TStackObjectSet& ZStackObjectGroup::getSet(ZStackObject::EType type) const
{
  return dynamic_cast<const TStackObjectSet&>(
        const_cast<ZStackObjectGroup&>(*this).getSet(type));
}

void ZStackObjectGroup::add(const ZStackObject *obj)
{
  if (obj != NULL) {
    insert(const_cast<ZStackObject*>(obj));
    getSet(obj->getType()).insert(const_cast<ZStackObject*>(obj));
  }
}

void ZStackObjectGroup::addInFront(ZStackObject *obj)
{
  if (obj != NULL) {
    int maxOrder = getMaxZOrder();
    obj->setZOrder(maxOrder + 1);
    add(obj);
  }
}

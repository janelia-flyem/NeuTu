#include "zstackobjectselector.h"

#include <iostream>
#include <unordered_set>

ZStackObjectSelector::ZStackObjectSelector()
{
}

/*
void ZStackObjectSelector::reset()
{
  m_selectedSet.clear();
  m_deselectedSet.clear();
}
*/

void ZStackObjectSelector::selectObject(ZStackObject *obj)
{
  if (!obj->isSelected()) {
    obj->setSelected(true);
    if (m_deselectedSet.count(obj) > 0) {
      m_deselectedSet.erase(obj);
    } else {
      m_selectedSet.insert(obj);
    }
  }
}

void ZStackObjectSelector::deselectObject(ZStackObject *obj)
{
  if (obj->isSelected()) {
    obj->setSelected(false);
    if (m_selectedSet.count(obj) > 0) {
      m_selectedSet.erase(obj);
    } else {
      m_deselectedSet.insert(obj);
    }
  }
}

void ZStackObjectSelector::setSelection(ZStackObject *obj, bool selecting)
{
  if (selecting) {
    selectObject(obj);
  } else {
    deselectObject(obj);
  }
}

/*
void ZStackObjectSelector::deselectAll()
{
  for (std::set<const ZStackObject*>::const_iterator iter = m_selectedSet.begin();
       iter != m_selectedSet.end(); ++iter) {
    ZStackObject *obj = const_cast<ZStackObject*>(*iter);
    obj->setSelected(false);
  }
  m_deselectedSet.insert(m_selectedSet.begin(), m_selectedSet.end());
  m_selectedSet.clear();
}
*/

void ZStackObjectSelector::print() const
{
  std::cout << "Selected:" << std::endl;
  for (std::set<const ZStackObject*>::const_iterator iter = m_selectedSet.begin();
       iter != m_selectedSet.end(); ++iter) {
    std::cout << "  ";
    const ZStackObject *obj = *iter;
    std::cout << obj->getTypeName() << ", " << obj->getSource() << std::endl;
  }

  std::cout << "De-Selected:" << std::endl;
  for (std::set<const ZStackObject*>::const_iterator iter = m_deselectedSet.begin();
       iter != m_deselectedSet.end(); ++iter) {
    std::cout << "  ";
    const ZStackObject *obj = *iter;
    std::cout << obj->getTypeName() << ", " << obj->getSource() << std::endl;
  }
}

void ZStackObjectSelector::deselectAll()
{
  for (std::set<const ZStackObject*>::const_iterator iter = m_selectedSet.begin();
       iter != m_selectedSet.end(); ++iter) {
    ZStackObject *obj = const_cast<ZStackObject*>(*iter);
    obj->setSelected(false);
  }
  m_deselectedSet.insert(m_selectedSet.begin(), m_selectedSet.end());
  m_selectedSet.clear();
}

/*
bool ZStackObjectSelector::isInSelectedSet(const ZStackObject *obj) const
{
  return m_selectedSet.count(const_cast<ZStackObject*>(obj)) > 0;
}

bool ZStackObjectSelector::isInDeselectedSet(const ZStackObject *obj) const
{
  return m_deselectedSet.count(const_cast<ZStackObject*>(obj)) > 0;
}
*/

/*
bool ZStackObjectSelector::isEmpty() const
{
  return m_selectedSet.empty() && m_deselectedSet.empty();
}
*/

namespace {

template <typename T>
void remove_object_from_set(std::set<T> &s, std::function<bool(const T&)> pred)
{
  for (auto iter = s.begin(); iter != s.end();) {
    if (pred(*iter)) {
      s.erase(iter++);
    } else {
      ++iter;
    }
  }
}

}

void ZStackObjectSelector::removeObjectByType(ZStackObject::EType type)
{
  using TConstZStackObjectPointer = const ZStackObject*;
  auto pred = std::function<bool(const TConstZStackObjectPointer&)>(
        [&](const TConstZStackObjectPointer &obj) -> bool {
    return (obj->getType() == type);
  });

  remove_object_from_set(m_selectedSet, pred);
  remove_object_from_set(m_deselectedSet, pred);
}

std::vector<ZStackObject*> ZStackObjectSelector::getSelectedList(
    ZStackObject::EType type) const
{
  std::vector<ZStackObject*> objList;
  for (std::set<const ZStackObject*>::const_iterator iter = m_selectedSet.begin();
       iter != m_selectedSet.end(); ++iter) {
    ZStackObject *obj = const_cast<ZStackObject*>(*iter);
    if (obj->getType() == type) {
      objList.push_back(obj);
    }
  }

  return objList;
}

std::vector<ZStackObject*> ZStackObjectSelector::getDeselectedList(
    ZStackObject::EType type) const
{
  std::vector<ZStackObject*> objList;
  for (std::set<const ZStackObject*>::const_iterator iter = m_deselectedSet.begin();
       iter != m_deselectedSet.end(); ++iter) {
    ZStackObject *obj = const_cast<ZStackObject*>(*iter);
    if (obj->getType() == type) {
      objList.push_back(obj);
    }
  }

  return objList;
}

std::set<ZStackObject*> ZStackObjectSelector::getSelectedObjectSet(
    ZStackObject::EType type) const
{
  std::set<ZStackObject*> objList;
  for (std::set<const ZStackObject*>::const_iterator iter = m_selectedSet.begin();
       iter != m_selectedSet.end(); ++iter) {
    ZStackObject *obj = const_cast<ZStackObject*>(*iter);
    if (obj->getType() == type) {
      objList.insert(obj);
    }
  }

  return objList;
}

std::set<ZStackObject*> ZStackObjectSelector::getDeselectedObjectSet(
    ZStackObject::EType type) const
{
  std::set<ZStackObject*> objList;
  for (std::set<const ZStackObject*>::const_iterator iter = m_deselectedSet.begin();
       iter != m_deselectedSet.end(); ++iter) {
    ZStackObject *obj = const_cast<ZStackObject*>(*iter);
    if (obj->getType() == type) {
      objList.insert(obj);
    }
  }

  return objList;
}

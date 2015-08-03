#include "zstackobjectselector.h"

#include <iostream>

#include "zstackobject.h"

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

void ZStackObjectSelector::print() const
{
  std::cout << "Selected:" << std::endl;
  for (std::set<const ZStackObject*>::const_iterator iter = m_selectedSet.begin();
       iter != m_selectedSet.end(); ++iter) {
    std::cout << "  ";
    const ZStackObject *obj = *iter;
    std::cout << obj->className() << ", " << obj->getSource() << std::endl;
  }

  std::cout << "De-Selected:" << std::endl;
  for (std::set<const ZStackObject*>::const_iterator iter = m_deselectedSet.begin();
       iter != m_deselectedSet.end(); ++iter) {
    std::cout << "  ";
    const ZStackObject *obj = *iter;
    std::cout << obj->className() << ", " << obj->getSource() << std::endl;
  }
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

std::set<ZStackObject*> ZStackObjectSelector::getSelectedSet(
    ZStackObject::EType type)
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

std::set<ZStackObject*> ZStackObjectSelector::getDeselectedSet(
    ZStackObject::EType type)
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

#include "zselector.h"
#include <iostream>

template <typename T>
ZSelector<T>::ZSelector()
{
}

template <typename T>
void ZSelector<T>::reset()
{
  m_selectedSet.clear();
  m_deselectedSet.clear();
}

template <typename T>
void ZSelector<T>::reset(const std::set<T> &selected,
                         const std::set<T> &prevSelected)
{
    reset();

    for (typename std::set<T>::const_iterator iter = selected.begin();
         iter != selected.end(); ++iter) {
        if (prevSelected.count(*iter) == 0) {
            m_selectedSet.insert(*iter);
        }
    }

    for (typename std::set<T>::const_iterator iter = prevSelected.begin();
         iter != prevSelected.end(); ++iter) {
        if (selected.count(*iter) == 0) {
            m_deselectedSet.insert(*iter);
        }
    }
}

template <typename T>
void ZSelector<T>::selectObject(const T &obj)
{
  if (m_deselectedSet.count(obj) > 0) {
    m_deselectedSet.erase(obj);
  }

  m_selectedSet.insert(obj);
}

template <typename T>
void ZSelector<T>::deselectObject(const T &obj)
{
  if (m_selectedSet.count(obj) > 0) {
    m_selectedSet.erase(obj);
    m_deselectedSet.insert(obj);
  }
}

template<typename T>
void ZSelector<T>::setSelection(T obj, bool selecting)
{
    if (selecting) {
        selectObject(obj);
    } else {
        deselectObject(obj);
    }
}

template<typename T>
void ZSelector<T>::deselectAll()
{
  m_deselectedSet.insert(m_selectedSet.begin(), m_selectedSet.end());
  m_selectedSet.clear();
}

template <typename T>
std::vector<T> ZSelector<T>::getSelectedList() const
{
  std::vector<T> selected;
  selected.insert(selected.begin(), m_selectedSet.begin(), m_selectedSet.end());

  return selected;
}

template <typename T>
std::vector<T> ZSelector<T>::getDeselectedList() const
{
  std::vector<T> selected;
  selected.insert(selected.begin(), m_deselectedSet.begin(),
                  m_deselectedSet.end());

  return selected;
}

template <typename T>
const std::set<T> &ZSelector<T>::getSelectedSet() const
{
  return m_selectedSet;
}

template <typename T>
const std::set<T> &ZSelector<T>::getDeselectedSet() const
{
  return m_deselectedSet;
}

template <typename T>
bool ZSelector<T>::isEmpty() const
{
  return m_selectedSet.empty() && m_deselectedSet.empty();
}

template <typename T>
bool ZSelector<T>::isInSelectedSet(const T& obj) const
{
  return m_selectedSet.count(obj) > 0;
}

template <typename T>
bool ZSelector<T>::isInDeselectedSet(const T& obj) const
{
  return m_deselectedSet.count(obj) > 0;
}

template <typename T>
void ZSelector<T>::print() const
{
  std::cout << m_selectedSet.size() << " selected." << std::endl;
  std::cout << m_deselectedSet.size() << " deselected." << std::endl;
}
#if 0
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
  for (std::set<ZStackObject*>::const_iterator iter = m_selectedSet.begin();
       iter != m_selectedSet.end(); ++iter) {
    std::cout << "  ";
    ZStackObject *obj = *iter;
    std::cout << obj->className() << ", " << obj->getSource() << std::endl;
  }

  std::cout << "De-Selected:" << std::endl;
  for (std::set<ZStackObject*>::const_iterator iter = m_deselectedSet.begin();
       iter != m_deselectedSet.end(); ++iter) {
    std::cout << "  ";
    ZStackObject *obj = *iter;
    std::cout << obj->className() << ", " << obj->getSource() << std::endl;
  }
}

bool ZStackObjectSelector::isInSelectedSet(const ZStackObject *obj) const
{
  return m_selectedSet.count(const_cast<ZStackObject*>(obj)) > 0;
}

bool ZStackObjectSelector::isInDeselectedSet(const ZStackObject *obj) const
{
  return m_deselectedSet.count(const_cast<ZStackObject*>(obj)) > 0;
}

bool ZStackObjectSelector::isEmpty() const
{
  return m_selectedSet.empty() && m_deselectedSet.empty();
}

std::vector<ZStackObject*> ZStackObjectSelector::getSelectedList(
    ZStackObject::EType type) const
{
  std::vector<ZStackObject*> objList;
  for (std::set<ZStackObject*>::const_iterator iter = m_selectedSet.begin();
       iter != m_selectedSet.end(); ++iter) {
    ZStackObject *obj = *iter;
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
  for (std::set<ZStackObject*>::const_iterator iter = m_deselectedSet.begin();
       iter != m_deselectedSet.end(); ++iter) {
    ZStackObject *obj = *iter;
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
  for (std::set<ZStackObject*>::const_iterator iter = m_selectedSet.begin();
       iter != m_selectedSet.end(); ++iter) {
    ZStackObject *obj = *iter;
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
  for (std::set<ZStackObject*>::const_iterator iter = m_deselectedSet.begin();
       iter != m_deselectedSet.end(); ++iter) {
    ZStackObject *obj = *iter;
    if (obj->getType() == type) {
      objList.insert(obj);
    }
  }

  return objList;
}
#endif

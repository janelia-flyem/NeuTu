#ifndef ZSTACKOBJECTGROUP_H
#define ZSTACKOBJECTGROUP_H

#include <QList>
#include <QSet>
#include <QMap>
#include <set>
#include <QMutex>

#include "zstackobject.h"
#include "zstackobjectselector.h"
#include "zsharedpointer.h"

/*!
 * \brief The aggregate class of ZStackObject
 *
 * No NULL object is allowed to be included in the group. Objects with different
 * types are not considered as source conflict even if they have the same source.
 */
typedef QSet<ZStackObject*> TStackObjectSet;
typedef QList<ZStackObject*> TStackObjectList;
typedef bool (*TObjectTest)(const ZStackObject*);

class ZStackObjectGroup
{
public:
  ZStackObjectGroup();
  ~ZStackObjectGroup();

  typedef QMap<ZStackObject::EType, TStackObjectList> TObjectListMap;
  typedef QMap<ZStackObject::EType, TStackObjectSet> TObjectSetMap;

  /*!
   * \brief Get the max Z order
   *
   * \return 0 if the group is empty
   */
  int getMaxZOrder() const;

  void setSelected(ZStackObject *obj, bool selected);
  void setSelected(bool selected);

  void setSelected(ZStackObject::EType type, bool selected);

  void deselectAll();
  void deselectAllUnsync();

  bool isEmpty() const;

  /*!
   * \brief Reset selection recorder
   */
  void resetSelection();

  inline ZStackObjectSelector* getSelector() {
    return &m_selector;
  }

  QMutex* getMutex() const {
    return &m_mutex;
  }

  void moveTo(ZStackObjectGroup &group);

  const ZStackObject *getLastObject(ZStackObject::EType type) const;
  ZStackObject* findFirstSameSource(const ZStackObject *obj) const;
  ZStackObject* findFirstSameSource(
      ZStackObject::EType type, const std::string &source) const;


  TStackObjectList findSameSource(const std::string &str) const;
  TStackObjectList findSameSource(const ZStackObject *obj) const;
  TStackObjectList findSameSource(
      ZStackObject::EType type, const std::string &source) const;
  /*
  TStackObjectList findSameSourceClass(
      ZStackObject::EType type, const std::string &source);
      */

  TStackObjectList findSameClass(
      ZStackObject::EType type, const std::string &objClass);

  ZStackObject* replaceFirstSameSource(ZStackObject *obj);

  template<typename InputIterator>
  QList<ZStackObject*> findSameSource(
      const InputIterator begin, const InputIterator end) const;

  void add(ZStackObject *obj, bool uniqueSource);
  void add(const ZStackObject *obj, bool uniqueSource);
  void add(ZStackObject *obj, int zOrder, bool uniqueSource);
//  QList<ZStackObject*> addU(const ZStackObject *obj);

  template <typename InputIterator>
  void add(const InputIterator begin, const InputIterator end,
           bool uniqueSource);

//  void addInFront(ZStackObject *obj, bool uniqueSource, QMutex *mutex = NULL);

  /*!
   * \brief Take an object
   *
   * \a obj is removed from the group.
   *
   * \return \a obj if it is in the group. Otherwise it returns NULL.
   */
  ZStackObject* take(ZStackObject *obj);
  TStackObjectList take(TObjectTest testFunc);
  TStackObjectList take(ZStackObject::EType type, TObjectTest testFunc);
  TStackObjectList take(ZStackObject::EType type);
  TStackObjectList takeSelected();
  TStackObjectList takeSelected(ZStackObject::EType type);
  TStackObjectList takeSameSource(
      ZStackObject::EType type, const std::string &source);

  template <typename InputIterator>
  TStackObjectList take(const InputIterator &first, const InputIterator &last);


  /*!
   * \brief Remove an object
   *
   * Nothing will be done if \a obj is not in the group.
   *
   * \param isDeleting Delete the object or not.
   * \return true iff \a obj is removed.
   */
  bool removeObject(ZStackObject *obj, bool isDeleting = true);
  bool removeObject(ZStackObject::EType type, bool deleting = true);
  bool removeObject(const TStackObjectSet &objSet, bool deleting = true);

  bool removeSelected(bool deleting = true);
  bool removeSelected(ZStackObject::EType type, bool deleting = true);

  template <typename InputIterator>
  void removeObject(InputIterator begin, InputIterator end,
                    bool deleting = true);

  QList<ZStackObject*>& getObjectList() {
    return m_objectList;
  }

  const QList<ZStackObject*>& getObjectList() const {
    return m_objectList;
  }

  int size() const {
    return m_objectList.size();
  }

  /*!
   * \brief Remove all objects
   */
  void removeAllObject(bool deleting = true);

  TStackObjectList& getObjectList(ZStackObject::EType type);
  const TStackObjectList& getObjectList(ZStackObject::EType type) const;

  template<typename T>
  QList<T*> getObjectList() const;


  TStackObjectList getObjectList(ZStackObject::EType type,
                                 TObjectTest testFunc) const;

  TStackObjectSet& getSelectedSet(ZStackObject::EType type);
  const TStackObjectSet& getSelectedSet(ZStackObject::EType type) const;

  TStackObjectSet getObjectSet(ZStackObject::EType type) const;

  bool hasObject(ZStackObject::EType type) const;
  bool hasObject(ZStackObject::ETarget target) const;
  bool hasSelected() const;
  bool hasSelected(ZStackObject::EType type) const;

  QList<ZStackObject::EType> getAllType() const;

  void compressZOrder();

public:
  bool containsUnsync(const ZStackObject *obj) const;

  int getMaxZOrderUnsync() const;

  void setSelectedUnsync(bool selected);
  void setSelectedUnsync(ZStackObject::EType type, bool selected);

  const ZStackObject *getLastObjectUnsync(ZStackObject::EType type) const;
  ZStackObject* findFirstSameSourceUnsync(const ZStackObject *obj) const;
  TStackObjectList findSameSourceUnsync(const std::string &str) const;
  TStackObjectList findSameSourceUnsync(
      ZStackObject::EType type, const std::string &source) const;
  TStackObjectList findSameSourceUnsync(const ZStackObject *obj) const;
  ZStackObject* findFirstSameSourceUnsync(
      ZStackObject::EType type, const std::string &source) const;
  TStackObjectList findSameSourceClassUnsync(
      ZStackObject::EType type, const std::string &source);
  TStackObjectList findSameClassUnsync(
      ZStackObject::EType type, const std::string &objClass);

  ZStackObject* replaceFirstSameSourceUnsync(ZStackObject *obj);
  template<typename InputIterator>
  QList<ZStackObject*> findSameSourceUnsync(
      const InputIterator begin, const InputIterator end) const;

  void addUnsync(ZStackObject *obj, bool uniqueSource);
  void addUnsync(const ZStackObject *obj, bool uniqueSource);
  void addUnsync(ZStackObject *obj, int zOrder, bool uniqueSource);
//  QList<ZStackObject*> addU(const ZStackObject *obj);

  template <typename InputIterator>
  void addUnsync(const InputIterator begin, const InputIterator end,
           bool uniqueSource);


  ZStackObject* takeUnsync(ZStackObject *obj);
  TStackObjectList takeUnsync(TObjectTest testFunc);
  TStackObjectList takeUnsync(ZStackObject::EType type, TObjectTest testFunc);
  TStackObjectList takeUnsync(ZStackObject::EType type);
  TStackObjectList takeSelectedUnsync();
  TStackObjectList takeSelectedUnsync(ZStackObject::EType type);
  TStackObjectList takeSameSourceUnsync(
      ZStackObject::EType type, const std::string &source);

  template <typename InputIterator>
  TStackObjectList takeUnsync(
      const InputIterator &first, const InputIterator &last);

  bool removeObjectUnsync(ZStackObject *obj, bool isDeleting = true);
  bool removeObjectUnsync(ZStackObject::EType type, bool deleting = true);
  bool removeObjectUnsync(const TStackObjectSet &objSet, bool deleting = true);

  bool removeSelectedUnsync(bool deleting = true);
  bool removeSelectedUnsync(ZStackObject::EType type, bool deleting = true);

  template <typename InputIterator>
  void removeObjectUnsync(InputIterator begin, InputIterator end,
                    bool deleting = true);

  /*!
   * \brief Remove all objects
   */
  void removeAllObjectUnsync(bool deleting = true);

  TStackObjectSet getObjectSetUnsync(ZStackObject::EType type) const;
  TStackObjectList& getObjectListUnsync(ZStackObject::EType type);
  const TStackObjectList& getObjectListUnsync(ZStackObject::EType type)
  const;

  QList<ZStackObject*> getObjectListUnsync(ZStackObjectRole::TRole role) const;
  QList<ZStackObject*> getObjectList(ZStackObjectRole::TRole role) const;

  template<typename T>
  QList<T*> getObjectListUnsync() const;

  TStackObjectList getObjectListUnsync(ZStackObject::EType type,
                                 TObjectTest testFunc) const;

  TStackObjectSet& getSelectedSetUnsync(ZStackObject::EType type);
  const TStackObjectSet& getSelectedSetUnsync(ZStackObject::EType type) const;

  bool hasObjectUnsync(ZStackObject::EType type) const;
  bool hasObjectUnsync(ZStackObject::ETarget target) const;
  bool hasSelectedUnsync() const;
  bool hasSelectedUnsync(ZStackObject::EType type) const;

  QList<ZStackObject::EType> getAllTypeUnsync() const;

  void compressZOrderUnsync();

private:
  static bool remove_p(TStackObjectSet &objSet, ZStackObject *obj);
  ZStackObjectGroup(const ZStackObjectGroup &group);
  ZStackObjectGroup& operator= (const ZStackObjectGroup &group);

private:
  QList<ZStackObject*> m_objectList;
  TObjectListMap m_sortedGroup;
  TObjectSetMap m_selectedSet;
  int m_currentZOrder;

  mutable QMutex m_mutex;

  ZStackObjectSelector m_selector;
};

template <typename InputIterator>
void ZStackObjectGroup::addUnsync(
    const InputIterator begin, const InputIterator end, bool uniqueSource)
{
  for (InputIterator iter = begin; iter != end; ++iter) {
    addUnsync(*iter, uniqueSource);
  }
}

template <typename InputIterator>
void ZStackObjectGroup::add(
    const InputIterator begin, const InputIterator end, bool uniqueSource)
{
  QMutexLocker locker(&m_mutex);
  addUnsync(begin, end, uniqueSource);
}

template <typename InputIterator>
void ZStackObjectGroup::removeObjectUnsync(
    InputIterator begin, InputIterator end, bool deleting)
{
  TStackObjectSet objSet;

  for (InputIterator iter = begin; iter != end; ++iter) {
    objSet.insert(*iter);
  }

  removeObjectUnsync(objSet, deleting);
}

template <typename InputIterator>
void ZStackObjectGroup::removeObject(
    InputIterator begin, InputIterator end, bool deleting)
{
  QMutexLocker locker(&m_mutex);
  removeObjectUnsync(begin, end, deleting);
}

template<typename InputIterator>
TStackObjectList ZStackObjectGroup::findSameSourceUnsync(
    const InputIterator begin, const InputIterator end) const
{
  TStackObjectList objList;
  for (InputIterator iter = begin; iter != end; ++iter) {
    const ZStackObject *obj = *iter;
    if (!obj->getSource().empty()) {
      objList.append(findSameSourceUnsync(*iter));
    }
  }

  return objList;
}

template<typename InputIterator>
TStackObjectList ZStackObjectGroup::findSameSource(
    const InputIterator begin, const InputIterator end) const
{
  QMutexLocker locker(&m_mutex);

  return findSameSourceUnsync(begin, end);
}

template <typename InputIterator>
TStackObjectList ZStackObjectGroup::takeUnsync(
    const InputIterator &first, const InputIterator &last)
{
  TStackObjectSet objSet;

  for (InputIterator iter = first; iter != last; ++iter) {
    objSet.insert(*iter);
  }

  TStackObjectList objList;

  if (!objSet.empty()) {
    QMutableListIterator<ZStackObject*> miter(m_objectList);
    while (miter.hasNext()) {
      ZStackObject *obj = miter.next();
      if (objSet.contains(obj)) {
        miter.remove();
        getObjectListUnsync(obj->getType()).removeOne(obj);
        objList.append(obj);
      }
    }
  }

  return objList;
}

template <typename InputIterator>
TStackObjectList ZStackObjectGroup::take(
    const InputIterator &first, const InputIterator &last)
{
  QMutexLocker locker(&m_mutex);

  return takeUnsync(first, last);
}

template<typename T>
QList<T*> ZStackObjectGroup::getObjectListUnsync() const
{
  QList<T*> tList;

  TStackObjectList& objList =
      const_cast<TStackObjectList&>(getObjectListUnsync(T::GetType()));
  for (TStackObjectList::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    tList.append(dynamic_cast<T*>(*iter));
  }

  return tList;
}

template<typename T>
QList<T*> ZStackObjectGroup::getObjectList() const
{
  QMutexLocker locker(&m_mutex);

  return getObjectListUnsync<T>();
}

typedef ZSharedPointer<ZStackObjectGroup> ZStackObjectGroupPtr;

#endif // ZSTACKOBJECTGROUP_H

#ifndef ZSTACKOBJECTGROUP_H
#define ZSTACKOBJECTGROUP_H

#include <QList>
#include <QSet>
#include <QMap>
#include <set>

#include "zstackobject.h"

/*!
 * \brief The aggregate class of ZStackObject
 *
 * No NULL object is allowed to be included in the group.
 */
typedef std::set<ZStackObject*> TStackObjectSet;

class ZStackObjectGroup : QSet<ZStackObject*>
{
public:
  ZStackObjectGroup();
  typedef QMap<ZStackObject::EType, TStackObjectSet> TObjectMap;

  /*!
   * \brief Get the max Z order
   *
   * \return 0 if the group is empty
   */
  int getMaxZOrder() const;

  void add(const ZStackObject *obj);
  void addInFront(ZStackObject *obj);

  /*!
   * \brief Take an object
   *
   * \a obj is removed from the group.
   *
   * \return \a obj if it is in the group. Otherwise it returns NULL.
   */
  ZStackObject* take(ZStackObject *obj);

  /*!
   * \brief Remove an object
   *
   * Nothing will be done if \a obj is not in the group.
   *
   * \param isDeleting Delete the object or not.
   * \return true iff \a obj is removed.
   */
  bool removeObject(ZStackObject *obj, bool isDeleting = true);

  /*!
   * \brief Remove all objects
   */
  void removeAll(bool isDeleting = true);

  TStackObjectSet& getSet(ZStackObject::EType type);
  const TStackObjectSet& getSet(ZStackObject::EType type) const;

private:
  bool remove_p(TStackObjectSet &objSet, ZStackObject *obj);

private:
  TObjectMap m_sortedGroup;
};

#endif // ZSTACKOBJECTGROUP_H

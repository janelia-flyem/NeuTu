#ifndef ZSTACKOBJECTPAINTSORTER_H
#define ZSTACKOBJECTPAINTSORTER_H

#include <vector>
#include <QList>

#include "common/utilities.h"
#include "zstackobject.h"

/*!
 * \brief Sort objects for painting
 *
 * This class is designed to replace ZPaintBundle.
 */
class ZStackObjectPaintSorter
{
public:
  ZStackObjectPaintSorter();

  void add(ZStackObject *obj);
  void add(const QList<ZStackObject*> &objList);

  using SortingZ = neutu::Boolean;

  QList<ZStackObject*> getVisibleObjectList(
      neutu::data3d::ETarget target, const SortingZ &sortingZ) const;
  std::vector<QList<ZStackObject*>> getVisibleObjectLists(
      const SortingZ &sortingZ) const;

  void forEachVisibleTarget(
      std::function<
        void(neutu::data3d::ETarget, const QList<ZStackObject*> &)> f,
      const SortingZ &sortingZ);

  void forEachVisibleTarget(
      const std::vector<neutu::data3d::ETarget> &targetList,
      std::function<
        void(neutu::data3d::ETarget, const QList<ZStackObject*> &)> f,
      const SortingZ &sortingZ);

  void clear();
  bool isEmpty() const;

private:
  template<typename InputIterator>
  void add(const InputIterator &first, const InputIterator &last);

  const QList<ZStackObject*>& getObjectList(neutu::data3d::ETarget target) const;

private:
  std::vector<QList<ZStackObject*>> m_objList;
};

#endif // ZSTACKOBJECTPAINTSORTER_H

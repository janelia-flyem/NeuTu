#include "zstackobjectpaintsorter.h"

#include <algorithm>

ZStackObjectPaintSorter::ZStackObjectPaintSorter()
{
  m_objList.resize(neutu::data3d::TARGET_COUNT);
}

template<typename InputIterator>
void ZStackObjectPaintSorter::add(
    const InputIterator &first, const InputIterator &last)
{
  for (InputIterator iter = first; iter != last; ++iter) {
    add(*iter);
  }
}

void ZStackObjectPaintSorter::clear()
{
  for (auto &e : m_objList) {
    e.clear();
  }
}

bool ZStackObjectPaintSorter::isEmpty() const
{
  return std::all_of(
        m_objList.begin(), m_objList.end(),
        [](const QList<ZStackObject*> &objList) { return objList.isEmpty(); });
}

void ZStackObjectPaintSorter::add(ZStackObject *obj)
{
  m_objList[neutu::EnumToUnderlyingType(obj->getTarget())].append(obj);
}

void ZStackObjectPaintSorter::add(const QList<ZStackObject *> &objList)
{
  add(objList.begin(), objList.end());
}

const QList<ZStackObject*>&
ZStackObjectPaintSorter::getObjectList(neutu::data3d::ETarget target) const
{
  return m_objList[neutu::EnumToUnderlyingType(target)];
}

QList<ZStackObject*> ZStackObjectPaintSorter::getVisibleObjectList(
    neutu::data3d::ETarget target, const SortingZ &sortingZ) const
{
  auto &fullList = getObjectList(target);
  QList<ZStackObject*> objList;
  for (ZStackObject *obj : fullList) {
    if (obj->isVisible()) {
      objList.append(obj);
    }
  }

  if (sortingZ) {
    std::sort(objList.begin(), objList.end(), ZStackObject::ZOrderLessThan());
  }

  return objList;
}

std::vector<QList<ZStackObject*>> ZStackObjectPaintSorter::getVisibleObjectLists(
    const SortingZ &sortingZ) const
{
  std::vector<QList<ZStackObject*>> objLists(m_objList.size());

  for (size_t i = 0; i < m_objList.size(); ++i) {
    objLists[i] = getVisibleObjectList(
          static_cast<neutu::data3d::ETarget>(i), sortingZ);
  }

  return objLists;
}

void ZStackObjectPaintSorter::forEachVisibleTarget(
    std::function<
      void(neutu::data3d::ETarget, const QList<ZStackObject*>&)> f,
    const SortingZ &sortingZ)
{
  for (size_t i = 0; i < m_objList.size(); ++i) {
    neutu::data3d::ETarget target = static_cast<neutu::data3d::ETarget>(i);
    auto objList = getVisibleObjectList(target, sortingZ);
    f(target, objList);
  }
}

void ZStackObjectPaintSorter::forEachVisibleTarget(
      const std::vector<neutu::data3d::ETarget> &targetList,
      std::function<
        void(neutu::data3d::ETarget, const QList<ZStackObject*> &)> f,
      const SortingZ &sortingZ)
{
  for (auto target : targetList) {
    auto objList = getVisibleObjectList(target, sortingZ);
    f(target, objList);
  }
}

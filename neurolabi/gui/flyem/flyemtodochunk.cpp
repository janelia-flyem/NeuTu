#include "flyemtodochunk.h"

#include "neulib/core/stringbuilder.h"

#include "geometry/zintpoint.h"

FlyEmTodoChunk::FlyEmTodoChunk()
{
  m_invalidItem.invalidate();
}

void FlyEmTodoChunk::setReady(bool ready)
{
  m_isReady = ready;
}

bool FlyEmTodoChunk::isReady() const
{
  return m_isReady;
}

bool FlyEmTodoChunk::isValid() const
{
  return m_isValid;
}

void FlyEmTodoChunk::invalidate()
{
  m_isValid = false;
}

bool FlyEmTodoChunk::isEmpty() const
{
  return m_itemMap.size() == 0;
}

size_t FlyEmTodoChunk::countItem() const
{
  return m_itemMap.size();
}

FlyEmTodoChunk::KeyType FlyEmTodoChunk::GetItemKey(int x, int y, int z)
{
  return neulib::StringBuilder("").
      append(x).append("_").append(y).append("_").append(z);
}

FlyEmTodoChunk::KeyType FlyEmTodoChunk::GetItemKey(const ZIntPoint &pos)
{
  return GetItemKey(pos.getX(), pos.getY(), pos.getZ());
}

bool FlyEmTodoChunk::hasItem(int x, int y, int z) const
{
  return m_itemMap.count(GetItemKey(x, y, z)) > 0;
}

ZFlyEmToDoItem& FlyEmTodoChunk::getItemRef(int x, int y, int z)
{
  auto key = GetItemKey(x, y, z);
  if (m_itemMap.count(key) > 0) {
    return m_itemMap.at(key);
  }

  return m_invalidItem;
}

ZFlyEmToDoItem& FlyEmTodoChunk::getItemRef(const ZIntPoint pos)
{
  return getItemRef(pos.getX(), pos.getY(), pos.getZ());
}

ZFlyEmToDoItem FlyEmTodoChunk::getItem(int x, int y, int z) const
{
  return const_cast<FlyEmTodoChunk&>(*this).getItemRef(x, y, z);
}

ZFlyEmToDoItem FlyEmTodoChunk::getItem(const ZIntPoint pos) const
{
  return getItem(pos.getX(), pos.getY(), pos.getZ());
}

bool FlyEmTodoChunk::removeItem(int x, int y, int z)
{
  return m_itemMap.erase(GetItemKey(x, y, z)) > 0;
}

bool FlyEmTodoChunk::removeItem(const ZIntPoint &pos)
{
  return removeItem(pos.getX(), pos.getY(), pos.getZ());
}


std::vector<ZFlyEmToDoItem> FlyEmTodoChunk::getItemList() const
{
  std::vector<ZFlyEmToDoItem> itemList;
  for (auto item : m_itemMap) {
    itemList.push_back(item.second);
  }

  return itemList;
}

void FlyEmTodoChunk::forEachItem(
    std::function<void (const ZFlyEmToDoItem &)> f) const
{
  for (auto item : m_itemMap) {
    f(item.second);
  }
}

void FlyEmTodoChunk::addItem(const ZFlyEmToDoItem &item)
{
  if (item.isValid()) {
    m_itemMap[GetItemKey(item.getPosition())] = item;
  }
}

void FlyEmTodoChunk::addItem(const std::vector<ZFlyEmToDoItem> &itemList)
{
  for (const auto &item : itemList) {
    addItem(item);
  }
}

ZFlyEmToDoItem FlyEmTodoChunk::pickItem(double x, double y, double z, double r)
{
  for (auto item : m_itemMap) {
    ZFlyEmToDoItem &t = item.second;
    double d2 = t.getPosition().distanceSquareTo(x, y, z);
    if (d2 <= r * r) {
      return t;
    }
  }

  return m_invalidItem;
}

ZFlyEmToDoItem FlyEmTodoChunk::pickClosestItem(
    double x, double y, double z, double r)
{
  ZFlyEmToDoItem closestItem;

  double r2 = r * r;
  for (auto item : m_itemMap) {
    ZFlyEmToDoItem &t = item.second;
    double minDist2 = Infinity;
    double d2 = t.getPosition().distanceSquareTo(x, y, z);
    double tr2 = t.getRadius() * t.getRadius();
    if (d2 <= r2 && d2 < tr2) {
      if (d2 < minDist2) {
        minDist2 = d2;
        closestItem = t;
      }
    }
  }

  return closestItem;
}

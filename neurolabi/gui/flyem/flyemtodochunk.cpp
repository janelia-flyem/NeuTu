#include "flyemtodochunk.h"

#include "neulib/core/stringbuilder.h"

#include "geometry/zintpoint.h"

FlyEmTodoChunk::FlyEmTodoChunk()
{

}

void FlyEmTodoChunk::setReady(bool ready)
{
  m_isReady = ready;
}

bool FlyEmTodoChunk::isReady() const
{
  return m_isReady;
}

std::string FlyEmTodoChunk::GetItemKey(int x, int y, int z)
{
  return neulib::StringBuilder("").
      append(x).append("_").append(y).append("_").append(z);
}

std::string FlyEmTodoChunk::GetItemKey(const ZIntPoint &pos)
{
  return GetItemKey(pos.getX(), pos.getY(), pos.getZ());
}

bool FlyEmTodoChunk::hasItem(int x, int y, int z) const
{
  return m_itemMap.count(GetItemKey(x, y, z)) > 0;
}

ZFlyEmToDoItem FlyEmTodoChunk::getItem(int x, int y, int z) const
{
  std::string key = GetItemKey(x, y, z);
  if (m_itemMap.count(key) > 0) {
    return m_itemMap.at(key);
  }

  return ZFlyEmToDoItem();
}

std::vector<ZFlyEmToDoItem> FlyEmTodoChunk::getItemList() const
{
  std::vector<ZFlyEmToDoItem> itemList;
  for (auto item : m_itemMap) {
    itemList.push_back(item.second);
  }

  return itemList;
}

void FlyEmTodoChunk::forEachItem(std::function<void (const ZFlyEmToDoItem &)> f)
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

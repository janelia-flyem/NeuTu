#include "flyemtodoblockgrid.h"

#if 0
#include "neulib/core/stringbuilder.h"
#include "common/math.h"
#include "geometry/zgeometry.h"
#include "flyemtodosource.h"

FlyEmTodoBlockGrid::FlyEmTodoBlockGrid()
{
  m_emptyChunk.invalidate();
}

std::string FlyEmTodoBlockGrid::GetChunkKey(int i, int j, int k)
{
  return neulib::StringBuilder("").
      append(i).append("_").append(j).append("_").append(k);
}

FlyEmTodoChunk FlyEmTodoBlockGrid::getChunk(int i, int j, int k) const
{
  std::lock_guard<std::mutex> guard(m_chunkMutex);
  return getTodoChunkRef(i, j, k);
}

FlyEmTodoChunk &FlyEmTodoBlockGrid::getTodoChunkRef(int i, int j, int k) const
{
//  std::lock_guard<std::mutex> guard(m_chunkMutex);
  std::string key = GetChunkKey(i, j, k);
  if (m_chunkMap.count(key) > 0) {
    return m_chunkMap.at(key);
  } else {
    if (m_source) {
      std::vector<ZFlyEmToDoItem> todoList = m_source->getData(
            getBlockBox(ZIntPoint(i, j, k)));
      FlyEmTodoChunk &chunk = m_chunkMap[key];
      chunk.addItem(todoList);
      chunk.setReady(true);
      return chunk;
//      return m_chunkMap[key];
    }
  }

  return m_emptyChunk;
}

FlyEmTodoChunk& FlyEmTodoBlockGrid::getTodoChunkRef(const ZIntPoint &pt) const
{
  return getTodoChunkRef(pt.getX(), pt.getY(), pt.getZ());
}

void FlyEmTodoBlockGrid::setSource(std::shared_ptr<FlyEmTodoSource> source)
{
  m_source = source;
  setBlockSize(source->getBlockSize());
  setGridByRange(source->getRange());
}

FlyEmTodoChunk& FlyEmTodoBlockGrid::getHostChunkRef(const ZIntPoint &pos) const
{
  ZIntPoint index = getBlockIndex(pos);
  if (containsBlock(index)) {
    return getTodoChunkRef(index.getX(), index.getY(), index.getZ());
  }

  return m_emptyChunk;
}

void FlyEmTodoBlockGrid::addItem(const ZFlyEmToDoItem &item)
{
  if (item.isValid()) {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    FlyEmTodoChunk &chunk = getHostChunkRef(item.getPosition());
    if (chunk.isValid()) {
      m_source->saveItem(item);
      chunk.addItem(item);
    } else {
      throw std::runtime_error("Failed to add todo at an invalid location.");
    }
  }
}

void FlyEmTodoBlockGrid::removeItem(const ZIntPoint &pos)
{
  std::lock_guard<std::mutex> guard(m_chunkMutex);
  FlyEmTodoChunk &chunk = getHostChunkRef(pos);
  if (chunk.isValid()) {
    m_source->removeItem(pos);
    chunk.removeItem(pos);
  } else {
    throw std::runtime_error("Failed to remove todo at an invalid location.");
  }
}

void FlyEmTodoBlockGrid::removeItem(int x, int y, int z)
{
  removeItem(ZIntPoint(x, y, z));
}

ZFlyEmToDoItem FlyEmTodoBlockGrid::getExistingItem(int x, int y, int z) const
{
  return getExistingItem(ZIntPoint(x, y, z));
}

ZFlyEmToDoItem FlyEmTodoBlockGrid::getExistingItem(const ZIntPoint &pos) const
{
  ZIntPoint blockIndex = getBlockIndex(pos);
  if (containsBlock(blockIndex)) {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    std::string key = GetChunkKey(
          blockIndex.getX(), blockIndex.getY(), blockIndex.getZ());
    if (m_chunkMap.count(key) > 0) {
      FlyEmTodoChunk &chunk = m_chunkMap.at(key);
      return chunk.getItem(pos);
    }
  }

  return ZFlyEmToDoItem();
}

ZFlyEmToDoItem FlyEmTodoBlockGrid::getItem(int x, int y, int z) const
{
  return getItem(ZIntPoint(x, y, z));
}

ZFlyEmToDoItem FlyEmTodoBlockGrid::getItem(const ZIntPoint &pos) const
{
  ZIntPoint blockIndex = getBlockIndex(pos);
  if (containsBlock(blockIndex)) {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    return getHostChunkRef(pos).getItemRef(pos);
  }

  return ZFlyEmToDoItem();
}

ZFlyEmToDoItem FlyEmTodoBlockGrid::pickExistingItem(
    double x, double y, double z) const
{
  ZIntPoint pos = ZPoint(x, y, z).toIntPoint();
  return getExistingItem(pos);
}

ZFlyEmToDoItem FlyEmTodoBlockGrid::pickClosestExistingItem(
    double x, double y, double z, double r) const
{
  ZIntPoint pos = ZPoint(x, y, z).toIntPoint();
  ZIntPoint blockIndex = getBlockIndex(pos);
  std::vector<ZFlyEmToDoItem> itemList;
  if (containsBlock(blockIndex)) {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    ZFlyEmToDoItem item = getTodoChunkRef(blockIndex).pickClosestItem(x, y, z, r);
    if (item.isValid()) {
      itemList.push_back(item);
    }
  }

  zgeom::raster::ForEachNeighbor<3>(
        blockIndex.getX(), blockIndex.getY(), blockIndex.getZ(),
        [&](int i, int j, int k) {
    if (containsBlock(i, j, k)) {
      std::lock_guard<std::mutex> guard(m_chunkMutex);
      ZFlyEmToDoItem item = getTodoChunkRef(i, j, k).pickClosestItem(x, y, z, r);
      if (item.isValid()) {
        itemList.push_back(item);
      }
    }
  });

#ifdef _DEBUG_0
  std::cout << "Hit item:" << std::endl;
  for (const auto &item: itemList) {
    std::cout << item.getPosition() << std::endl;
  }
#endif

  ZFlyEmToDoItem picked;
  double minDist2 = Infinity;
  for (const ZFlyEmToDoItem &t : itemList) {
    double d2 = t.getPosition().distanceSquareTo(x, y, z);
    if (d2 < minDist2) {
      minDist2 = d2;
      picked = t;
    }
  }

  return picked;
}

void FlyEmTodoBlockGrid::forEachItemInChunk(
    int i, int j, int k, std::function<void (const ZFlyEmToDoItem &)> f)
{
  if (containsBlock(i, j, k)) {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    const FlyEmTodoChunk& chunk = getTodoChunkRef(i, j, k);
    if (chunk.isValid()) {
      chunk.forEachItem(f);
    }
  }
}

bool FlyEmTodoBlockGrid::setExistingSelection(const ZIntPoint &itemPos, bool selecting)
{
  ZIntPoint blockIndex = getBlockIndex(itemPos);
  if (containsBlock(blockIndex)) {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    std::string key = GetChunkKey(
          blockIndex.getX(), blockIndex.getY(), blockIndex.getZ());
    if (m_chunkMap.count(key) > 0) {
      FlyEmTodoChunk &chunk = m_chunkMap.at(key);
      ZFlyEmToDoItem &item = chunk.getItemRef(itemPos);
      if (item.isValid()) {
        item.setSelected(selecting);
        return true;
      }
    }
  }

  return false;
}

/*
std::vector<ZFlyEmToDoItem> FlyEmTodoBlockGrid::getIntersectTodoList(
    const ZAffineRect &plane) const
{

}
*/
#endif

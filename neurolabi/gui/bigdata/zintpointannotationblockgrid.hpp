#ifndef ZINTPOINTANNOTATIONBLOCKGRID_HPP
#define ZINTPOINTANNOTATIONBLOCKGRID_HPP

#include <functional>

#include "geometry/zpoint.h"
#include "geometry/zgeometry.h"

#include "zblockgrid.h"
#include "zintpointannotationchunk.h"
#include "zintpointannotationsource.hpp"

template<typename T, typename TChunk>
class ZIntPointAnnotationBlockGrid : public ZBlockGrid
{
public:
  ZIntPointAnnotationBlockGrid() {
    m_emptyChunk.invalidate();
  }

  void setSource(std::shared_ptr<ZIntPointAnnotationSource<T>> source)
  {
    m_source = source;
    setBlockSize(source->getBlockSize());
    setGridByRange(source->getRange());
  }

  void addItem(const T &item);
  void removeItem(const ZIntPoint &pos);
  void removeItem(int x, int y, int z) {
    removeItem(ZIntPoint(x, y, z));
  }
  void updateItem(const ZIntPoint &pos);
  void moveItem(const ZIntPoint &from, const ZIntPoint &to);

  void processItem(const ZIntPoint &pos, std::function<void(T*)> f);

  void updatePartner(const ZIntPoint &pos)
  {
    processItem(pos, [this](T *item) {
      m_source->updatePartner(item);
    });
  }

  /*
  std::vector<ZFlyEmToDoItem> getIntersectTodoList(
      const ZAffineRect &plane) const;
      */

  T getExistingItem(int x, int y, int z) const
  {
    return getExistingItem({x, y, z});
  }

  T getExistingItem(const ZIntPoint &pos) const;

  T getItem(int x, int y, int z) const
  {
    return getExistingItem(ZIntPoint(x, y, z));
  }

  T getItem(const ZIntPoint &pos) const
  {
    ZIntPoint blockIndex = getBlockIndex(pos);
    if (containsBlock(blockIndex)) {
      std::lock_guard<std::mutex> guard(m_chunkMutex);
      auto key = blockIndex;
      if (m_chunkMap.count(key) > 0) {
        TChunk &chunk = m_chunkMap.at(key);
        return chunk.getItem(pos);
      }
    }

    return T();
  }

  T pickExistingItem(double x, double y, double z) const
  {
    ZIntPoint pos = ZPoint(x, y, z).toIntPoint();
    return getExistingItem(pos);
  }

  T pickClosestExistingItem(
      double x, double y, double z, double r) const;
  T hitClosestExistingItem(
      double x, double y, double z, double r) const;

  void forEachItemInChunk(
      int i, int j, int k, std::function<void(const T &item)> f)
  {
    if (containsBlock(i, j, k)) {
      std::lock_guard<std::mutex> guard(m_chunkMutex);
      const TChunk& chunk = getChunkRef(i, j, k);
      if (chunk.isValid()) {
        chunk.forEachItem(f);
      }
    }
  }

  void processChunk(
      int i, int j, int k,
      std::function<void(const ZIntPointAnnotationChunk<T> &item)> f) const
  {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    f(getChunkRef(i, j, k));
  }

  TChunk getChunk(int i, int j, int k) const
  {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    return getChunkRef(i, j, k);
  }

  bool setExistingSelection(const ZIntPoint &itemPos, bool selecting);

private:
  static std::string GetChunkKey(int i, int j, int k);
  TChunk& getHostChunkRef(const ZIntPoint &pos) const
  {
    ZIntPoint index = getBlockIndex(pos);
    if (containsBlock(index)) {
      return getChunkRef(index.getX(), index.getY(), index.getZ());
    }

    return m_emptyChunk;
  }

  TChunk& getChunkRef(int i, int j, int k) const
  {
    return getChunkRef({i, j, k});
  }

  TChunk& getChunkRef(const ZIntPoint &index) const
  {
    ZIntPoint key = index;
    if (m_chunkMap.count(key) > 0) {
      return m_chunkMap.at(key);
    } else {
      if (m_source) {
        std::vector<T> todoList = m_source->getData(getBlockBox(index));
        TChunk &chunk = m_chunkMap[key];
        chunk.update(todoList);
//        chunk.addItem(todoList);
//        chunk.setReady(true);
        return chunk;
      }
    }

    return m_emptyChunk;
  }

private:
  mutable std::mutex m_chunkMutex;
  mutable std::unordered_map<ZIntPoint, TChunk> m_chunkMap;
  mutable TChunk m_emptyChunk;
  std::shared_ptr<ZIntPointAnnotationSource<T>> m_source;
};

template<typename T, typename TChunk>
void ZIntPointAnnotationBlockGrid<T, TChunk>::addItem(const T &item)
{
  if (item.isValid()) {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    TChunk &chunk = getHostChunkRef(item.getPosition());
    if (chunk.isValid()) {
      m_source->saveItem(item);
      chunk.addItem(item);
    } else {
      throw std::runtime_error("Failed to add item at an invalid location.");
    }
  }
}

template<typename T, typename TChunk>
void ZIntPointAnnotationBlockGrid<T, TChunk>::removeItem(const ZIntPoint &pos)
{
  std::lock_guard<std::mutex> guard(m_chunkMutex);
  TChunk &chunk = getHostChunkRef(pos);
  if (chunk.isValid()) {
    m_source->removeItem(pos);
    chunk.removeItem(pos);
  } else {
    throw std::runtime_error("Failed to remove item at an invalid location.");
  }
}

template<typename T, typename TChunk>
void ZIntPointAnnotationBlockGrid<T, TChunk>::updateItem(const ZIntPoint &pos)
{
  std::lock_guard<std::mutex> guard(m_chunkMutex);
  TChunk &chunk = getHostChunkRef(pos);
  if (chunk.isValid()) {
    T item = m_source->getItem(pos);
    if (item.isValid()) {
      chunk.addItem(item);
    } else {
      chunk.removeItem(pos);
    }
  }
}

template<typename T, typename TChunk>
void ZIntPointAnnotationBlockGrid<T, TChunk>::moveItem(
    const ZIntPoint &from, const ZIntPoint &to)
{
  if (from != to) {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    TChunk &sourceChunk = getHostChunkRef(from);
    TChunk &dstChunk = getHostChunkRef(to);
    if (sourceChunk.isValid() && dstChunk.isValid()) {
      m_source->moveItem(from, to);
      sourceChunk.removeItem(from);
      T item = m_source->getItem(to);
      if (item.isValid()) {
        dstChunk.addItem(item);
      } else {
        throw std::runtime_error("Something went wrong while moving item.");
      }
    } else {
      throw std::runtime_error("Failed to move item.");
    }
  }
}

template<typename T, typename TChunk>
void ZIntPointAnnotationBlockGrid<T, TChunk>::processItem(
    const ZIntPoint &pos, std::function<void(T*)> f)
{
  std::lock_guard<std::mutex> guard(m_chunkMutex);
  ZIntPoint index = getBlockIndex(pos);
  if (containsBlock(index)) {
    T& item =  getChunkRef(index).getItemRef(pos);
    if (item.isValid()) {
      f(&item);
    }
  }
}

template<typename T, typename TChunk>
T ZIntPointAnnotationBlockGrid<T, TChunk>::pickClosestExistingItem(
    double x, double y, double z, double r) const
{
  ZIntPoint pos = ZPoint(x, y, z).toIntPoint();
  ZIntPoint blockIndex = getBlockIndex(pos);
  std::vector<T> itemList;
  if (containsBlock(blockIndex)) {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    T item = getChunkRef(blockIndex).pickClosestItem(x, y, z, r);
    if (item.isValid()) {
      itemList.push_back(item);
    }
  }

  zgeom::raster::ForEachNeighbor<3>(
        blockIndex.getX(), blockIndex.getY(), blockIndex.getZ(),
        [&](int i, int j, int k) {
    if (containsBlock(i, j, k)) {
      std::lock_guard<std::mutex> guard(m_chunkMutex);
      T item = getChunkRef(i, j, k).pickClosestItem(x, y, z, r);
      if (item.isValid()) {
        itemList.push_back(item);
      }
    }
  });

  T picked;
  double minDist2 = Infinity;
  for (const T &t : itemList) {
    double d2 = t.getPosition().distanceSquareTo(x, y, z);
    if (d2 < minDist2) {
      minDist2 = d2;
      picked = t;
    }
  }

  return picked;
}

template<typename T, typename TChunk>
T ZIntPointAnnotationBlockGrid<T, TChunk>::hitClosestExistingItem(
    double x, double y, double z, double r) const
{
  ZIntPoint pos = ZPoint(x, y, z).toIntPoint();
  ZIntPoint blockIndex = getBlockIndex(pos);
  std::vector<T> itemList;
  if (containsBlock(blockIndex)) {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    T item = getChunkRef(blockIndex).hitClosestItem(x, y, z, r);
    if (item.isValid()) {
      itemList.push_back(item);
    }
  }

  zgeom::raster::ForEachNeighbor<3>(
        blockIndex.getX(), blockIndex.getY(), blockIndex.getZ(),
        [&](int i, int j, int k) {
    if (containsBlock(i, j, k)) {
      std::lock_guard<std::mutex> guard(m_chunkMutex);
      T item = getChunkRef(i, j, k).hitClosestItem(x, y, z, r);
      if (item.isValid()) {
        itemList.push_back(item);
      }
    }
  });

  T picked;
  double minDist2 = Infinity;
  for (const T &t : itemList) {
    double d2 = t.getPosition().distanceSquareTo(x, y, z);
    if (d2 < minDist2) {
      minDist2 = d2;
      picked = t;
    }
  }

  return picked;
}

template<typename T, typename TChunk>
bool ZIntPointAnnotationBlockGrid<T, TChunk>::setExistingSelection(
    const ZIntPoint &itemPos, bool selecting)
{
  ZIntPoint blockIndex = getBlockIndex(itemPos);
  if (containsBlock(blockIndex)) {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    auto key = blockIndex;
    if (m_chunkMap.count(key) > 0) {
      TChunk &chunk = m_chunkMap.at(key);
      T &item = chunk.getItemRef(itemPos);
      if (item.isValid()) {
        item.setSelected(selecting);
        m_source->updatePartner(&item);
        return true;
      }
    }
  }

  return false;
}

template<typename T, typename TChunk>
T ZIntPointAnnotationBlockGrid<T, TChunk>::getExistingItem(const ZIntPoint &pos) const
{
  ZIntPoint blockIndex = getBlockIndex(pos);
  if (containsBlock(blockIndex)) {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    auto key = blockIndex;
    if (m_chunkMap.count(key) > 0) {
      TChunk &chunk = m_chunkMap.at(key);
      return chunk.getItem(pos);
    }
  }

  return T();
}

#endif // ZINTPOINTANNOTATIONBLOCKGRID_HPP

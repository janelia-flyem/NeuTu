#ifndef ZINTPOINTANNOTATIONBLOCKGRID_HPP
#define ZINTPOINTANNOTATIONBLOCKGRID_HPP

#include <functional>

#include "geometry/zpoint.h"
#include "geometry/zgeometry.h"

#include "zblockgrid.h"
#include "zintpointannotationchunk.h"
#include "zintpointannotationsource.hpp"

template<typename TItem, typename TChunk>
class ZIntPointAnnotationBlockGrid : public ZBlockGrid
{
public:
  ZIntPointAnnotationBlockGrid() {
    m_emptyChunk.invalidate();
  }

  void setSource(std::shared_ptr<ZIntPointAnnotationSource<TItem>> source)
  {
    m_source = source;
    setBlockSize(source->getBlockSize());
    setGridByRange(source->getRange());
  }

  void addItem(const TItem &item);
  void removeItem(const ZIntPoint &pos);
  void removeItem(int x, int y, int z);
  void moveItem(const ZIntPoint &from, const ZIntPoint &to);

  void syncItemToCache(const ZIntPoint &pos);
  void processItemInCache(const ZIntPoint &pos, std::function<void(TItem*)> f);

  void syncPartnerToCache(const ZIntPoint &pos)
  {
    processItemInCache(pos, [this](TItem *item) {
      m_source->updatePartner(item);
    });
  }

  TItem getCachedItem(int x, int y, int z) const
  {
    return getCachedItem({x, y, z});
  }

  TItem getCachedItem(const ZIntPoint &pos) const;

  TItem getItem(int x, int y, int z) const
  {
    return getItem(ZIntPoint(x, y, z));
  }

  TItem getItem(const ZIntPoint &pos) const
  {
    ZIntPoint blockIndex = getBlockIndex(pos);
    if (containsBlock(blockIndex)) {
      std::lock_guard<std::mutex> guard(m_chunkMutex);
      return getChunkRef(blockIndex).getItem(pos);
    }

    return TItem();
  }

  TItem pickCachedItem(double x, double y, double z) const
  {
    ZIntPoint pos = ZPoint(x, y, z).toIntPoint();
    return getCachedItem(pos);
  }

  TItem pickClosestCachedItem(
      double x, double y, double z, double r) const;
  TItem hitClosestCachedItem(
      double x, double y, double z, double r, int viewId) const;

  void forEachItemInChunk(
      int i, int j, int k, std::function<void(const TItem &item)> f)
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
      std::function<void(const ZIntPointAnnotationChunk<TItem> &item)> f) const
  {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    f(getChunkRef(i, j, k));
  }

  TChunk getChunk(int i, int j, int k) const
  {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    return getChunkRef(i, j, k);
  }

  bool setSelectionForCached(const ZIntPoint &itemPos, bool selecting);

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
    if (m_chunkMap.count(key) == 0 && m_source) {
      m_chunkMap[key];
    }

    TChunk &chunk =
        (m_chunkMap.count(key) > 0) ? m_chunkMap.at(key) : m_emptyChunk;

    if (chunk.updateNeeded() && m_source) {
      std::vector<TItem> todoList = m_source->getData(getBlockBox(index));
      chunk.update(todoList);
    }

    return chunk;
  }

private:
  mutable std::mutex m_chunkMutex;
  mutable std::unordered_map<ZIntPoint, TChunk> m_chunkMap;
  mutable TChunk m_emptyChunk;
  std::shared_ptr<ZIntPointAnnotationSource<TItem>> m_source;
};

template<typename TItem, typename TChunk>
void ZIntPointAnnotationBlockGrid<TItem, TChunk>::addItem(const TItem &item)
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

template<typename TItem, typename TChunk>
void ZIntPointAnnotationBlockGrid<TItem, TChunk>::removeItem(const ZIntPoint &pos)
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

template<typename TItem, typename TChunk>
void ZIntPointAnnotationBlockGrid<TItem, TChunk>::removeItem(int x, int y, int z)
{
   removeItem(ZIntPoint(x, y, z));
 }

template<typename TItem, typename TChunk>
void ZIntPointAnnotationBlockGrid<TItem, TChunk>::syncItemToCache(const ZIntPoint &pos)
{
  std::lock_guard<std::mutex> guard(m_chunkMutex);
  TChunk &chunk = getHostChunkRef(pos);
  if (chunk.isValid()) {
    TItem item = m_source->getItem(pos);
    if (item.isValid()) {
      chunk.addItem(item);
    } else {
      chunk.removeItem(pos);
    }
  }
}

template<typename TItem, typename TChunk>
void ZIntPointAnnotationBlockGrid<TItem, TChunk>::moveItem(
    const ZIntPoint &from, const ZIntPoint &to)
{
  if (from != to) {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    TChunk &sourceChunk = getHostChunkRef(from);
    TChunk &dstChunk = getHostChunkRef(to);
    if (sourceChunk.isValid() && dstChunk.isValid()) {
      m_source->moveItem(from, to);
      sourceChunk.removeItem(from);
      TItem item = m_source->getItem(to);
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

template<typename TItem, typename TChunk>
void ZIntPointAnnotationBlockGrid<TItem, TChunk>::processItemInCache(
    const ZIntPoint &pos, std::function<void(TItem*)> f)
{
  std::lock_guard<std::mutex> guard(m_chunkMutex);
  ZIntPoint index = getBlockIndex(pos);
  if (containsBlock(index)) {
    TItem& item =  getChunkRef(index).getItemRef(pos);
    if (item.isValid()) {
      f(&item);
    }
  }
}

template<typename TItem, typename TChunk>
TItem ZIntPointAnnotationBlockGrid<TItem, TChunk>::pickClosestCachedItem(
    double x, double y, double z, double r) const
{
  ZIntPoint pos = ZPoint(x, y, z).toIntPoint();
  ZIntPoint blockIndex = getBlockIndex(pos);
  std::vector<TItem> itemList;
  if (containsBlock(blockIndex)) {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    TItem item = getChunkRef(blockIndex).pickClosestItem(x, y, z, r);
    if (item.isValid()) {
      itemList.push_back(item);
    }
  }

  zgeom::raster::ForEachNeighbor<3>(
        blockIndex.getX(), blockIndex.getY(), blockIndex.getZ(),
        [&](int i, int j, int k) {
    if (containsBlock(i, j, k)) {
      std::lock_guard<std::mutex> guard(m_chunkMutex);
      TItem item = getChunkRef(i, j, k).pickClosestItem(x, y, z, r);
      if (item.isValid()) {
        itemList.push_back(item);
      }
    }
  });

  TItem picked;
  double minDist2 = Infinity;
  for (const TItem &t : itemList) {
    double d2 = t.getPosition().distanceSquareTo(x, y, z);
    if (d2 < minDist2) {
      minDist2 = d2;
      picked = t;
    }
  }

  return picked;
}

template<typename TItem, typename TChunk>
TItem ZIntPointAnnotationBlockGrid<TItem, TChunk>::hitClosestCachedItem(
    double x, double y, double z, double r, int viewId) const
{
  ZIntPoint pos = ZPoint(x, y, z).toIntPoint();
  ZIntPoint blockIndex = getBlockIndex(pos);
  std::vector<TItem> itemList;
  if (containsBlock(blockIndex)) {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    TItem item = getChunkRef(blockIndex).hitClosestItem(x, y, z, r, viewId);
    if (item.isValid()) {
      itemList.push_back(item);
    }
  }

  zgeom::raster::ForEachNeighbor<3>(
        blockIndex.getX(), blockIndex.getY(), blockIndex.getZ(),
        [&](int i, int j, int k) {
    if (containsBlock(i, j, k)) {
      std::lock_guard<std::mutex> guard(m_chunkMutex);
      TItem item = getChunkRef(i, j, k).hitClosestItem(x, y, z, r, viewId);
      if (item.isValid()) {
        itemList.push_back(item);
      }
    }
  });

  TItem picked;
  double minDist2 = Infinity;
  for (const TItem &t : itemList) {
    double d2 = t.getPosition().distanceSquareTo(x, y, z);
    if (d2 < minDist2) {
      minDist2 = d2;
      picked = t;
    }
  }

  return picked;
}

template<typename TItem, typename TChunk>
bool ZIntPointAnnotationBlockGrid<TItem, TChunk>::setSelectionForCached(
    const ZIntPoint &itemPos, bool selecting)
{
  ZIntPoint blockIndex = getBlockIndex(itemPos);
  if (containsBlock(blockIndex)) {
    std::lock_guard<std::mutex> guard(m_chunkMutex);
    auto key = blockIndex;
    if (m_chunkMap.count(key) > 0) {
      TChunk &chunk = m_chunkMap.at(key);
      TItem &item = chunk.getItemRef(itemPos);
      if (item.isValid()) {
        item.setSelected(selecting);
        m_source->updatePartner(&item);
        return true;
      }
    }
  }

  return false;
}

template<typename TItem, typename TChunk>
TItem ZIntPointAnnotationBlockGrid<TItem, TChunk>::getCachedItem(
    const ZIntPoint &pos) const
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

  return TItem();
}

#endif // ZINTPOINTANNOTATIONBLOCKGRID_HPP

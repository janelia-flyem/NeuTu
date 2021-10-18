#include "zstackblocksource.h"

#include "zstackblockfactory.h"
#include "bigdata/zstackblockgrid.h"
#include "zutils.h"
#include "zobject3dscan.h"

ZStackBlockSource::ZStackBlockSource()
{
}

ZStackBlockSource::~ZStackBlockSource()
{
}

void ZStackBlockSource::clearCache()
{
  m_cache.clear();
}

void ZStackBlockSource::setBlockFactory(
    std::unique_ptr<ZStackBlockFactory> &&factory)
{
  clearCache();
  m_factory = std::move(factory);

  setBlockSize(m_factory->getBlockSize());
  setGridSize(m_factory->getGridSize());
}

void ZStackBlockSource::setBlockFactory(ZStackBlockFactory *factory)
{
  setBlockFactory(std::unique_ptr<ZStackBlockFactory>(factory));
}

bool ZStackBlockSource::hasBlockFactory() const
{
  return bool(m_factory);
}

ZStackBlockGrid* ZStackBlockSource::getBlockGrid(int zoom) const
{
  ZStackBlockGrid *grid = nullptr;
  if (int(m_cache.size()) > zoom) {
    grid = m_cache.at(zoom).get();
  }

  return grid;
}

ZStackBlockGrid* ZStackBlockSource::takeBlockGrid(int zoom)
{
  ZStackBlockGrid *grid = nullptr;
  if (int(m_cache.size()) > zoom) {
    grid = m_cache.at(zoom).release();
  }

  return grid;
}

ZIntPoint ZStackBlockSource::getBlockSize() const
{
  return m_gridConfig.getBlockSize();
}

void ZStackBlockSource::setBlockGrid(int zoom, ZStackBlockGrid *grid)
{
  if (zoom >= int(m_cache.size())) {
    m_cache.resize(zoom + 1);
  }
  m_cache[zoom] = std::unique_ptr<ZStackBlockGrid>(grid);
}

void ZStackBlockSource::setBlockSize(const ZIntPoint &size)
{
  setBlockSize(size.getX(), size.getY(), size.getZ());
}

void ZStackBlockSource::setBlockSize(int width, int height, int depth)
{
  m_gridConfig.setBlockSize(width, height, depth);
}

void ZStackBlockSource::setGridSize(const ZIntPoint &size)
{
  setGridSize(size.getX(), size.getY(), size.getZ());
}

void ZStackBlockSource::setGridSize(int width, int height, int depth)
{
  m_gridConfig.setGridSize(width, height, depth);
}

void ZStackBlockSource::cacheStack(int i, int j, int k, int zoom, ZStack *stack)
{
  if (stack) {
    if (zoom >= int(m_cache.size())) {
      m_cache.resize(zoom + 1);
    }
    auto &grid = m_cache[zoom];
    if (!grid) {
      m_cache[zoom] = std::make_unique<ZStackBlockGrid>();
      m_cache[zoom]->configure(m_gridConfig, zoom);
    }
    grid->consumeStack(ZIntPoint(i, j, k), stack);
  }
}

bool ZStackBlockSource::isCached(int i, int j, int k, int zoom)
{
  if (zoom >= int(m_cache.size())) {
    return false;
  }

  std::unique_ptr<ZStackBlockGrid> &grid = m_cache[zoom];

  if (grid) {
    return (grid->getStack(ZIntPoint(i, j, k)) != nullptr);
  }

  return false;
}

bool ZStackBlockSource::isAllCached(int i, int j, int k, int n, int zoom)
{
  for (int t = 0; t < n; ++t) {
    if (!isCached(i + t, j, k, zoom)) {
      return false;
    }
  }

  return true;
}

bool ZStackBlockSource::isAllCached(const ZIntPoint &blockIndex, int n, int zoom)
{
  return isAllCached(
        blockIndex.getX(), blockIndex.getY(), blockIndex.getZ(), n, zoom);
}

void ZStackBlockSource::cacheStack(const ZIntPoint &blockIndex, int n, int zoom)
{
  if (!isAllCached(blockIndex, n, zoom)) {
    std::vector<ZStack*> stackArray = m_factory->make(blockIndex, n, zoom);
    for (size_t i = 0; i < stackArray.size(); ++i) {
      cacheStack(blockIndex.getX() + i, blockIndex.getY(), blockIndex.getZ(),
                 zoom, stackArray[i]);
    }
  }
}

void ZStackBlockSource::cacheStack(const ZObject3dScan &obj, int zoom)
{
  if (m_factory && zoom <= getMaxZoom()) {
    ZObject3dScan::ConstSegmentIterator iter(&obj);
    while (iter.hasNext()) {
      const ZObject3dScan::Segment& seg = iter.next();
      ZIntPoint blockIndex(seg.getStart(), seg.getY(), seg.getZ());
      int n = seg.getEnd() - seg.getStart() + 1;
      cacheStack(blockIndex, n, zoom);
      /*
      std::vector<ZStack*> stackArray = m_factory->make(blockIndex, n, zoom);
      for (size_t i = 0; i < stackArray.size(); ++i) {
        cacheStack(blockIndex.getX() + i, blockIndex.getY(), blockIndex.getZ(),
                   zoom, stackArray[i]);
      }
      */
    }
  }
}

namespace {
/*
std::vector<std::pair<int, int>> CutSegment(
    int x0, int x1, int cx0, int cx1)
{
  std::vector<std::pair<int, int>> result;
  if (x0 < cx0) {
    result.emplace_back(x0, std::min(x1, cx0 - 1));
  }

  if (x1 > cx1) {
    result.emplace_back(std::max(x0, cx1 + 1), x1);
  }

  return result;
}
*/

std::vector<std::pair<int, int>> IntersectSegment(
    int x0, int x1, int cx0, int cx1)
{
  std::vector<std::pair<int, int>> result;

  if (x0 < cx0) {
    x0 = cx0;
  }

  if (x1 > cx1) {
    x1 = cx1;
  }

  if (x0 <= x1) {
    result.emplace_back(x0, x1);
  }

  return result;
}
}

void ZStackBlockSource::cacheStack(
    const ZObject3dScan &obj, const ZIntCuboid &range, int zoom)
{
#ifdef _DEBUG_
  std::cout << "Caching stack range: " << range.toString() << std::endl;
  std::cout << "Caching object:" << std::endl;
  obj.print();
#endif

  if (range.isEmpty()) {
    cacheStack(obj, zoom);
  } else {
    if (m_factory && zoom <= getMaxZoom()) {
      ZObject3dScan::ConstSegmentIterator iter(&obj);
      while (iter.hasNext()) {
        const ZObject3dScan::Segment& seg = iter.next();
        //      ZIntPoint blockIndex(seg.getStart(), seg.getY(), seg.getZ());
        if (range.containsYZ(seg.getY(), seg.getZ())) {
          std::vector<std::pair<int, int>> segArray =
              IntersectSegment(seg.getStart(), seg.getEnd(),
                         range.getMinCorner().getX(), range.getMaxCorner().getX());
          for (const auto& newSeg : segArray) {
            ZIntPoint blockIndex(newSeg.first, seg.getY(), seg.getZ());
            int n = newSeg.second - newSeg.first + 1;
            if (!isAllCached(blockIndex, n, zoom)) {
              std::vector<ZStack*> stackArray = m_factory->make(blockIndex, n, zoom);
              for (size_t i = 0; i < stackArray.size(); ++i) {
                cacheStack(blockIndex.getX() + i, blockIndex.getY(), blockIndex.getZ(),
                           zoom, stackArray[i]);
              }
            }
          }
        }
      }
    }
  }
}

ZStack* ZStackBlockSource::getStack(int i, int j, int k, int zoom)
{
  ZStack *stack = nullptr;

  ZStackBlockGrid *grid = getBlockGrid(zoom);
  if (grid) {
    stack = grid->getStack(ZIntPoint(i, j, k));
  }

  if (stack == nullptr) {
    stack = m_factory->make(ZIntPoint(i, j, k), zoom);
    cacheStack(i, j, k, zoom, stack);
  }

  return stack;
}

int ZStackBlockSource::getMaxZoom() const
{
  if (m_factory) {
    return m_factory->getMaxZoom();
  }

  return 0;
}

void ZStackBlockSource::mergeCache(ZStackBlockSource &source)
{
  for (size_t i = 0; i < source.m_cache.size(); ++i) {
    ZStackBlockGrid *grid = source.m_cache[i].release();
    if (grid) {
      if (m_cache.size() <= i) {
        m_cache.resize(i + 1);
      }
      if (m_cache[i]) {
        m_cache[i]->consume(grid);
      } else {
        m_cache[i] = std::unique_ptr<ZStackBlockGrid>(grid);
      }
    }
  }
}

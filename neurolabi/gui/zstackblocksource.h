#ifndef ZSTACKBLOCKSOURCE_H
#define ZSTACKBLOCKSOURCE_H

#include <memory>
#include <vector>

#include "bigdata/zblockgrid.h"

class ZStackBlockGrid;
class ZStackBlockFactory;
class ZStack;
class ZIntPoint;
class ZObject3dScan;

class ZStackBlockSource
{
public:
  ZStackBlockSource();
  ~ZStackBlockSource();

  ZStack* getStack(int i, int j, int k, int zoom);

  void setBlockFactory(std::unique_ptr<ZStackBlockFactory> &&factory);
  void setBlockFactory(ZStackBlockFactory *factory);

  bool hasBlockFactory() const;

  ZStackBlockGrid* getBlockGrid(int zoom) const;
  ZStackBlockGrid* takeBlockGrid(int zoom);

  ZIntPoint getBlockSize() const;

  void setBlockGrid(int zoom, ZStackBlockGrid *grid);

  void setGridSize(int width, int height, int depth);
  void setBlockSize(int width, int height, int depth);
  void setBlockSize(const ZIntPoint &size);
  void setGridSize(const ZIntPoint &size);

  void clearCache();

  int getMaxZoom() const;

  void mergeCache(ZStackBlockSource &source);

  void cacheStack(const ZIntPoint &blockIndex, int n, int zoom);
  void cacheStack(const ZObject3dScan &obj, int zoom);
  void cacheStack(const ZObject3dScan &obj, const ZIntCuboid &range, int zoom);

//  ZStackBlockGrid* makeDownSample(int xintv, int yintv, int zintv);

private:
  void cacheStack(int i, int j, int k, int zoom, ZStack *stack);
  bool isCached(int i, int j, int k, int zoom);
  bool isAllCached(int i, int j, int k, int n, int zoom);
  bool isAllCached(const ZIntPoint &blockIndex, int n, int zoom);


private:
  std::vector<std::unique_ptr<ZStackBlockGrid>> m_cache;
  std::unique_ptr<ZStackBlockFactory> m_factory;
  ZBlockGrid m_gridConfig;
};

#endif // ZSTACKBLOCKSOURCE_H

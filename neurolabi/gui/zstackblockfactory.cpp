#include "zstackblockfactory.h"

#include "geometry/zintpoint.h"

ZStackBlockFactory::ZStackBlockFactory()
{

}


ZStackBlockFactory::~ZStackBlockFactory()
{

}

int ZStackBlockFactory::getMaxZoom() const
{
  return 0;
}

ZIntPoint ZStackBlockFactory::getBlockSize() const
{
  return ZIntPoint(32, 32, 32);
}

ZIntPoint ZStackBlockFactory::getGridSize() const
{
  return ZIntPoint(1000, 1000, 1000);
}

std::vector<ZStack*> ZStackBlockFactory::make(
    const ZIntPoint &blockIndex, int n, int zoom) const
{
  return makeV(blockIndex.getX(), blockIndex.getY(), blockIndex.getZ(), n, zoom);
}

ZStack* ZStackBlockFactory::make(
    const ZIntPoint &blockIndex, int zoom) const
{
  std::vector<ZStack*> stackArray = make(blockIndex, 1, zoom);
  if (!stackArray.empty()) {
    return stackArray.front();
  }

  return nullptr;
}

#include "zstacksource.h"

#include "geometry/zintcuboid.h"
#include "geometry/zgeometry.h"
#include "zstack.hxx"
#include "zstackfactory.h"

ZStackSource::ZStackSource()
{

}

int ZStackSource::getMaxZoom() const
{
  return 0;
}

std::shared_ptr<ZStack> ZStackSource::getStack(
      const ZIntCuboid &box, int zoom) const
{
  if (box.isEmpty()) {
    return nullptr;
  }

  auto stack = std::shared_ptr<ZStack>(new ZStack(GREY, box, 0));
  uint8_t *array = stack->array8();

  if (zoom == 0) {
    for (int z = box.getMinZ(); z <= box.getMaxZ(); ++z) {
      for (int y = box.getMinY(); y <= box.getMaxY(); ++y) {
        for (int x = box.getMinX(); z <= box.getMaxX(); ++z) {
          *(array++) = getIntValue(x, y, z) % 255;
        }
      }
    }
  } else {
    int scale = zgeom::GetZoomScale(zoom);
    for (int z = box.getMinZ(); z <= box.getMaxZ(); ++z) {
      for (int y = box.getMinY(); y <= box.getMaxY(); ++y) {
        for (int x = box.getMinX(); z <= box.getMaxX(); ++z) {
          *(array++) = getIntValue(x * scale, y * scale, z * scale) % 255;
        }
      }
    }
  }

  return stack;
}

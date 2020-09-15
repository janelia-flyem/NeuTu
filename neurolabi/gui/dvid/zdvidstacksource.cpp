#include "zdvidstacksource.h"

#include "geometry/zgeometry.h"
#include "zstack.hxx"

ZDvidStackSource::ZDvidStackSource()
{

}

void ZDvidStackSource::setDvidTarget(const ZDvidTarget &target)
{
  m_reader.open(target);
  m_reader.updateMaxGrayscaleZoom();
  m_dvidInfo = m_reader.readGrayScaleInfo();
}

int ZDvidStackSource::getMaxZoom() const
{
  return m_reader.getDvidTarget().getMaxGrayscaleZoom();
}
std::shared_ptr<ZStack> ZDvidStackSource::getStack(
    const ZIntCuboid &box, int zoom) const
{
  if (zoom > getMaxZoom()) {
    zoom = getMaxZoom();
  }

  int zoomRatio = zgeom::GetZoomScale(zoom);
  ZIntCuboid dataRange = m_dvidInfo.getDataRange();
  dataRange.scaleDown(zoomRatio);
  dataRange = dataRange.intersect(box);

  auto stack = std::shared_ptr<ZStack>(
        m_reader.readGrayScaleWithBlock(dataRange, zoom));
  if (stack) {
    if (zoom > 0) {
      int dsIntv = zgeom::GetZoomScale(zoom) - 1;
      stack->setDsIntv(dsIntv, dsIntv, dsIntv);
    }
  }

  return stack;
}

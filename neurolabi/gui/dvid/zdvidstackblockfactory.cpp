#include "zdvidstackblockfactory.h"

#include "geometry/zintpoint.h"

ZDvidStackBlockFactory::ZDvidStackBlockFactory()
{

}

const ZDvidReader& ZDvidStackBlockFactory::getReader() const
{
  return m_reader;
}

const ZDvidInfo& ZDvidStackBlockFactory::getDvidInfo() const
{
  return m_dvidInfo;
}

void ZDvidStackBlockFactory::setDvidTarget(const ZDvidTarget &target)
{
  if (m_reader.open(target.getGrayScaleTarget())) {
    m_reader.updateMaxGrayscaleZoom();
    m_dvidInfo = m_reader.readGrayScaleInfo();
  }
}

int ZDvidStackBlockFactory::getMaxZoom() const
{
  return getReader().getDvidTarget().getMaxGrayscaleZoom();
}

ZIntPoint ZDvidStackBlockFactory::getBlockSize() const
{
  if (m_reader.isReady()) {
    return m_dvidInfo.getBlockSize();
  }

  return ZStackBlockFactory::getBlockSize();
}

ZIntPoint ZDvidStackBlockFactory::getGridSize() const
{
  if (m_reader.isReady()) {
    return m_dvidInfo.getGridSize();
  }

  return ZStackBlockFactory::getGridSize();
}

std::vector<ZStack*> ZDvidStackBlockFactory::makeV(
    int i, int j, int k, int n, int zoom) const
{
  std::vector<ZStack*> result;

  if (getMaxZoom() >= zoom && n > 0) {
    if (getReader().isReady()) {
      result = getReader().readGrayScaleBlock(
            ZIntPoint(i, j, k), m_dvidInfo, n, zoom);
    }
  }

  return result;
}

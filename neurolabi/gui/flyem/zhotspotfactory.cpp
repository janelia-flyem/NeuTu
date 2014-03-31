#include "zhotspotfactory.h"

FlyEm::ZHotSpotFactory::ZHotSpotFactory()
{
}

FlyEm::ZHotSpot*
FlyEm::ZHotSpotFactory::createPointHotSpot(double x, double y, double z)
{
  FlyEm::ZHotSpot *hotSpot = new FlyEm::ZHotSpot;

  FlyEm::ZPointGeometry *geometry = new FlyEm::ZPointGeometry;
  geometry->setCenter(x, y, z);
  hotSpot->setGeometry(geometry);
  hotSpot->setType(FlyEm::ZHotSpot::TYPE_POINT);

  return hotSpot;
}

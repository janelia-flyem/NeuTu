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

FlyEm::ZHotSpot* FlyEm::ZHotSpotFactory::createCurveHotSpot()
{
  FlyEm::ZHotSpot *hotSpot = new FlyEm::ZHotSpot;
  FlyEm::ZCurveGeometry *geometry = new FlyEm::ZCurveGeometry;
  hotSpot->setGeometry(geometry);
  hotSpot->setType(FlyEm::ZHotSpot::TYPE_CURVE);

  return hotSpot;
}

FlyEm::ZHotSpot*
FlyEm::ZHotSpotFactory::createCurveHotSpot(const ZPointArray &pointArray)
{
  FlyEm::ZHotSpot *hotSpot = createCurveHotSpot();
  FlyEm::ZCurveGeometry *geometry =
      dynamic_cast<FlyEm::ZCurveGeometry*>(hotSpot->getGeometry());
  geometry->setAnchor(pointArray);

  return hotSpot;
}

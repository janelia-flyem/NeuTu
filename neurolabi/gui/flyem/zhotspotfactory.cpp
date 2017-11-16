#include "zhotspotfactory.h"

flyem::ZHotSpotFactory::ZHotSpotFactory()
{
}

flyem::ZHotSpot*
flyem::ZHotSpotFactory::createPointHotSpot(double x, double y, double z)
{
  flyem::ZHotSpot *hotSpot = new flyem::ZHotSpot;

  flyem::ZPointGeometry *geometry = new flyem::ZPointGeometry;
  geometry->setCenter(x, y, z);
  hotSpot->setGeometry(geometry);
  hotSpot->setType(flyem::ZHotSpot::TYPE_POINT);

  return hotSpot;
}

flyem::ZHotSpot* flyem::ZHotSpotFactory::createCurveHotSpot()
{
  flyem::ZHotSpot *hotSpot = new flyem::ZHotSpot;
  flyem::ZCurveGeometry *geometry = new flyem::ZCurveGeometry;
  hotSpot->setGeometry(geometry);
  hotSpot->setType(flyem::ZHotSpot::TYPE_CURVE);

  return hotSpot;
}

flyem::ZHotSpot*
flyem::ZHotSpotFactory::createCurveHotSpot(const ZPointArray &pointArray)
{
  flyem::ZHotSpot *hotSpot = createCurveHotSpot();
  flyem::ZCurveGeometry *geometry =
      dynamic_cast<flyem::ZCurveGeometry*>(hotSpot->getGeometry());
  geometry->setAnchor(pointArray);

  return hotSpot;
}

#ifndef ZHOTSPOTFACTORY_H
#define ZHOTSPOTFACTORY_H

#include "flyem/zhotspot.h"

namespace FlyEm {

class ZHotSpotFactory
{
public:
  ZHotSpotFactory();

public:
  static ZHotSpot* createPointHotSpot(double x, double y, double z);
  static ZHotSpot* createCurveHotSpot();
  static ZHotSpot* createCurveHotSpot(const ZPointArray &pointArray);
};

}

#endif // ZHOTSPOTFACTORY_H

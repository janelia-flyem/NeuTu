#include "zflyemutilities.h"
#include <cmath>
#include <algorithm>

double FlyEm::GetFlyEmRoiMarkerRadius(double s)
{
  return 10.0 + s / 2000;
}

double FlyEm::GetFlyEmRoiMarkerRadius(double width, double height)
{
  return GetFlyEmRoiMarkerRadius(std::min(width, height));
}

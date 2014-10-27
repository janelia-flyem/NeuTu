#include "zflyemutilities.h"
#include <cmath>
#include <algorithm>

double FlyEm::GetFlyEmRoiMarkerRadius(double s)
{
  return 20.0 + s / 200;
}

double FlyEm::GetFlyEmRoiMarkerRadius(double width, double height)
{
  return GetFlyEmRoiMarkerRadius(std::min(width, height));
}

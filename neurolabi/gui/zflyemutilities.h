#ifndef ZFLYEMUTILITIES_H
#define ZFLYEMUTILITIES_H

#include <string>
#include <set>

#include "tz_stdint.h"

class ZIntPoint;
class ZStack;

namespace flyem
{

double GetFlyEmRoiMarkerRadius(double s);
double GetFlyEmRoiMarkerRadius(double width, double height);
std::set<uint64_t> LoadBodySet(const std::string &filePath);


ZIntPoint FindClosestBg(const ZStack *stack, int x, int y, int z);
}

#endif // ZFLYEMUTILITIES_H

#ifndef ZFLYEMUTILITIES_H
#define ZFLYEMUTILITIES_H

#include <string>
#include <set>

namespace FlyEm
{

double GetFlyEmRoiMarkerRadius(double s);
double GetFlyEmRoiMarkerRadius(double width, double height);
std::set<uint64_t> LoadBodySet(const std::string &filePath);

}

#endif // ZFLYEMUTILITIES_H

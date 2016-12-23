#include "zflyemutilities.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include "zstring.h"

double FlyEm::GetFlyEmRoiMarkerRadius(double s)
{
  return 10.0 + s / 2000;
}

double FlyEm::GetFlyEmRoiMarkerRadius(double width, double height)
{
  return GetFlyEmRoiMarkerRadius(std::min(width, height));
}

std::set<uint64_t> FlyEm::LoadBodySet(const std::string &input)
{
//  ZString

  std::set<uint64_t> bodySet;

  FILE *fp = fopen(input.c_str(), "r");
  if (fp != NULL) {
    ZString str;
    while (str.readLine(fp)) {
      std::vector<uint64_t> bodyArray = str.toUint64Array();
      bodySet.insert(bodyArray.begin(), bodyArray.end());
    }
    fclose(fp);
  } else {
    std::cout << "Failed to open " << input << std::endl;
  }

  return bodySet;
}

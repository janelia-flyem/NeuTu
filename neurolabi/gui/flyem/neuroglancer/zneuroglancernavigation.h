#ifndef ZNEUROGLANCERNAVIGATION_H
#define ZNEUROGLANCERNAVIGATION_H

#include <string>

//#include "geometry/zintpoint.h"
//#include "geometry/zpoint.h"

class ZJsonObject;

class ZNeuroglancerNavigation
{
public:
  ZNeuroglancerNavigation();

  void setVoxelSize(int x, int y, int z);
  void setCoordinates(double x, double y, double z);

  ZJsonObject toJsonObject() const;
//  std::string toPathString() const;

private:
  int m_voxelSize[3] = {1, 1, 1};
  double m_voxelCoordinates[3] = {1000, 1000, 1000};
  double m_zoomFactor = 32;
};

#endif // ZNEUROGLANCERNAVIGATION_H

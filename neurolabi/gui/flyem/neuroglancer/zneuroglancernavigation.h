#ifndef ZNEUROGLANCERNAVIGATION_H
#define ZNEUROGLANCERNAVIGATION_H

#include <string>

//#include "geometry/zintpoint.h"
//#include "geometry/zpoint.h"

#include "zresolution.h"

class ZJsonObject;

class ZNeuroglancerNavigation
{
public:
  ZNeuroglancerNavigation();

  //For current neuroglancer link
  void setPosition(double x, double y, double z);
  void setPosition(const ZPoint &pos);
  void configureJson(ZJsonObject &objRef) const;
  void setVoxelSize(double x, double y, double z, char unit);
  void setVoxelSize(double x, double y, double z, ZResolution::EUnit unit);
  void setVoxelSize(const ZResolution &res);
  void setZoomScale2D(double s);
  void setProjectionScale(double s);

  //For legacy neuroglancer link
  void setVoxelSize(int x, int y, int z);
  void setCoordinates(double x, double y, double z);
  ZJsonObject toJsonObject() const;

private:
//  int m_voxelSize[3] = {1, 1, 1};
  ZResolution m_voxelSize;
  double m_voxelCoordinates[3] = {1000, 1000, 1000};
  double m_crossSectionScale = 1.0;
  double m_projectionScale = 0;
};

#endif // ZNEUROGLANCERNAVIGATION_H

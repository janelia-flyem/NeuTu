#include "zneuroglancernavigation.h"

#include "zjsonobject.h"

ZNeuroglancerNavigation::ZNeuroglancerNavigation()
{

}

ZJsonObject ZNeuroglancerNavigation::toJsonObject() const
{
  ZJsonObject obj;
  ZJsonObject poseObj;
  ZJsonObject positionObj;
  positionObj.setEntry("voxelSize", m_voxelSize, 3);
  positionObj.setEntry("voxelCoordinates", m_voxelCoordinates, 3);
  poseObj.setEntry("position", positionObj);
  obj.setEntry("pose", poseObj);
  obj.setEntry("zoomFactor", m_zoomFactor);

  return obj;
}

void ZNeuroglancerNavigation::setVoxelSize(int x, int y, int z)
{
  m_voxelSize[0] = x;
  m_voxelSize[1] = y;
  m_voxelSize[2] = z;
}

void ZNeuroglancerNavigation::setCoordinates(double x, double y, double z)
{
  m_voxelCoordinates[0] = x;
  m_voxelCoordinates[1] = y;
  m_voxelCoordinates[2] = z;
}

#include "zneuroglancernavigation.h"

#include "zjsonobject.h"
#include "zjsonarray.h"

ZNeuroglancerNavigation::ZNeuroglancerNavigation()
{

}

ZJsonObject ZNeuroglancerNavigation::toJsonObject() const
{
  ZJsonObject obj;
  ZJsonObject poseObj;
  ZJsonObject positionObj;
  double voxelSize[3];
  voxelSize[0] = m_voxelSize.getVoxelSize(
        neutu::EAxis::X, ZResolution::EUnit::UNIT_NANOMETER);
  voxelSize[1] = m_voxelSize.getVoxelSize(
        neutu::EAxis::Y, ZResolution::EUnit::UNIT_NANOMETER);
  voxelSize[2] = m_voxelSize.getVoxelSize(
        neutu::EAxis::Z, ZResolution::EUnit::UNIT_NANOMETER);
  positionObj.setEntry("voxelSize", voxelSize, 3);
  positionObj.setEntry("voxelCoordinates", m_voxelCoordinates, 3);
  poseObj.setEntry("position", positionObj);
  obj.setEntry("pose", poseObj);
  obj.setEntry("zoomFactor", m_crossSectionScale);

  return obj;
}

/*
void ZNeuroglancerNavigation::setVoxelSize(int x, int y, int z)
{
  m_voxelSize[0] = x;
  m_voxelSize[1] = y;
  m_voxelSize[2] = z;
}
*/

void ZNeuroglancerNavigation::setCoordinates(double x, double y, double z)
{
  m_voxelCoordinates[0] = x;
  m_voxelCoordinates[1] = y;
  m_voxelCoordinates[2] = z;
}

void ZNeuroglancerNavigation::setPosition(double x, double y, double z)
{
  setCoordinates(x, y, z);
}

void ZNeuroglancerNavigation::setPosition(const ZPoint &pos)
{
  setPosition(pos.getX(), pos.getY(), pos.getZ());
}

void ZNeuroglancerNavigation::setVoxelSize(int x, int y, int z)
{
  m_voxelSize.setVoxelSize(x, y, z);
  m_voxelSize.setUnit(ZResolution::EUnit::UNIT_PIXEL);
}

void ZNeuroglancerNavigation::setVoxelSize(
    double x, double y, double z, char unit)
{
  m_voxelSize.setVoxelSize(x, y, z);
  m_voxelSize.setUnit(unit);
}

void ZNeuroglancerNavigation::setVoxelSize(
    double x, double y, double z, ZResolution::EUnit unit)
{
  m_voxelSize.setVoxelSize(x, y, z);
  m_voxelSize.setUnit(unit);
}

void ZNeuroglancerNavigation::setVoxelSize(const ZResolution &res)
{
  m_voxelSize = res;
}

void ZNeuroglancerNavigation::setZoomScale2D(double s)
{
  m_crossSectionScale = 1.0/s;
}

namespace {

void configure_dim(ZJsonObject &objRef, const char *key, double value)
{
  ZJsonArray dim;
  dim.append(value);
  dim.append("nm");
  objRef.setEntry(key, dim);
}

}

void ZNeuroglancerNavigation::configureJson(ZJsonObject &objRef) const
{
  objRef.setEntry("position", m_voxelCoordinates, 3);
  objRef.setEntry("crossSectionScale", m_crossSectionScale);

  ZPoint dims = m_voxelSize.getVoxelDims(ZResolution::EUnit::UNIT_NANOMETER);
  ZJsonObject dimObject;
  configure_dim(dimObject, "x", dims.getX());
  configure_dim(dimObject, "y", dims.getY());
  configure_dim(dimObject, "z", dims.getZ());

  objRef.setEntry("dimensions", dimObject);
}

#include "zflyemcoordinateconverter.h"
#include "zerror.h"


ZFlyEmCoordinateConverter::ZFlyEmCoordinateConverter() :
  m_zStart(1490), m_margin(10)
{
  m_stackSize.resize(3, 0);
  m_voxelResolution.resize(3, 1.0);
}

void ZFlyEmCoordinateConverter::setStackSize(int xDim, int yDim, int zDim)
{
  m_stackSize[0] = xDim;
  m_stackSize[1] = yDim;
  m_stackSize[2] = zDim;
}

void ZFlyEmCoordinateConverter::setVoxelResolution(
    double xRes, double yRes, double zRes)
{
  m_voxelResolution[0] = xRes;
  m_voxelResolution[1] = yRes;
  m_voxelResolution[2] = zRes;
}

void ZFlyEmCoordinateConverter::setZStart(int zStart)
{
  m_zStart = zStart;
}

void ZFlyEmCoordinateConverter::setMargin(int margin)
{
  m_margin = margin;
}

void ZFlyEmCoordinateConverter::convertFromRavelerSpace(
    double *x, double *y, double *z, ESpace target)
{
  switch(target) {
  case IMAGE_SPACE:
    if (m_stackSize[1] > 0) {
      *y = m_stackSize[1] - *y + 1;
    }
    break;
  case PHYSICAL_SPACE:
  case ROI_SPACE:
    convertFromRavelerSpace(x, y, z, IMAGE_SPACE);
    convertFromImageSpace(x, y, z, target);
    break;
  default:
    break;
  }
}

void ZFlyEmCoordinateConverter::convertFromImageSpace(
    double *x, double *y, double *z, ESpace target)
{
  switch(target) {
  case RAVELER_SPACE:
    if (m_stackSize[1] > 0) {
      *y = m_stackSize[1] - *y + 1;
    }
    break;
  case PHYSICAL_SPACE:
    *x *= m_voxelResolution[0];
    *y *= m_voxelResolution[1];
    *z *= m_voxelResolution[2];
    break;
  case ROI_SPACE:
    *x -= m_margin;
    *y -= m_margin;
    *z -= m_margin + m_zStart;
    break;
  default:
    break;
  }
}

void ZFlyEmCoordinateConverter::convertFromPhysicalSpace(
    double *x, double *y, double *z, ESpace target)
{
  switch(target) {
  case IMAGE_SPACE:
    *x /= m_voxelResolution[0];
    *y /= m_voxelResolution[1];
    *z /= m_voxelResolution[2];
    break;
  case RAVELER_SPACE:
  case ROI_SPACE:
    convertFromPhysicalSpace(x, y, z, IMAGE_SPACE);
    convertFromImageSpace(x, y, z, target);
    break;
  default:
    break;
  }
}

void ZFlyEmCoordinateConverter::convertFromRoiSpace(
    double *x, double *y, double *z, ESpace target)
{
  switch(target) {
  case IMAGE_SPACE:
    *x += m_margin;
    *y += m_margin;
    *z += m_margin + m_zStart;
    break;
  case RAVELER_SPACE:
  case PHYSICAL_SPACE:
    convertFromRoiSpace(x, y, z, IMAGE_SPACE);
    convertFromImageSpace(x, y, z, target);
    break;
  default:
    break;
  }
}

void ZFlyEmCoordinateConverter::convert(
    double *x, double *y, double *z, ESpace source, ESpace target)
{
  switch (source) {
  case RAVELER_SPACE:
    convertFromRavelerSpace(x, y, z, target);
    break;
  case IMAGE_SPACE:
    convertFromImageSpace(x, y, z, target);
    break;
  case PHYSICAL_SPACE:
    convertFromPhysicalSpace(x, y, z, target);
    break;
  case ROI_SPACE:
    convertFromRoiSpace(x, y, z, target);
    break;
  default:
    RECORD_WARNING_UNCOND("Unknown space.");
  }
}

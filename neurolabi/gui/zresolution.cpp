#include "zresolution.h"

#include <cmath>

#include "zjsonobject.h"
#include "zjsonparser.h"

ZResolution::ZResolution()
{
  reset();
}

void ZResolution::reset()
{
  m_voxelSize[0] = 1.0;
  m_voxelSize[1] = 1.0;
  m_voxelSize[2] = 1.0;
  m_unit = 'p';
}

void ZResolution::setUnit(const std::string &unit)
{
  if (unit == "um" || unit == "microns") {
    m_unit = 'u';
  } else if (unit == "nm" || unit == "nanometers") {
    m_unit = 'n';
  } else {
    m_unit = 'p';
  }
}

std::string ZResolution::getUnitName() const
{
  switch (m_unit) {
  case 'u':
    return "um";
  case 'n':
    return "nm";
  case 'p':
    return "pixel";
  }

  return "undefined";
}

ZResolution::EUnit ZResolution::getUnit() const
{
  switch (m_unit) {
  case 'u':
    return UNIT_MICRON;
  case 'n':
    return UNIT_NANOMETER;
  case 'p':
    return UNIT_PIXEL;
  }

  return UNIT_PIXEL;
}

void ZResolution::setUnit(EUnit unit)
{
  switch (unit) {
  case UNIT_PIXEL:
    m_unit = 'p';
    break;
  case UNIT_MICRON:
    m_unit = 'u';
    break;
  case UNIT_NANOMETER:
    m_unit = 'n';
    break;
  }
}

void ZResolution::convertUnit(char unit)
{
  if (m_unit == 'u' && unit == 'n') {
    for (size_t i = 0; i < 3; ++i) {
      m_voxelSize[i] *= 1000.0;
    }
  } else if (unit == 'u' && m_unit == 'n') {
    for (size_t i = 0; i < 3; ++i) {
      m_voxelSize[i] /= 1000.0;
    }
  } else if (unit == 'p' && m_unit != 'p') {
    m_voxelSize[2] /= m_voxelSize[0];
    m_voxelSize[1] /= m_voxelSize[0];
    m_voxelSize[0] = 1.0;
  }
  m_unit = unit;
}


void ZResolution::loadJsonObject(const ZJsonObject &obj)
{
  reset();

  if (obj.hasKey("unit")) {
    setUnit(ZJsonParser::stringValue(obj["unit"]));
  }

  if (obj.hasKey("voxel_size")) {
    ZJsonArray sizeJson(obj.value("voxel_size"));
    if (sizeJson.size() == 3) {
      setVoxelSize(ZJsonParser::numberValue(sizeJson.at(0)),
                   ZJsonParser::numberValue(sizeJson.at(1)),
                   ZJsonParser::numberValue(sizeJson.at(2)));
    }
  }
}

double ZResolution::getUnitVoxelSize(EUnit unit) const
{
  double v = 1.0;

  EUnit currentUnit = getUnit();
  if (currentUnit != unit) {
    switch (unit) {
    case UNIT_MICRON:
      if (currentUnit == UNIT_NANOMETER) {
        v /= 1000.0;
      }
      break;
    case UNIT_NANOMETER:
      if (currentUnit == UNIT_MICRON) {
        v *= 1000.0;
      }
      break;
    case UNIT_PIXEL:
      break;
    }
  }

  return v;
}

double ZResolution::getVoxelSize(neutube::EAxis axis, EUnit unit) const
{
  return m_voxelSize[neutube::EnumValue(axis)] * getUnitVoxelSize(unit);
}

double ZResolution::getPlaneVoxelSize(neutube::EPlane plane) const
{
  double v = 1.0;
  switch (plane) {
  case neutube::EPlane::XY:
    v = voxelSizeX() * voxelSizeY();
    break;
  case neutube::EPlane::YZ:
    v = voxelSizeY() * voxelSizeZ();
    break;
  case neutube::EPlane::XZ:
    v = voxelSizeX() * voxelSizeZ();
    break;
  }

  return v;
}

double ZResolution::getPlaneVoxelSpan(neutube::EPlane plane) const
{
  double v1 = 1.0;
  double v2 = 1.0;
  switch (plane) {
  case neutube::EPlane::XY:
    v1 = voxelSizeX();
    v2 = voxelSizeY();
    break;
  case neutube::EPlane::YZ:
    v1 = voxelSizeY();
    v2 = voxelSizeZ();
    break;
  case neutube::EPlane::XZ:
    v1 = voxelSizeX();
    v2 = voxelSizeZ();
    break;
  }

  double v = 1.0;
  if (v1 == v2) {
    v = v1;
  } else {
    v = sqrt(v1 * v2);
  }

  return v;
}

double ZResolution::getPlaneVoxelSize(neutube::EPlane plane, EUnit unit) const
{
  double v = 1.0;
  switch (plane) {
  case neutube::EPlane::XY:
    v = getVoxelSize(neutube::EAxis::X, unit) * getVoxelSize(neutube::EAxis::Y, unit);
    break;
  case neutube::EPlane::YZ:
    v = getVoxelSize(neutube::EAxis::Y, unit) * getVoxelSize(neutube::EAxis::Z, unit);
    break;
  case neutube::EPlane::XZ:
    v = getVoxelSize(neutube::EAxis::X, unit) * getVoxelSize(neutube::EAxis::Z, unit);
    break;
  }

  return v;
}

double ZResolution::getPlaneVoxelSpan(neutube::EPlane plane, EUnit unit) const
{
  return getPlaneVoxelSpan(plane) * getUnitVoxelSize(unit);
}

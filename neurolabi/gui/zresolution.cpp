#include "zresolution.h"

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

void ZResolution::set(double x, double y, double z, char unit)
{
  setVoxelSize(x, y, z);
  setUnit(unit);
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

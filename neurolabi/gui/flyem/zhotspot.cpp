#include "zhotspot.h"

#include <iostream>
#include <sstream>
#include "zstring.h"
#include "zjsonarray.h"
#include "tz_math.h"

void FlyEm::ZGeometry::print() const
{
  std::cout << "FlyEm::ZGeometry::print():" << std::endl;
  std::cout << "to do" << std::endl;
}

void FlyEm::ZPointGeometry::setCenter(double x, double y, double z)
{
  m_center.set(x, y, z);
}

ZTextLineCompositer FlyEm::ZPointGeometry::toLineCompositer() const
{
  ZTextLineCompositer compositer;
  compositer.appendLine("Point: " + m_center.toString());

  return compositer;
}

ZJsonObject FlyEm::ZPointGeometry::toJsonObject() const
{
  ZJsonObject obj;
  double coords[3];
  coords[0] = m_center.x();
  coords[1] = m_center.y();
  coords[2] = m_center.z();
  obj.setEntry("center", coords, 3);

  return obj;
}

void FlyEm::ZStructureInfo::print() const
{
  std::cout << toLineCompositer().toString(2) << std::endl;
}

ZTextLineCompositer FlyEm::ZStructureInfo::toLineCompositer() const
{
  ZTextLineCompositer compositer;
  compositer.appendLine("Structure:");

  std::ostringstream stream;
  stream << "Source: " << m_sourceBody;
  compositer.appendLine(stream.str(), 1);

  std::ostringstream stream2;
  stream2 << "Target: ";
  for (std::vector<int>::const_iterator iter = m_targetBodyArray.begin();
       iter != m_targetBodyArray.end(); ++iter) {
    stream2 << *iter << ' ';
  }
  compositer.appendLine(stream2.str(), 1);

  return compositer;
}

std::string FlyEm::ZStructureInfo::getTypeString() const
{
  switch (m_type) {
  case TYPE_MERGE:
    return "merge";
  case TYPE_SPLIT:
    return "split";
  case TYPE_UNKNOWN:
    return "unknown";
  }

  return "";
}

ZJsonObject FlyEm::ZStructureInfo::toJsonObject() const
{
  ZJsonObject obj;
  obj.setEntry("type", getTypeString());
  obj.setEntry("source", m_sourceBody);
  obj.setEntry("target", &(m_targetBodyArray[0]), m_targetBodyArray.size());

  return obj;
}

FlyEm::ZHotSpot::ZHotSpot() :
  m_geometry(NULL), m_structInfo(NULL), m_confidence(0), m_type(TYPE_POINT)
{
}

FlyEm::ZHotSpot::~ZHotSpot()
{
  delete m_geometry;
  delete m_structInfo;
}

void FlyEm::ZHotSpot::print() const
{
  std::cout << toLineCompositer().toString(2) << std::endl;
}

ZTextLineCompositer FlyEm::ZHotSpot::toLineCompositer() const
{
  ZTextLineCompositer compositer;
  compositer.appendLine("Hot Spot:");
  if (m_geometry != NULL) {
    compositer.appendLine(m_geometry->toLineCompositer(), 1);
  } else {
    compositer.appendLine("No geometrical information", 1);
  }

  if (m_structInfo != NULL) {
    compositer.appendLine(m_structInfo->toLineCompositer(), 1);
  } else {
    compositer.appendLine("No structural information", 1);
  }

  std::ostringstream stream;
  stream << "Confidence: " << m_confidence;
  compositer.appendLine(stream.str(), 1);

  return compositer;
}

void FlyEm::ZHotSpot::setGeometry(FlyEm::ZGeometry *geometry)
{
  if (m_geometry != NULL) {
    delete m_geometry;
  }

  m_geometry = geometry;
}

void FlyEm::ZHotSpot::setStructure(ZStructureInfo *structure)
{
  if (m_structInfo != NULL) {
    delete m_structInfo;
  }

  m_structInfo = structure;
}

std::string FlyEm::ZHotSpot::getTypeString() const
{
  switch (m_type) {
  case TYPE_POINT:
    return "point";
  case TYPE_REGION:
    return "region";
  case TYPE_SURFACE:
    return "surface";
  }

  return "";
}

ZJsonObject FlyEm::ZHotSpot::toJsonObject() const
{
  ZJsonObject obj;
  obj.setEntry("Confidence", m_confidence);
  obj.setEntry("Type", getTypeString());
  if (m_geometry != NULL) {
    ZJsonObject geometryObject = m_geometry->toJsonObject();
    obj.setEntry("Geometry", geometryObject);
  }
  if (m_structInfo != NULL) {
    ZJsonObject structObject = m_structInfo->toJsonObject();
    obj.setEntry("Structure", structObject);
  }

  return obj;
}

ZJsonObject FlyEm::ZHotSpot::toRavelerJsonObject(
    const double *resolution, const int *imageSize) const
{
  ZJsonObject obj;
  obj.setEntry("text", getTypeString());
  if (m_structInfo != NULL) {
    obj.setEntry("body ID", m_structInfo->getSource());
  }
  if (m_geometry != NULL) {


    ZPointGeometry *geometry = dynamic_cast<ZPointGeometry*>(m_geometry);
    if (geometry != NULL) {
      int x = iround(geometry->getCenter().x() / resolution[0]);
      int y = iround(geometry->getCenter().y() / resolution[1]);
      y = imageSize[1] - y + 1;
      int z = iround(geometry->getCenter().z() / resolution[2]);

      ZJsonArray arrayObj;
      arrayObj <<  x << y << z;
      obj.setEntry("location", arrayObj);
    }
  }

  return obj;
}

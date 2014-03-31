#include "zhotspot.h"

#include <iostream>
#include <sstream>
#include "zstring.h"

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

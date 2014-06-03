#include "zhotspot.h"

#include <iostream>
#include <sstream>
#include "zstring.h"
#include "zjsonarray.h"
#include "tz_math.h"
#include "flyem/zflyemcoordinateconverter.h"

void FlyEm::ZGeometry::print() const
{
  std::cout << toLineCompositer().toString(2) << std::endl;
  /*
  std::cout << "FlyEm::ZGeometry::print():" << std::endl;
  std::cout << "to do" << std::endl;
  */
}


/******************ZPointGeometry*******************/
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

ZPointArray FlyEm::ZPointGeometry::toPointArray() const
{
  ZPointArray ptArray;

  ptArray.append(getCenter());

  return ptArray;
}

ZLineSegmentArray FlyEm::ZPointGeometry::toLineSegmentArray() const
{
  return ZLineSegmentArray();
}
//////////////////ZPointGeometry//////////////////////////

/******************ZCurveGeometry*******************/
void FlyEm::ZCurveGeometry::appendPoint(double x, double y, double z)
{
  m_curve.append(x, y, z);
}

void FlyEm::ZCurveGeometry::appendPoint(const ZPoint &pt)
{
  m_curve.append(pt);
}

ZTextLineCompositer FlyEm::ZCurveGeometry::toLineCompositer() const
{
  ZTextLineCompositer compositer;
  compositer.appendLine("Curve: ");
  for (size_t i = 0; i < m_curve.size(); ++i) {
    const ZPoint &pt = m_curve[i];
    compositer.appendLine(pt.toString(), 1);
  }

  return compositer;
}

ZJsonObject FlyEm::ZCurveGeometry::toJsonObject() const
{
  ZJsonObject obj;
  ZJsonArray ptArray(obj.setArrayEntry("curve"), false);
  for (size_t i = 0; i < m_curve.size(); ++i) {
    const ZPoint &pt = m_curve[i];
    ZJsonArray ptObj;
    ptObj << pt[0] << pt[1] << pt[2];
    ptArray.append(ptObj);
  }

  return obj;
}

size_t FlyEm::ZCurveGeometry::getAnchorNumber() const
{
  return m_curve.size();
}

const ZPoint& FlyEm::ZCurveGeometry::getAnchor(size_t index) const
{
  return m_curve[index];
}

void FlyEm::ZCurveGeometry::setAnchor(const ZPointArray &curve)
{
  m_curve = curve;
}

ZPointArray FlyEm::ZCurveGeometry::toPointArray() const
{
  return m_curve;
}

ZLineSegmentArray FlyEm::ZCurveGeometry::toLineSegmentArray() const
{
  ZLineSegmentArray lineArray;
  for (size_t i = 1; i < m_curve.size(); ++i) {
    lineArray.append(m_curve[i - 1], m_curve[i]);
  }

  return lineArray;
}

//////////////////ZCurveGeometry//////////////////////////

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
  m_geometry(NULL), m_guidance(NULL), m_structInfo(NULL),
  m_confidence(0), m_type(TYPE_POINT)
{
}

FlyEm::ZHotSpot::~ZHotSpot()
{
  delete m_geometry;
  delete m_guidance;
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

  if (m_guidance != NULL) {
    compositer.appendLine("guidance: ", 1);
    compositer.appendLine(m_guidance->toLineCompositer(), 2);
  } else {
    compositer.appendLine("No guidance", 1);
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

void FlyEm::ZHotSpot::setGuidance(ZGeometry *geometry)
{
  if (m_guidance != NULL) {
    delete m_guidance;
  }

  m_guidance = geometry;
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
  case TYPE_CURVE:
    return "curve";
  }

  return "";
}

ZJsonObject FlyEm::ZHotSpot::toJsonObject() const
{
  ZJsonObject obj;
  obj.setEntry("confidence", m_confidence);
  obj.setEntry("type", getTypeString());
  if (m_geometry != NULL) {
    ZJsonObject geometryObject = m_geometry->toJsonObject();
    obj.setEntry("geometry", geometryObject);
  }
  if (m_guidance != NULL) {
    ZJsonObject guidanceObject = m_guidance->toJsonObject();
    obj.setEntry("guidance", guidanceObject);
  }
  if (m_structInfo != NULL) {
    ZJsonObject structObject = m_structInfo->toJsonObject();
    obj.setEntry("structure", structObject);
  }

  return obj;
}

ZPointArray FlyEm::ZHotSpot::toPointArray() const
{
  ZPointArray ptArray;
  if (m_geometry != NULL) {
    ptArray = m_geometry->toPointArray();
  }

  if (m_guidance != NULL) {
    ptArray.append(m_guidance->toPointArray());
  }

  return ptArray;
}

ZLineSegmentArray FlyEm::ZHotSpot::toLineSegmentArray() const
{
  ZLineSegmentArray lineArray;
  if (m_geometry != NULL) {
    lineArray = m_geometry->toLineSegmentArray();
  }

  if (m_guidance != NULL) {
    lineArray.append(m_guidance->toLineSegmentArray());
  }

  return lineArray;
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
    ZPointArray ptArray = toPointArray();

    ZFlyEmCoordinateConverter converter;
    converter.setStackSize(imageSize[0], imageSize[1], imageSize[2]);
    converter.setVoxelResolution(resolution[0], resolution[1], resolution[2]);
    for (size_t i = 0; i < ptArray.size(); ++i) {
      ZPoint &pt = ptArray[i];
      converter.convert(&pt, ZFlyEmCoordinateConverter::PHYSICAL_SPACE,
                        ZFlyEmCoordinateConverter::RAVELER_SPACE);
      ZJsonArray arrayObj;
      arrayObj <<  iround(pt.x()) << iround(pt.y()) << iround(pt.z());
      obj.setEntry("location", arrayObj);
    }
#if 0
    ZPointGeometry *geometry = dynamic_cast<ZPointGeometry*>(m_geometry);
    if (geometry != NULL) {
      ptArray = geometry->toPointArray();
    }

    ZCurveGeometry

      int x = iround(geometry->getCenter().x() / resolution[0]);
      int y = iround(geometry->getCenter().y() / resolution[1]);
      y = imageSize[1] - y + 1;
      int z = iround(geometry->getCenter().z() / resolution[2]);

      ZJsonArray arrayObj;
      arrayObj <<  x << y << z;
      obj.setEntry("location", arrayObj);
    } else {
      ZCurveGeometry *geometry = dynamic_cast<ZCurveGeometry*>(m_geometry);

      if (geometry != NULL) {
        ZFlyEmCoordinateConverter converter;
        converter.setStackSize(imageSize[0], imageSize[1], imageSize[2]);
        converter.setVoxelResolution(resolution[0], resolution[1], resolution[2]);
        for (size_t i = 0; i < geometry->getAnchorNumber(); ++i) {
          ZPoint pt = geometry->getAnchor(i);
          converter.convert(&pt, ZFlyEmCoordinateConverter::PHYSICAL_SPACE,
                            ZFlyEmCoordinateConverter::RAVELER_SPACE);
          ZJsonArray arrayObj;
          arrayObj <<  iround(pt.x()) << iround(pt.y()) << iround(pt.z());
          obj.setEntry("location", arrayObj);
        }
      }
    }
#endif
  }

  return obj;
}

bool FlyEm::ZHotSpot::compareConfidence(
    const ZHotSpot *spot1, const ZHotSpot *spot2)
{
  return spot1->getConfidence() > spot2->getConfidence();
}

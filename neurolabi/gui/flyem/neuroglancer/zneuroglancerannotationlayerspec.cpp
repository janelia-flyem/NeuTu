#include "zneuroglancerannotationlayerspec.h"

#include "zjsonarray.h"
#include "geometry/zintpoint.h"

#include "neulib/core/stringbuilder.h"
//#include "common/zstringbuilder.h"

ZNeuroglancerAnnotationLayerSpec::ZNeuroglancerAnnotationLayerSpec()
{
  setType("annotation");
}

ZJsonObject ZNeuroglancerAnnotationLayerSpec::toJsonObject() const
{
  ZJsonObject rootObj = ZNeuroglancerLayerSpec::toJsonObject();
  rootObj.setEntry("voxelSize", m_voxelSize, 3);
  rootObj.setNonEmptyEntry("tool", m_tool);
  rootObj.setNonEmptyEntry("linkedSegmentationLayer", m_linkedSegmentationLayer);
  rootObj.setEntry("annotationFillOpacity", m_opacity);
  if (m_color.size() == 3) {

    std::string colorStr = neulib::StringBuilder("rgba(").
        append(m_color[0]).append(",").
        append(m_color[1]).append(",").
        append(m_color[2]).append(",1.0)");
    rootObj.setEntry("annotationColor", colorStr);
  }

  ZJsonArray annotationJson;
  for (const ZJsonObject &obj : m_annotationList) {
    annotationJson.append(obj);
  }
  if (!annotationJson.isEmpty()) {
    rootObj.setEntry("annotations", annotationJson);
  }

  return rootObj;
}

void ZNeuroglancerAnnotationLayerSpec::setLinkedSegmentation(
    const std::string &layerName)
{
  m_linkedSegmentationLayer = layerName;
}

void ZNeuroglancerAnnotationLayerSpec::setTool(const std::string &tool)
{
  m_tool = tool;
}

void ZNeuroglancerAnnotationLayerSpec::setOpacity(double opacity)
{
  m_opacity = opacity;
}

void ZNeuroglancerAnnotationLayerSpec::setColor(uint8_t r, uint8_t g, uint8_t b)
{
  m_color.resize(3, 255);
  m_color[0] = r;
  m_color[1] = g;
  m_color[2] = b;
}

void ZNeuroglancerAnnotationLayerSpec::setVoxelSize(int x, int y, int z)
{
  m_voxelSize[0] = x;
  m_voxelSize[1] = y;
  m_voxelSize[2] = z;
}

void ZNeuroglancerAnnotationLayerSpec::setVoxelSize(const ZIntPoint &s)
{
  setVoxelSize(s.getX(), s.getY(), s.getZ());
}

void ZNeuroglancerAnnotationLayerSpec::addAnnotation(const ZJsonObject &obj)
{
  if (!obj.isEmpty()) {
    m_annotationList.push_back(obj);
  }
}

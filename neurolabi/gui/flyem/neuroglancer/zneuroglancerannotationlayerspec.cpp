#include "zneuroglancerannotationlayerspec.h"

#include "zjsonarray.h"

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

void ZNeuroglancerAnnotationLayerSpec::setVoxelSize(int x, int y, int z)
{
  m_voxelSize[0] = x;
  m_voxelSize[1] = y;
  m_voxelSize[2] = z;
}

void ZNeuroglancerAnnotationLayerSpec::addAnnotation(const ZJsonObject &obj)
{
  if (!obj.isEmpty()) {
    m_annotationList.push_back(obj);
  }
}

#ifndef ZNEUROGLANCERANNOTATIONLAYERSPEC_H
#define ZNEUROGLANCERANNOTATIONLAYERSPEC_H

#include <vector>

#include "zneuroglancerlayerspec.h"

#include "zjsonobject.h"

class ZIntPoint;

/*!
 * \brief The class for annotation layer specification
 */
class ZNeuroglancerAnnotationLayerSpec : public ZNeuroglancerLayerSpec
{
public:
  ZNeuroglancerAnnotationLayerSpec();

  void setVoxelSize(int x, int y, int z);
  void setVoxelSize(const ZIntPoint &s);
  void setTool(const std::string &tool);
  void setLinkedSegmentation(const std::string &layerName);
  void addAnnotation(const ZJsonObject &obj);

  ZJsonObject toJsonObject() const override;

private:
  std::string m_tool;
  int m_voxelSize[3] = {1, 1, 1};
  std::string m_linkedSegmentationLayer;
  std::vector<ZJsonObject> m_annotationList;
};

#endif // ZNEUROGLANCERANNOTATIONLAYERSPEC_H

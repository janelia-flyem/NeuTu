#ifndef ZNEUROGLANCERANNOTATIONLAYERSPEC_H
#define ZNEUROGLANCERANNOTATIONLAYERSPEC_H

#include <vector>
#include <cstdint>

#include "zneuroglancerlayerspec.h"

#include "zjsonobject.h"

class ZIntPoint;
class ZCuboid;

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
  void setOpacity(double opacity);
  void addAnnotation(const ZJsonObject &obj);
  void addAnnotation(const ZCuboid &box);
  void setColor(uint8_t r, uint8_t g, uint8_t b);

  ZJsonObject toJsonObject() const override;

private:
  std::string m_tool;
  double m_opacity = 1.0;
  std::vector<uint8_t> m_color;
  int m_voxelSize[3] = {1, 1, 1};
  std::string m_linkedSegmentationLayer;
  std::vector<ZJsonObject> m_annotationList;
};

#endif // ZNEUROGLANCERANNOTATIONLAYERSPEC_H

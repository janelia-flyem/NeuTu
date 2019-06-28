#ifndef ZNEUROGLANCERLAYERSPECFACTORY_H
#define ZNEUROGLANCERLAYERSPECFACTORY_H

#include <memory>

class ZDvidTarget;
class ZNeuroglancerLayerSpec;
class ZNeuroglancerAnnotationLayerSpec;

class ZNeuroglancerLayerSpecFactory
{
public:
  ZNeuroglancerLayerSpecFactory();

  static std::shared_ptr<ZNeuroglancerLayerSpec> MakeGrayscaleLayer(
      const ZDvidTarget &target);
  static std::shared_ptr<ZNeuroglancerLayerSpec> MakeSegmentationLayer(
      const ZDvidTarget &target);
  static std::shared_ptr<ZNeuroglancerAnnotationLayerSpec>
  MakePointAnnotationLayer(const std::string &linkedSegmentationLayer);

};

#endif // ZNEUROGLANCERLAYERSPECFACTORY_H

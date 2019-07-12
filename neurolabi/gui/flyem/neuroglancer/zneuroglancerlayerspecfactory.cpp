#include "zneuroglancerlayerspecfactory.h"

#include "dvid/zdvidtarget.h"
#include "zneuroglancerlayerspec.h"
#include "zneuroglancerannotationlayerspec.h"

ZNeuroglancerLayerSpecFactory::ZNeuroglancerLayerSpecFactory()
{

}

std::shared_ptr<ZNeuroglancerLayerSpec>
ZNeuroglancerLayerSpecFactory::MakeGrayscaleLayer(const ZDvidTarget &target)
{
  std::shared_ptr<ZNeuroglancerLayerSpec> layer;
  if (target.hasGrayScaleData()) {
    layer = std::make_shared<ZNeuroglancerLayerSpec>();
    layer->setName("grayscale");
    layer->setType("image");
    layer->setSource(
          "dvid://http://" + target.getGrayScaleSource().getAddressWithPort() +
          "/" + target.getGrayScaleSource().getUuid() + "/" +
          target.getGrayScaleName());
  }

  return layer;
}

std::shared_ptr<ZNeuroglancerLayerSpec>
ZNeuroglancerLayerSpecFactory::MakeSegmentationLayer(const ZDvidTarget &target)
{
  std::shared_ptr<ZNeuroglancerLayerSpec> layer;
  if (target.hasSegmentation()) {
    layer = std::make_shared<ZNeuroglancerLayerSpec>();
    layer->setName("segmentation");
    layer->setType("segmentation");
    layer->setSource(
          "dvid://http://" + target.getAddressWithPort() +
          "/" + target.getUuid() + "/" + target.getSegmentationName());
  }

  return layer;
}

std::shared_ptr<ZNeuroglancerAnnotationLayerSpec>
ZNeuroglancerLayerSpecFactory::MakePointAnnotationLayer(const ZDvidTarget &target,
    const std::string &linkedSegmentationLayer)
{
  std::shared_ptr<ZNeuroglancerAnnotationLayerSpec> layer;

  layer = std::make_shared<ZNeuroglancerAnnotationLayerSpec>();
  layer->setName("annotation");
  layer->setType("annotation");
  layer->setLinkedSegmentation(linkedSegmentationLayer);
  layer->setTool("annotatePoint");
  layer->setSource(
        "dvid://http://" + target.getAddressWithPort() + "/" + target.getUuid() +
        "/" + target.getBookmarkName());

  return layer;
}

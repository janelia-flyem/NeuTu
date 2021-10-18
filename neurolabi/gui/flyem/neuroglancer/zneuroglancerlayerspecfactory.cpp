#include "zneuroglancerlayerspecfactory.h"

#include "neutubeconfig.h"
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
    layer->setType(ZNeuroglancerLayerSpec::TYPE_GRAYSCALE);
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
    layer->setType(ZNeuroglancerLayerSpec::TYPE_SEGMENTATION);
    layer->setSource(
          "dvid://http://" + target.getAddressWithPort() +
          "/" + target.getUuid() + "/" + target.getSegmentationName());
  }

  return layer;
}

std::shared_ptr<ZNeuroglancerLayerSpec>
ZNeuroglancerLayerSpecFactory::MakeSkeletonLayer(const ZDvidTarget &target)
{
  std::shared_ptr<ZNeuroglancerLayerSpec> layer;
  if (target.hasSegmentation()) {
    layer = std::make_shared<ZNeuroglancerLayerSpec>();
    layer->setName("skeletons");
    layer->setType(ZNeuroglancerLayerSpec::TYPE_SKELETON);
    layer->setSource(
          "dvid://http://" + target.getAddressWithPort() +
          "/" + target.getUuid() + "/" + target.getSkeletonName());
  }

  return layer;
}

std::shared_ptr<ZNeuroglancerAnnotationLayerSpec>
ZNeuroglancerLayerSpecFactory::MakeLocalAnnotationLayer(const std::string &name)
{
  std::shared_ptr<ZNeuroglancerAnnotationLayerSpec> layer =
      std::make_shared<ZNeuroglancerAnnotationLayerSpec>();
  layer->setName(name);
  layer->setColor(255, 255, 255);
  layer->setSource("local://annotations");

  return layer;
}

std::shared_ptr<ZNeuroglancerAnnotationLayerSpec>
ZNeuroglancerLayerSpecFactory::MakePointAnnotationLayer(
    const ZDvidTarget &target, ZDvidData::ERole dataRole,
    const std::string &linkedSegmentationLayer)
{
  std::shared_ptr<ZNeuroglancerAnnotationLayerSpec> layer;

  if (dataRole == ZDvidData::ERole::BOOKMARK ||
      dataRole == ZDvidData::ERole::SYNAPSE) {
    layer = std::make_shared<ZNeuroglancerAnnotationLayerSpec>();
//    layer->setType(ZNeuroglancerLayerSpec::TYPE_ANNOTATION);
    layer->setLinkedSegmentation(linkedSegmentationLayer);
    std::string dataName = target.getBookmarkName();
    std::string queryString;

    switch (dataRole) {
    case ZDvidData::ERole::BOOKMARK:
      layer->setName("annotation");
      layer->setTool("annotatePoint");
      layer->setColor(255, 0, 0);
      queryString = "?usertag=true&user=" + NeutubeConfig::GetUserName();
      break;
    case ZDvidData::ERole::SYNAPSE:
      dataName = target.getSynapseName();
      layer->setColor(255, 255, 0);
      layer->setName("synapse");
      break;
    default:
      break;
    }

    std::string source =
        "dvid://http://" + target.getAddressWithPort() + "/" + target.getUuid() +
        "/" + dataName + queryString;

    layer->setSource(source);
  }

  return layer;
}

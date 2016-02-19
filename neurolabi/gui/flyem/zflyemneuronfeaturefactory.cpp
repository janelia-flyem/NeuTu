#include "zflyemneuronfeaturefactory.h"
#include "zmapgenerator.h"

std::map<ZFlyEmNeuronFeature::EFeatureId, std::string>
ZFlyEmNeuronFeatureFactory::m_featureIdNameMap =
    ZMapGenerator<ZFlyEmNeuronFeature::EFeatureId, std::string>()
<< ZFlyEmNeuronFeatureFactory::makeFeature(
     ZFlyEmNeuronFeature::LEAF_NUMBER, "Number of leaves")
<< ZFlyEmNeuronFeatureFactory::makeFeature(
     ZFlyEmNeuronFeature::BRANCH_POINT_NUMBER, "number of branch points")
<< ZFlyEmNeuronFeatureFactory::makeFeature(
     ZFlyEmNeuronFeature::BOX_VOLUME, "box volume")
<< ZFlyEmNeuronFeatureFactory::makeFeature(
     ZFlyEmNeuronFeature::MAX_SEGMENT_LENGTH, "maximum segment length")
<< ZFlyEmNeuronFeatureFactory::makeFeature(
     ZFlyEmNeuronFeature::MAX_BRANCH_PATH_LENGTH, "maximum path length")
<< ZFlyEmNeuronFeatureFactory::makeFeature(
     ZFlyEmNeuronFeature::RADIUS_MEAN, "average radius")
<< ZFlyEmNeuronFeatureFactory::makeFeature(
     ZFlyEmNeuronFeature::RADIUS_VARIANCE, "radius variance")
<< ZFlyEmNeuronFeatureFactory::makeFeature(
     ZFlyEmNeuronFeature::LATERAL_VERTICAL_RATIO, "lateral/vertical ratio")
<< ZFlyEmNeuronFeatureFactory::makeFeature(
     ZFlyEmNeuronFeature::AVERAGE_CURVATURE, "Average curvature")
<< ZFlyEmNeuronFeatureFactory::makeFeature(
     ZFlyEmNeuronFeature::MOST_SPREAD_LAYER, "Most spread layer")
<< ZFlyEmNeuronFeatureFactory::makeFeature(
     ZFlyEmNeuronFeature::ARBOR_SPREAD, "Arbor spread")
<< ZFlyEmNeuronFeatureFactory::makeFeature(
    ZFlyEmNeuronFeature::OVERALL_LENGTH, "Overall length")
<< ZFlyEmNeuronFeatureFactory::makeFeature(
    ZFlyEmNeuronFeature::BRANCH_NUMBER, "Branch number")
<< ZFlyEmNeuronFeatureFactory::makeFeature(
     ZFlyEmNeuronFeature::TBAR_NUMBER, "TBar number")
<< ZFlyEmNeuronFeatureFactory::makeFeature(
     ZFlyEmNeuronFeature::PSD_NUMBER, "PSD number")
<< ZFlyEmNeuronFeatureFactory::makeFeature(
     ZFlyEmNeuronFeature::CENTROID_X, "Centroid X")
<< ZFlyEmNeuronFeatureFactory::makeFeature(
     ZFlyEmNeuronFeature::CENTROID_Y, "Centroid Y");

std::map<std::string, ZFlyEmNeuronFeature::EFeatureId>
ZFlyEmNeuronFeatureFactory::m_featureNameIdMap =
    ZMapGenerator<std::string, ZFlyEmNeuronFeature::EFeatureId>()
<< ZFlyEmNeuronFeatureFactory::makeFeature2(
     ZFlyEmNeuronFeature::LEAF_NUMBER, "Number of leaves")
<< ZFlyEmNeuronFeatureFactory::makeFeature2(
     ZFlyEmNeuronFeature::BRANCH_POINT_NUMBER, "number of branch points")
<< ZFlyEmNeuronFeatureFactory::makeFeature2(
     ZFlyEmNeuronFeature::BOX_VOLUME, "box volume")
<< ZFlyEmNeuronFeatureFactory::makeFeature2(
     ZFlyEmNeuronFeature::MAX_SEGMENT_LENGTH, "maximum segment length")
<< ZFlyEmNeuronFeatureFactory::makeFeature2(
     ZFlyEmNeuronFeature::MAX_BRANCH_PATH_LENGTH, "maximum path length")
<< ZFlyEmNeuronFeatureFactory::makeFeature2(
     ZFlyEmNeuronFeature::RADIUS_MEAN, "average radius")
<< ZFlyEmNeuronFeatureFactory::makeFeature2(
     ZFlyEmNeuronFeature::RADIUS_VARIANCE, "radius variance")
<< ZFlyEmNeuronFeatureFactory::makeFeature2(
     ZFlyEmNeuronFeature::LATERAL_VERTICAL_RATIO, "lateral/vertical ratio")
<< ZFlyEmNeuronFeatureFactory::makeFeature2(
     ZFlyEmNeuronFeature::AVERAGE_CURVATURE, "Average curvature")
<< ZFlyEmNeuronFeatureFactory::makeFeature2(
     ZFlyEmNeuronFeature::MOST_SPREAD_LAYER, "Most spread layer")
<< ZFlyEmNeuronFeatureFactory::makeFeature2(
     ZFlyEmNeuronFeature::ARBOR_SPREAD, "Arbor spread")
<< ZFlyEmNeuronFeatureFactory::makeFeature2(
    ZFlyEmNeuronFeature::OVERALL_LENGTH, "Overall length")
<< ZFlyEmNeuronFeatureFactory::makeFeature2(
    ZFlyEmNeuronFeature::BRANCH_NUMBER, "Branch number")
<< ZFlyEmNeuronFeatureFactory::makeFeature2(
     ZFlyEmNeuronFeature::TBAR_NUMBER, "TBar number")
<< ZFlyEmNeuronFeatureFactory::makeFeature2(
     ZFlyEmNeuronFeature::PSD_NUMBER, "PSD number")
<< ZFlyEmNeuronFeatureFactory::makeFeature2(
     ZFlyEmNeuronFeature::CENTROID_X, "Centroid X")
<< ZFlyEmNeuronFeatureFactory::makeFeature2(
     ZFlyEmNeuronFeature::CENTROID_Y, "Centroid Y");


std::pair<ZFlyEmNeuronFeature::EFeatureId, std::string>
ZFlyEmNeuronFeatureFactory::makeFeature(
      ZFlyEmNeuronFeature::EFeatureId key, const std::string &value)
{
  return std::pair<ZFlyEmNeuronFeature::EFeatureId, std::string>(key, value);
}

std::pair<std::string, ZFlyEmNeuronFeature::EFeatureId>
ZFlyEmNeuronFeatureFactory::makeFeature2(
      ZFlyEmNeuronFeature::EFeatureId key, const std::string &value)
{
  return std::pair<std::string, ZFlyEmNeuronFeature::EFeatureId>(value, key);
}

ZFlyEmNeuronFeatureFactory::ZFlyEmNeuronFeatureFactory()
{
}

ZFlyEmNeuronFeature ZFlyEmNeuronFeatureFactory::makeFeature(
    ZFlyEmNeuronFeature::EFeatureId id)
{
  return ZFlyEmNeuronFeature(id, m_featureIdNameMap[id]);
}

ZFlyEmNeuronFeature ZFlyEmNeuronFeatureFactory::makeFeature(
    const std::string &name)
{
  return ZFlyEmNeuronFeature(m_featureNameIdMap[name], name);
}

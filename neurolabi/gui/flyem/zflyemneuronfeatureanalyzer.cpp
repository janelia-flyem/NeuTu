#include "zflyemneuronfeatureanalyzer.h"
#include "zobject3dscan.h"
#include "zswcglobalfeatureanalyzer.h"
#include "zvectorgenerator.h"
#include "zflyemneuronfeaturefactory.h"

std::vector<std::string> ZFlyEmNeuronFeatureAnalyzer::m_featureName =
    ZVectorGenerator<std::string>() << "Number of leaves"
                                  << "number of branch points"
                                  << "box volume"
                                  << "maximum segment length"
                                  << "maximum path length"
                                  << "average radius"
                                  << "radius variance"
                                  << "lateral/vertical ratio"
                                  << "Average curvature"
                                  << "Most spread layer"
                                  << "Arbor spread";

ZFlyEmNeuronFeatureAnalyzer::ZFlyEmNeuronFeatureAnalyzer()
{
}

int ZFlyEmNeuronFeatureAnalyzer::computeMostSpreadedLayer(
    const ZFlyEmNeuron &neuron)
{
  ZObject3dScan *obj = neuron.getBody();
  int layer = -1;
  if (obj != NULL) {
    int minZ = obj->getMinZ();
    int maxZ = obj->getMaxZ();
    double maxSpread = 0.0;
    for (int z = minZ; z < maxZ; ++z) {
      double spread = obj->getSpread(z);
      if (spread > maxSpread) {
        maxSpread = spread;
        layer = z;
      }
    }
  }

  return layer;
}

double ZFlyEmNeuronFeatureAnalyzer::computeSpreadRadius(
    const ZFlyEmNeuron &neuron, int layer)
{
  ZObject3dScan *obj = neuron.getBody();
  if (obj != NULL) {
    return obj->getSpread(layer);
  }

  return 0.0;
}

void ZFlyEmNeuronFeatureAnalyzer::computeFeatureSet(
    const ZFlyEmNeuron &neuron, std::vector<ZFlyEmNeuronFeature> &featureSet)
{
  for (std::vector<ZFlyEmNeuronFeature>::iterator
       iter = featureSet.begin(); iter != featureSet.end(); ++iter) {
    computeFeature(neuron, *iter);
  }
}

std::vector<double> ZFlyEmNeuronFeatureAnalyzer::computeFeatureSet(
    const ZFlyEmNeuron &neuron)
{
  ZSwcTree *tree = neuron.getModel();

  std::vector<double> featureSet =
      ZSwcGlobalFeatureAnalyzer::computeFeatureSet(
        *tree, ZSwcGlobalFeatureAnalyzer::NGF2);

  int layer = computeMostSpreadedLayer(neuron);
  featureSet.push_back(layer);
  featureSet.push_back(computeSpreadRadius(neuron, layer));

  return featureSet;
}

bool ZFlyEmNeuronFeatureAnalyzer::computeFeature(
    const ZFlyEmNeuron &neuron, ZFlyEmNeuronFeature &feature)
{
  switch (feature.getId()) {
  case ZFlyEmNeuronFeature::OVERALL_LENGTH:
    feature.setValue(neuron.getModel()->length());
    break;
  case ZFlyEmNeuronFeature::BRANCH_NUMBER:
    feature.setValue(Swc_Tree_Branch_Number(neuron.getModel()->data()));
    break;
  case ZFlyEmNeuronFeature::TBAR_NUMBER:
    feature.setValue(neuron.getTBarNumber());
    break;
  case ZFlyEmNeuronFeature::PSD_NUMBER:
    feature.setValue(neuron.getPsdNumber());
    break;
  case ZFlyEmNeuronFeature::CENTROID_X:
  {
    double x, y, z;
    Swc_Tree_Centroid(neuron.getModel()->data(), &x, &y, &z);
    feature.setValue(x);
  }
    break;
  case ZFlyEmNeuronFeature::CENTROID_Y:
  {
    double x, y, z;
    Swc_Tree_Centroid(neuron.getModel()->data(), &x, &y, &z);
    feature.setValue(y);
  }
    break;
  default:
    feature.setValue(0);
    return false;
  }

  return true;
}

std::vector<ZFlyEmNeuronFeature>
ZFlyEmNeuronFeatureAnalyzer::computeFullFeatureSet(const ZFlyEmNeuron &neuron)
{
  std::vector<ZFlyEmNeuronFeature> featureSet;

  ZSwcTree *tree = neuron.getModel();

  std::vector<double> features =
      ZSwcGlobalFeatureAnalyzer::computeFeatureSet(
        *tree, ZSwcGlobalFeatureAnalyzer::NGF1);

  int layer = computeMostSpreadedLayer(neuron);
  features.push_back(layer);
  features.push_back(computeSpreadRadius(neuron, layer));

  featureSet.resize(features.size());
  size_t i = 0;
  for (; i < features.size(); ++i) {
    std::string featureName = ZSwcGlobalFeatureAnalyzer::getFeatureName(
          ZSwcGlobalFeatureAnalyzer::NGF1, 0);
    featureSet[i] = ZFlyEmNeuronFeatureFactory::makeFeature(featureName);
    featureSet[i].setValue(features[i]);
  }

  featureSet[i] = ZFlyEmNeuronFeatureFactory::makeFeature("Most spread layer");
  featureSet[i].setValue(features[i]);
  i++;
  featureSet[i] = ZFlyEmNeuronFeatureFactory::makeFeature("Arbor spread");
  featureSet[i].setValue(features[i]);

  return featureSet;
}

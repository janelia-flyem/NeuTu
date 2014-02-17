#include "zflyemneuronfeatureanalyzer.h"
#include "zobject3dscan.h"
#include "zswcglobalfeatureanalyzer.h"
#include "zvectorgenerator.h"

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

std::vector<double> ZFlyEmNeuronFeatureAnalyzer::computeFeatureSet(
    const ZFlyEmNeuron &neuron)
{
  ZSwcTree *tree = neuron.getModel();

  std::vector<double> featureSet =
      ZSwcGlobalFeatureAnalyzer::computeFeatureSet(
        *tree, ZSwcGlobalFeatureAnalyzer::NGF1);

  int layer = computeMostSpreadedLayer(neuron);
  featureSet.push_back(layer);
  featureSet.push_back(computeSpreadRadius(neuron, layer));

  return featureSet;
}

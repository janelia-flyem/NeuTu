#ifndef ZFLYEMNEURONFEATUREANALYZER_H
#define ZFLYEMNEURONFEATUREANALYZER_H

#include "zflyemneuron.h"
#include "zflyemneuronfeature.h"

class ZFlyEmNeuronFeatureAnalyzer
{
public:
  ZFlyEmNeuronFeatureAnalyzer();

public:
  static int computeMostSpreadedLayer(const ZFlyEmNeuron &neuron);
  static double computeSpreadRadius(const ZFlyEmNeuron &neuron, int layer);

  static std::vector<double> computeFeatureSet(const ZFlyEmNeuron &neuron);
  static inline const std::vector<std::string>& getFeatureName() {
    return m_featureName;
  }
  static inline int getFeatureNumber() { return m_featureName.size(); }

  static std::vector<ZFlyEmNeuronFeature> computeFullFeatureSet(
      const ZFlyEmNeuron &neuron);

  void computeFeatureSet(
      const ZFlyEmNeuron &neuron, std::vector<ZFlyEmNeuronFeature> &featureSet);

  bool computeFeature(const ZFlyEmNeuron &neuron, ZFlyEmNeuronFeature &feature);

private:
  static std::vector<std::string> m_featureName;
  static std::string m_emptyFeatureName;
};

#endif // ZFLYEMNEURONFEATUREANALYZER_H

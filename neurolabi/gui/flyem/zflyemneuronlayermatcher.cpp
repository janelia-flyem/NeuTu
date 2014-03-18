#include "zflyemneuronlayermatcher.h"
#include "tz_error.h"
#include "zdynamicprogrammer.h"
#include "zobject3dscan.h"
#include "zflyemneuron.h"
#include "tz_utilities.h"

ZFlyEmLayerFeatureSequence::ZFlyEmLayerFeatureSequence()
{

}

double ZFlyEmLayerFeatureSequence::getLayer(size_t index) const
{
  return (*this)[index].first;
}

double ZFlyEmLayerFeatureSequence::getValue(size_t index) const
{
  return (*this)[index].second;
}

void ZFlyEmLayerFeatureSequence::append(double layer, double value)
{
  push_back(std::pair<double, double>(layer, value));
}

ZFlyEmNeuronLayerMatcher::ZFlyEmNeuronLayerMatcher() : m_matchingScore(0.0),
  m_layerScale(100.0), m_layerBaseFactor(1.0)
{
}

double ZFlyEmNeuronLayerMatcher::match(
    ZFlyEmNeuron *neuron1, ZFlyEmNeuron *neuron2)
{
  m_matchingResult.clear();
  double score = 0.0;

  ZFlyEmLayerFeatureSequence layerSequence1 = computeLayerFeature(neuron1);
  ZFlyEmLayerFeatureSequence layerSequence2 = computeLayerFeature(neuron2);
  score = match(layerSequence1, layerSequence2);

  m_matchingScore = score;

  return score;
}

double ZFlyEmNeuronLayerMatcher::computeSimilarity(
    double layer1, double value1, double layer2, double value2) const
{
  double layerDiff = fabs(layer1 - layer2);

  double s2 = dmax2(value1, value2);
  double s1 = dmin2(value1, value2);

  TZ_ASSERT(s1 > 0.0, "Invalid number");

  return sqrt(s1) * s1 / s2 /
      (layerDiff / m_layerScale + m_layerBaseFactor);
}

double ZFlyEmNeuronLayerMatcher::match(const ZFlyEmLayerFeatureSequence &seq1,
                                       const ZFlyEmLayerFeatureSequence &seq2)
{
  ZMatrix simMat(seq1.getLayerNumber(), seq2.getLayerNumber());

  for (size_t i = 0; i < seq1.getLayerNumber(); ++i) {
    double layer1 = seq1.getLayer(i);
    double value1 = seq1.getValue(i);
    for (size_t j = 0; j < seq2.getLayerNumber(); ++j) {
      double layer2 = seq2.getLayer(j);
      double value2 = seq2.getValue(j);
      simMat.set(i, j, computeSimilarity(layer1, value1, layer2, value2));
    }
  }

#ifdef _DEBUG_2
  simMat.debugOutput();
#endif

  ZDynamicProgrammer dp;
  dp.setGapPenalty(0.1);
  dp.match(simMat);

  const ZDynamicProgrammer::MatchResult &matches = dp.getMatchingResult();

  m_matchingResult.clear();
  for (ZDynamicProgrammer::MatchResult::const_iterator iter = matches.begin();
       iter !=  matches.end(); ++iter) {
    addMatched(seq1.getLayer(iter->first), seq2.getLayer(iter->second));
//    m_matchingResult.push_back(
//          std::pair<double, double>(
//            seq1.getLayer(iter->first), seq2.getLayer(iter->second)));
  }

  return dp.getScore();
}

void ZFlyEmNeuronLayerMatcher::addMatched(double v1, double v2)
{
  m_matchingResult.push_back(std::pair<double, double>(v1, v2));
}

ZFlyEmLayerFeatureSequence ZFlyEmNeuronLayerMatcher::computeLayerFeature(
    ZFlyEmNeuron *neuron) const
{
  ZFlyEmLayerFeatureSequence feature;
  ZObject3dScan *body = neuron->getBody();
  if (body != NULL) {
    TZ_ASSERT(body->isCanonized(), "Uncanonized body is not accpetable");
    int minZ = body->getMinZ();
    int maxZ = body->getMaxZ();
    for (int z = minZ; z <= maxZ; ++z) {
      size_t value = body->getVoxelNumber(z);
      if (value > 0) {
        feature.append(z, value);
      }
    }
  }

  return feature;
}

void ZFlyEmNeuronLayerMatcher::print() const
{
  std::cout << "ZFlyEmNeuronLayerMatcher:" << std::endl;
  std::cout << "  Layer scale: " << m_layerScale << std::endl;
  std::cout << "  Base factor: " << m_layerBaseFactor << std::endl;
  if (!m_matchingResult.empty()) {
    std::cout << "  Matching score: " << m_matchingScore << std::endl;
  }
}

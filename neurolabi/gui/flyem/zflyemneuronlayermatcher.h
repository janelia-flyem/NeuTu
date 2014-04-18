#ifndef ZFLYEMNEURONLAYERMATCHER_H
#define ZFLYEMNEURONLAYERMATCHER_H

#include <map>
#include <vector>
#include <utility>
#include "zmatrix.h"

class ZFlyEmNeuron;

class ZFlyEmLayerFeatureSequence :
    public std::vector<std::pair<double, double> > {
public:
  ZFlyEmLayerFeatureSequence();

  double getLayer(size_t index) const;
  double getValue(size_t index) const;
  void append(double layer, double value);
  inline size_t getLayerNumber() const { return size(); }

private:

};

class ZFlyEmNeuronLayerMatcher
{
public:
  ZFlyEmNeuronLayerMatcher();

  double match(ZFlyEmNeuron *neuron1, ZFlyEmNeuron *neuron2);
  inline void setLayerScale(double scale) { m_layerScale = scale; }

  void print() const;

private:
  double match(const ZFlyEmLayerFeatureSequence &seq1,
               const ZFlyEmLayerFeatureSequence &seq2);
  double computeSimilarity(double layer1, double value1,
                           double layer2, double value2) const;
  ZFlyEmLayerFeatureSequence computeLayerFeature(ZFlyEmNeuron *neuron) const;

  void addMatched(double v1, double v2);

private:
  double m_matchingScore;
  std::vector<std::pair<double, double> > m_matchingResult;
  double m_layerScale;
  double m_layerBaseFactor;
};

#endif // ZFLYEMNEURONLAYERMATCHER_H

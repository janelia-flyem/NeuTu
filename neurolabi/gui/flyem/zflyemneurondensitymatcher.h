#ifndef ZFLYEMNEURONDENSITYMATCHER_H
#define ZFLYEMNEURONDENSITYMATCHER_H

class ZFlyEmNeuronDensity;
class ZFlyEmNeuronDensityUnit;

class ZFlyemNeuronDensityMatcher
{
public:
  ZFlyemNeuronDensityMatcher();

  double match(const ZFlyEmNeuronDensity &density1,
               const ZFlyEmNeuronDensity &density2);
  double computeSimilarity(const ZFlyEmNeuronDensityUnit &density1,
                           const ZFlyEmNeuronDensityUnit &density2) const;

  double computeSimilarity(double z1, double v1, double z2, double v2) const;

private:
  double m_gapPenalty;
  double m_layerBaseFactor;
  double m_layerScale;
  double m_layerMargin;
};

#endif // ZFLYEMNEURONDENSITYMATCHER_H

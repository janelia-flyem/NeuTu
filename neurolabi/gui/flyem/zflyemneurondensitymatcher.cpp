#include "zflyemneurondensitymatcher.h"
#include <cmath>

#include "zdynamicprogrammer.h"
#include "zflyemneurondensity.h"
#include "zhistogram.h"
#include "zmatrix.h"
#include "tz_math.h"
#include "tz_utilities.h"

ZFlyemNeuronDensityMatcher::ZFlyemNeuronDensityMatcher() : m_gapPenalty(0.1),
  m_layerBaseFactor(1.0), m_layerScale(100.0), m_layerMargin(100.0)
{
}

double ZFlyemNeuronDensityMatcher::match(
    const ZFlyEmNeuronDensity &density1, const ZFlyEmNeuronDensity &density2)
{
  ZHistogram hist1 = density1.getHistogram(m_layerMargin);
  ZHistogram hist2 = density2.getHistogram(m_layerMargin);

  ZMatrix simmat;

  double dz = m_layerMargin;
  double startZ1 = iround(hist1.getBinStart(0) / m_layerMargin) * m_layerMargin;
  int count1 = hist1.getBinNumber();

  double startZ2 = iround(hist2.getBinStart(0) / m_layerMargin) * m_layerMargin;
  int count2 = hist2.getBinNumber();

  simmat.resize(count1, count2);
  for (int j = 0; j < count2; ++j) {
    double z2 = j * dz + startZ2;
    double v2 = hist2.getCount(z2);
    for (int i = 0; i < count1; ++i) {
      double z1 = i * dz + startZ1;
      double v1 = hist1.getCount(z1);
      simmat.set(i, j, computeSimilarity(z1, v1, z2, v2));
    }
  }

  ZDynamicProgrammer programmer;
  programmer.setGapPenalty(m_gapPenalty);
  programmer.match(simmat);

  return programmer.getScore();
}

double ZFlyemNeuronDensityMatcher::computeSimilarity(
    double z1, double v1, double z2, double v2) const
{
  if (v1 == 0.0 || v2 == 0.0) {
    return 0.0;
  }

//  v1 = sqrt(v1);
//  v2 = sqrt(v2);

  double layerDiff = fabs(z1 - z2);

  double s2 = dmax2(v1, v2);
  double s1 = dmin2(v1, v2);

//  TZ_ASSERT(s1 > 0.0, "Invalid number");

  return sqrt(s1) * s1 / s2 /
      (layerDiff / m_layerScale + m_layerBaseFactor);
}

double ZFlyemNeuronDensityMatcher::computeSimilarity(
    const ZFlyEmNeuronDensityUnit &density1,
    const ZFlyEmNeuronDensityUnit &density2) const
{
  return computeSimilarity(density1.getZ(), density1.getDensity(),
                           density2.getZ(), density2.getDensity());
}

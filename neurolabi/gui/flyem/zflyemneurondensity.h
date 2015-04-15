#ifndef ZFLYEMNEURONDENSITY_H
#define ZFLYEMNEURONDENSITY_H

#include <vector>
#include <string>

class ZHistogram;

class ZFlyEmNeuronDensityUnit {
public:
  ZFlyEmNeuronDensityUnit(double z = 0, double v = 0);
  inline double getZ() const { return m_z; }
  inline double getDensity() const { return m_density; }

private:
  double m_z;
  double m_density;
};

class ZFlyEmNeuronDensity
{
public:
  ZFlyEmNeuronDensity();

  ZHistogram getHistogram(double binSize) const;
  void append(double z, double v);
  void importTxtFile(const std::string &filePath);

private:
  std::vector<ZFlyEmNeuronDensityUnit> m_data;
};

#endif // ZFLYEMNEURONDENSITY_H

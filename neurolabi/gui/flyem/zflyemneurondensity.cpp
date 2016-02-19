#include "zflyemneurondensity.h"
#include "zhistogram.h"
#include "zmatrix.h"
#include "zstring.h"

ZFlyEmNeuronDensityUnit::ZFlyEmNeuronDensityUnit(double z, double v) :
  m_z(z), m_density(v)
{

}

ZFlyEmNeuronDensity::ZFlyEmNeuronDensity()
{
}

ZHistogram ZFlyEmNeuronDensity::getHistogram(double binSize) const
{
  ZHistogram hist;
  hist.setInterval(binSize);

  for (std::vector<ZFlyEmNeuronDensityUnit>::const_iterator
       iter = m_data.begin(); iter != m_data.end(); ++iter) {
    const ZFlyEmNeuronDensityUnit& unit = *iter;
    hist.addCount(unit.getZ(), unit.getDensity());
  }

  return hist;
}

void ZFlyEmNeuronDensity::append(double z, double v)
{
  m_data.push_back(ZFlyEmNeuronDensityUnit(z, v));
}

void ZFlyEmNeuronDensity::importTxtFile(const std::string &filePath)
{
  ZString str;
  FILE *fp = fopen(filePath.c_str(), "r");

  while(str.readLine(fp)) {
    std::vector<double> valueArray = str.toDoubleArray();
    if (valueArray.size() >= 2) {
      append(valueArray[0], valueArray[1]);
    }
  }

  fclose(fp);
}

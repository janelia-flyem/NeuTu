#ifndef ZFLYEMNEURONEXPORTER_H
#define ZFLYEMNEURONEXPORTER_H

#include "zflyemneuron.h"
#include "zflyemneuronarray.h"

class ZFlyEmNeuronFilter;

class ZFlyEmNeuronExporter
{
public:
  ZFlyEmNeuronExporter();

  void exportIdPosition(const ZFlyEmNeuronArray &neuronArray,
                        const std::string &filePath);

  void exportIdVolume(const ZFlyEmNeuronArray &neuronArray,
                      const std::string &filePath);

  inline void setNeuronFilter(ZFlyEmNeuronFilter *filter) {
    m_neuronFilter = filter;
  }

  inline void setRavelerHeight(int height) {
    m_ravelerHeight = height;
  }

  inline void useRavelerCoordinate(bool enabled) {
    m_usingRavelerCoordinate = enabled;
  }

private:
  int m_ravelerHeight;
  bool m_usingRavelerCoordinate;
  ZFlyEmNeuronFilter *m_neuronFilter;
};

#endif // ZFLYEMNEURONEXPORTER_H

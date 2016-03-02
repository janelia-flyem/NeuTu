#ifndef ZSTACKOBJECTSOURCEFACTORY_H
#define ZSTACKOBJECTSOURCEFACTORY_H

#include <string>
#include "tz_stdint.h"
#include "neutube_def.h"

class ZStackObjectSourceFactory
{
public:
  ZStackObjectSourceFactory();

public:
  static std::string MakeWatershedBoundarySource(int label);
  static std::string MakeRectRoiSource(const std::string &suffix = "");
  static std::string MakeFlyEmBodyMaskSource(uint64_t bodyId);
  static std::string MakeFlyEmBodySource(uint64_t bodyId);
  static uint64_t ExtractIdFromFlyEmBodySource(const std::string &source);
  static std::string MakeCurrentMsTileSource(int resLevel);
  static std::string MakeDvidTileSource();
  static std::string MakeDvidLabelSliceSource(NeuTube::EAxis axis);
  static std::string MakeDvidGraySliceSource();
  static std::string MakeSplitObjectSource();
  static std::string MakeNodeAdaptorSource();
  static std::string MakeFlyEmBoundBoxSource();
  static std::string MakeFlyEmRoiSource(const std::string &roiName);
  static std::string MakeFlyEmPlaneObjectSource();
  static std::string MakeFlyEmSynapseSource();
  static std::string MakeFlyEmTBarSource();
  static std::string MakeFlyEmTBarSource(uint64_t bodyId);

  static std::string MakeFlyEmPsdSource();
  static std::string MakeFlyEmPsdSource(uint64_t bodyId);

  static std::string MakeFlyEmSplitRoiSource();
  static std::string MakeFlyEmExtNeuronClass();
  static std::string MakeStackBoundBoxSource();
  static std::string MakeDvidSynapseEnsembleSource();
  static std::string MakeDvidSynapseEnsembleSource(NeuTube::EAxis axis);
  static std::string MakeTodoListEnsembleSource();
  static std::string MakeTodoListEnsembleSource(NeuTube::EAxis axis);
};

#endif // ZSTACKOBJECTSOURCEFACTORY_H

#ifndef ZSTACKOBJECTSOURCEFACTORY_H
#define ZSTACKOBJECTSOURCEFACTORY_H

#include <string>
#include "tz_stdint.h"
#include "common/neutube_def.h"

class ZStackObjectSourceFactory
{
public:
  ZStackObjectSourceFactory();

public:
  static std::string MakeWatershedBoundarySource(int label);
  static std::string MakeRectRoiSource(const std::string &suffix = "");
  static std::string MakeFlyEmBodyMaskSource(uint64_t bodyId);
//  static std::string MakeFlyEmCoarseBodySource(uint64_t bodyId);
  static std::string MakeFlyEmBodySource(uint64_t bodyId, int zoom = 0);
  static std::string MakeFlyEmBodySource(
      uint64_t bodyId, int zoom, const std::string &tag);
  static std::string MakeFlyEmBodySource(
      uint64_t bodyId, int zoom, flyem::EBodyType bodyType);
  /*
  static std::string MakeFlyEmBodySource(
      uint64_t bodyId, int zoom, flyem::EBodyType bodyType, bool coarse);
      */
  static std::string GetBodyTypeName(flyem::EBodyType bodyType);
  static std::string MakeFlyEmBodyDiffSource();
  static bool IsBodyDiffSource(const std::string &source);
  static std::string MakeFlyEmBodyDiffSource(
      uint64_t bodyId, const std::string &tag);

  static std::string ExtractBodyStrFromFlyEmBodySource(const std::string &source);
  static uint64_t ExtractIdFromFlyEmBodySource(const std::string &source);
  static int ExtractZoomFromFlyEmBodySource(const std::string &source);
  static flyem::EBodyType ExtractBodyTypeFromFlyEmBodySource(
      const std::string &source);
//  static bool IsCoarseBodySource(const std::string &source);

  static std::string MakeCurrentMsTileSource(int resLevel);
  static std::string MakeDvidTileSource();
  static std::string MakeDvidLabelSliceSource(neutu::EAxis axis);
  static std::string MakeDvidGraySliceSource(neutu::EAxis axis);
  static std::string MakeSplitObjectSource();
  static std::string MakeSplitResultSource();
  static std::string MakeSplitResultSource(int label);
  static std::string MakeNodeAdaptorSource();
  static std::string MakeFlyEmBoundBoxSource();
  static std::string MakeFlyEmRoiSource(const std::string &roiName);
  static std::string MakeFlyEmPlaneObjectSource();
  static std::string MakeSlicViewObjectSource();
  static std::string MakeFlyEmSynapseSource();
  static std::string MakeFlyEmTBarSource();
  static std::string MakeFlyEmTBarSource(uint64_t bodyId);
  static std::string MakeFlyEmSeedSource(uint64_t bodyId);

  static std::string MakeFlyEmPsdSource();
  static std::string MakeFlyEmPsdSource(uint64_t bodyId);

  static std::string MakeFlyEmSplitRoiSource();
  static std::string MakeFlyEmExtNeuronClass();
  static std::string MakeStackBoundBoxSource();
  static std::string MakeDvidSynapseEnsembleSource();
  static std::string MakeDvidSynapseEnsembleSource(neutu::EAxis axis);
  static std::string MakeTodoListEnsembleSource();
  static std::string MakeTodoListEnsembleSource(neutu::EAxis axis);
  static std::string MakeTodoPunctaSource();
  static std::string MakeTodoPunctaSource(uint64_t bodyId);
  static std::string MakeCrossHairSource();
  static std::string MakeProtocolRangeSource();
};

#endif // ZSTACKOBJECTSOURCEFACTORY_H

#ifndef ZSTACKOBJECTSOURCEFACTORY_H
#define ZSTACKOBJECTSOURCEFACTORY_H

#include <string>
#include "tz_stdint.h"

class ZStackObjectSourceFactory
{
public:
  ZStackObjectSourceFactory();

public:
  static std::string MakeWatershedBoundarySource(int label);
  static std::string MakeRectRoiSource(const std::string &suffix = "");
  static std::string MakeFlyEmBodyMaskSource(uint64_t bodyId);
  static std::string MakeFlyEmBodySource(uint64_t bodyId);
  static std::string MakeCurrentMsTileSource(int resLevel);
  static std::string MakeDvidTileSource();
  static std::string MakeDvidLabelSliceSource();
  static std::string MakeDvidGraySliceSource();
  static std::string MakeSplitObjectSource();
};

#endif // ZSTACKOBJECTSOURCEFACTORY_H

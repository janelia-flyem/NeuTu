#ifndef ZSTACKOBJECTSOURCEFACTORY_H
#define ZSTACKOBJECTSOURCEFACTORY_H

#include <string>

class ZStackObjectSourceFactory
{
public:
  ZStackObjectSourceFactory();

public:
  static std::string MakeWatershedBoundarySource(int label);
  static std::string MakeRectRoiSource(const std::string &suffix = "");
  static std::string MakeFlyEmBodyMaskSource(int bodyId);
  static std::string MakeFlyEmBodySource(int bodyId);
  static std::string MakeCurrentMsTileSource(int resLevel);
};

#endif // ZSTACKOBJECTSOURCEFACTORY_H

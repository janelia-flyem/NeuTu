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
};

#endif // ZSTACKOBJECTSOURCEFACTORY_H

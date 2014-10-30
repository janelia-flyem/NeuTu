#include "zstackobjectsourcefactory.h"
#include "zstring.h"
#include "zstackobjectsource.h"

ZStackObjectSourceFactory::ZStackObjectSourceFactory()
{
}

std::string ZStackObjectSourceFactory::MakeWatershedBoundarySource(int label)
{
  ZString objectSource =ZStackObjectSource::getSource(
        ZStackObjectSource::ID_LOCAL_WATERSHED_BORDER);
  objectSource.appendNumber(label);

  return objectSource;
}

std::string ZStackObjectSourceFactory::MakeRectRoiSource(
    const std::string &suffix)
{
  std::string source = ZStackObjectSource::getSource(
        ZStackObjectSource::ID_RECT_ROI);
  if (!suffix.empty()) {
    source += ":" + suffix;
  }

  return source;
}

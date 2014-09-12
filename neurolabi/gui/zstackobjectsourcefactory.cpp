#include "zstackobjectsourcefactory.h"
#include "zstring.h"

ZStackObjectSourceFactory::ZStackObjectSourceFactory()
{
}

std::string ZStackObjectSourceFactory::MakeWatershedBoundarySource(int label)
{
  ZString objectSource = "#localSeededWatershed:Temporary_Border:";
  objectSource.appendNumber(label);

  return objectSource;
}

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

std::string ZStackObjectSourceFactory::MakeFlyEmBodyMaskSource(uint64_t bodyId)
{
  ZString source = "#.FlyEmBodyMaskSource#";
  source.appendNumber(bodyId);

  return source;
}

std::string ZStackObjectSourceFactory::MakeFlyEmBodySource(uint64_t bodyId)
{
  ZString source = "#.FlyEmBody#";
  source.appendNumber(bodyId);

  return source;
}

std::string ZStackObjectSourceFactory::MakeCurrentMsTileSource(int resLevel)
{
  ZString source = "#.MutliResTile#";
  source.appendNumber(resLevel);

  return source;
}

std::string ZStackObjectSourceFactory::MakeDvidTileSource()
{
  return "#.DVIDTileEnsemble";
}

std::string ZStackObjectSourceFactory::MakeDvidLabelSliceSource()
{
  return "#.DVIDLabelSlice";
}

std::string ZStackObjectSourceFactory::MakeDvidGraySliceSource()
{
  return "#.DVIDGraySlice";
}

std::string ZStackObjectSourceFactory::MakeSplitObjectSource()
{
  return "#.FlyEMSplitObject";
}

std::string ZStackObjectSourceFactory::MakeNodeAdaptorSource()
{
  return "#.NodeAdapter";
}

std::string ZStackObjectSourceFactory::MakeFlyEmBoundBoxSource()
{
  return "#.FlyEMBoundBox";
}

std::string ZStackObjectSourceFactory::MakeFlyEmPlaneObjectSource()
{
  return "#.FlyEMPlaneObject";
}

std::string ZStackObjectSourceFactory::MakeFlyEmSynapseSource()
{
  return "#.FlyEMSynapse";
}

std::string ZStackObjectSourceFactory::MakeFlyEmTBarSource()
{
  return "#.FlyEMSynapse.TBar";
}

std::string ZStackObjectSourceFactory::MakeFlyEmPsdSource()
{
  return "#.FlyEMSynapse.Psd";
}

std::string ZStackObjectSourceFactory::MakeFlyEmSplitRoiSource()
{
  return "#.FlyEmSplitRoi";
}

std::string ZStackObjectSourceFactory::MakeFlyEmExtNeuronClass()
{
  return "#.FlyEmExternalNeuron";
}


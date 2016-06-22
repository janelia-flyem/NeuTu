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

uint64_t ZStackObjectSourceFactory::ExtractIdFromFlyEmBodySource(
    const std::string &source)
{
  uint64_t id = 0;

  ZString sourceBase = "#.FlyEmBody#";
  if (source.length() > sourceBase.length()) {
    ZString substr = source.substr(sourceBase.length() - 1);
    std::vector<uint64_t> idArray = substr.toUint64Array();
    if (idArray.size() == 1) {
      id = idArray.front();
    }
  }

  return id;
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

std::string ZStackObjectSourceFactory::MakeDvidLabelSliceSource(
    NeuTube::EAxis axis)
{
  std::string source = "#.DVIDLabelSlice";
  switch (axis) {
  case NeuTube::X_AXIS:
    source += ".X";
    break;
  case NeuTube::Y_AXIS:
    source += ".Y";
    break;
  default:
    break;
  }

  return source;
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

std::string ZStackObjectSourceFactory::MakeFlyEmRoiSource(const std::string &roiName)
{
  return "#.FlyemRoi#" + roiName;
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

std::string ZStackObjectSourceFactory::MakeFlyEmTBarSource(uint64_t bodyId)
{
  ZString source = MakeFlyEmTBarSource() + "#";
  source.appendNumber(bodyId);

  return source;
}


std::string ZStackObjectSourceFactory::MakeFlyEmPsdSource()
{
  return "#.FlyEMSynapse.Psd";
}

std::string ZStackObjectSourceFactory::MakeFlyEmPsdSource(uint64_t bodyId)
{
  ZString source = MakeFlyEmPsdSource() + "#";
  source.appendNumber(bodyId);

  return source;
}

std::string ZStackObjectSourceFactory::MakeFlyEmSplitRoiSource()
{
  return "#.FlyEmSplitRoi";
}

std::string ZStackObjectSourceFactory::MakeFlyEmExtNeuronClass()
{
  return "#.FlyEmExternalNeuron";
}

std::string ZStackObjectSourceFactory::MakeStackBoundBoxSource()
{
  return "#.StackBoundBox";
}

std::string ZStackObjectSourceFactory::MakeDvidSynapseEnsembleSource()
{
  return "#.DVIDSynapseEnsemble";
}

std::string
ZStackObjectSourceFactory::MakeDvidSynapseEnsembleSource(NeuTube::EAxis axis)
{
  std::string source = MakeDvidSynapseEnsembleSource();
  switch (axis) {
  case NeuTube::X_AXIS:
    source += ".X";
    break;
  case NeuTube::Y_AXIS:
    source += ".Y";
    break;
  default:
    break;
  }

  return source;
}

std::string
ZStackObjectSourceFactory::MakeTodoListEnsembleSource()
{
  return "#.DVIDFlyEMTodoList";
}

std::string
ZStackObjectSourceFactory::MakeTodoListEnsembleSource(NeuTube::EAxis axis)
{
  std::string source = MakeTodoListEnsembleSource();
  switch (axis) {
  case NeuTube::X_AXIS:
    source += ".X";
    break;
  case NeuTube::Y_AXIS:
    source += ".Y";
    break;
  default:
    break;
  }

  return source;
}

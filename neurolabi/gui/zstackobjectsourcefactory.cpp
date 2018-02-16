#include "zstackobjectsourcefactory.h"

#include <iostream>
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

std::string ZStackObjectSourceFactory::MakeFlyEmBodyDiffSource()
{
  return "#.FlyEm.Body.Diff#";
}

bool ZStackObjectSourceFactory::IsBodyDiffSource(const std::string &source)
{
  return ZString(source).startsWith(MakeFlyEmBodyDiffSource());
}

std::string ZStackObjectSourceFactory::MakeFlyEmBodyDiffSource(
    uint64_t bodyId, const std::string &tag)
{
  ZString source = MakeFlyEmBodyDiffSource();
  source.appendNumber(bodyId);
  source += "." + tag;

  return source;
}

std::string ZStackObjectSourceFactory::GetBodyTypeName(
    flyem::EBodyType bodyType)
{
  switch (bodyType) {
  case flyem::BODY_NULL:
    break;
  case flyem::BODY_FULL:
    return "full";
  case flyem::BODY_COARSE:
    return "coarse";
  case flyem::BODY_SKELETON:
    return "skeleton";
  case flyem::BODY_MESH:
    return "mesh";
    break;
  }

  return "";
}

std::string ZStackObjectSourceFactory::MakeFlyEmCoarseBodySource(uint64_t bodyId)
{
  return MakeFlyEmBodySource(bodyId, 0, flyem::BODY_COARSE);
}

std::string ZStackObjectSourceFactory::MakeFlyEmBodySource(
    uint64_t bodyId, int zoom)
{
  ZString source = "#.FlyEmBody#";
  source.appendNumber(bodyId);
  if (zoom > 0) {
    source += "_";
    source.appendNumber(zoom);
  }

  return source;
}

std::string ZStackObjectSourceFactory::MakeFlyEmBodySource(
    uint64_t bodyId, int zoom, const std::string &tag)
{
  return MakeFlyEmBodySource(bodyId, zoom) + "#." + tag;
}

std::string ZStackObjectSourceFactory::MakeFlyEmBodySource(
    uint64_t bodyId, int zoom, flyem::EBodyType bodyType)
{
  return MakeFlyEmBodySource(bodyId, zoom, GetBodyTypeName(bodyType));
}

std::string ZStackObjectSourceFactory::ExtractBodyStrFromFlyEmBodySource(
    const std::string &source)
{
  ZString substr;
  ZString sourceBase = "#.FlyEmBody";
  if (source.length() > sourceBase.length() + 1) {
    ZString::size_type startPos = source.find('#', sourceBase.length()) + 1;
    ZString::size_type tokenPos = source.find('#', startPos);
    if (tokenPos == ZString::npos) {
      substr = source.substr(startPos);
    } else {
      substr = source.substr(startPos, tokenPos - startPos);
    }
  }

  return substr;
}

uint64_t ZStackObjectSourceFactory::ExtractIdFromFlyEmBodySource(
    const std::string &source)
{
  uint64_t id = 0;

  ZString substr = ExtractBodyStrFromFlyEmBodySource(source);
  std::vector<uint64_t> idArray = substr.toUint64Array();
  if (idArray.size() >= 1) {
    id = idArray.front();
  }

  return id;
}

flyem::EBodyType ZStackObjectSourceFactory::ExtractBodyTypeFromFlyEmBodySource(
      const std::string &source)
{
  ZString str(source);
  flyem::EBodyType bodyType = flyem::BODY_NULL;
  if (str.endsWith("#.full")) {
    bodyType = flyem::BODY_FULL;
  } else if (str.endsWith("#.coarse")) {
    bodyType = flyem::BODY_COARSE;
  } else if (str.endsWith("#.skeleton")) {
    bodyType = flyem::BODY_SKELETON;
  }

  return bodyType;
}

int ZStackObjectSourceFactory::ExtractZoomFromFlyEmBodySource(
    const std::string &source)
{
  int zoom = 0;

  ZString substr = ExtractBodyStrFromFlyEmBodySource(source);
  std::vector<uint64_t> idArray = substr.toUint64Array();
  if (idArray.size() >= 2) {
    zoom = idArray[1];
  }

  return zoom;
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
    neutube::EAxis axis)
{
  std::string source = "#.DVIDLabelSlice";
  switch (axis) {
  case neutube::X_AXIS:
    source += ".X";
    break;
  case neutube::Y_AXIS:
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
  std::string source = "#.FlyEMSplitObject";
  /*
  if (type == flyem::LABEL_SUPERVOXEL) {
    source += ".Supervoxel";
  }
  */

  return source;
}

std::string ZStackObjectSourceFactory::MakeSplitResultSource()
{
  return "#.FlyEMSplitResult";
}

std::string ZStackObjectSourceFactory::MakeSplitResultSource(int label)
{
  ZString source = MakeSplitResultSource();
  source.appendNumber(label);

  return source;
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

std::string ZStackObjectSourceFactory::MakeFlyEmSeedSource(uint64_t bodyId)
{
  ZString source = "#.FlyEMSeed#";
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
ZStackObjectSourceFactory::MakeDvidSynapseEnsembleSource(neutube::EAxis axis)
{
  std::string source = MakeDvidSynapseEnsembleSource();
  switch (axis) {
  case neutube::X_AXIS:
    source += ".X";
    break;
  case neutube::Y_AXIS:
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

std::string ZStackObjectSourceFactory::MakeCrossHairSource()
{
  return "#.CrossHair";
}

std::string ZStackObjectSourceFactory::MakeTodoPunctaSource()
{
  return "#.DVIDFlyEMTodoPuncta";
}

std::string ZStackObjectSourceFactory::MakeTodoPunctaSource(uint64_t bodyId)
{
  ZString source = MakeTodoPunctaSource() + "#";
  source.appendNumber(bodyId);

  return source;
}

std::string
ZStackObjectSourceFactory::MakeTodoListEnsembleSource(neutube::EAxis axis)
{
  std::string source = MakeTodoListEnsembleSource();
  switch (axis) {
  case neutube::X_AXIS:
    source += ".X";
    break;
  case neutube::Y_AXIS:
    source += ".Y";
    break;
  default:
    break;
  }

  return source;
}

#include "zdvidglobal.h"

#include "qt/network/znetworkutils.h"

#include "zjsonparser.h"

#include "zdvidurl.h"
#include "zdvidinfo.h"
#include "zdvidutil.h"
#include "zdvidversiondag.h"

ZDvidGlobal::ZDvidGlobal()
{

}

ZJsonObject ZDvidGlobal::Memo::ReadDataInfoJson(
        const ZDvidTarget &target, const std::string &dataName)
{
  std::string url = ZDvidUrl(target).getInfoUrl(dataName);

  return ZNetworkUtils::ReadJsonObjectMemo(url);
}

ZDvidInfo ZDvidGlobal::Memo::ReadDataInfo(
    const ZDvidTarget &target, const std::string &dataName)
{
  ZJsonObject obj = ReadDataInfoJson(target, dataName);

  ZDvidInfo dvidInfo;

  if (!obj.isEmpty()) {
    dvidInfo.set(obj);
    dvidInfo.setDvidNode(
          target.getRootUrl(), target.getPort(), target.getUuid());
  }

  return dvidInfo;
}

dvid::ENodeStatus ZDvidGlobal::Memo::ReadNodeStatus(const ZDvidTarget &target)
{
  dvid::ENodeStatus status = dvid::ENodeStatus::NORMAL;

  ZDvidUrl url(target);
  std::string repoUrl = url.getRepoUrl() + "/info";
  if (repoUrl.empty()) {
    status = dvid::ENodeStatus::INVALID;
  } else {
    int statusCode = 0;
    dvid::MakeHeadRequest(repoUrl, statusCode);
    if (statusCode != 200) {
      status = dvid::ENodeStatus::OFFLINE;
    } else {
      ZJsonObject obj =
          ZNetworkUtils::ReadJsonObjectMemo(url.getCommitInfoUrl());
      if (obj.hasKey("Locked")) {
        bool locked = ZJsonParser::booleanValue(obj["Locked"]);
        if (locked) {
          status = dvid::ENodeStatus::LOCKED;
        }
      }
    }
  }

  return status;
}

ZJsonObject ZDvidGlobal::Memo::ReadInfo(const ZDvidTarget &target)
{
  ZDvidUrl url(target);

  return ZNetworkUtils::ReadJsonObjectMemo(url.getInfoUrl());
}

ZDvidInfo ZDvidGlobal::Memo::ReadGrayscaleInfo(const ZDvidTarget &target)
{
  return ReadDataInfo(target, target.getGrayScaleName());
}

ZDvidInfo ZDvidGlobal::Memo::ReadSegmentationInfo(const ZDvidTarget &target)
{
  return ReadDataInfo(target, target.getSegmentationName());
}

ZJsonObject ZDvidGlobal::Memo::ReadRepoInfo(const ZDvidTarget &target)
{
  ZDvidUrl dvidUrl(target);

  return ZNetworkUtils::ReadJsonObjectMemo(dvidUrl.getRepoInfoUrl());
}

ZDvidVersionDag ZDvidGlobal::Memo::ReadVersionDag(const ZDvidTarget &target)
{
  ZDvidVersionDag dag;

  if (target.isValid()) {
    ZJsonObject repoInfo = ReadRepoInfo(target);
    dag.load(repoInfo);
#ifdef _DEBUG_2
    dag.print();
#endif
  }

  return dag;
}

int ZDvidGlobal::Memo::ReadMaxZoomFromDataName(
        const ZDvidTarget &target, std::function<std::string(int)> nameGetter)
{
  int zoom = 0;

  ZJsonObject infoJson = ReadInfo(target);
  ZDvidVersionDag dag = ReadVersionDag(target);
  int level = 1;
  while (level < 50) {
    if (dvid::IsDataValid(nameGetter(level), target, infoJson, dag)) {
      zoom = level;
    } else {
      break;
    }
    ++level;
  }

  return zoom;
}

int ZDvidGlobal::Memo::ReadMaxGrayscaleZoom(const ZDvidTarget &target)
{
  int zoom = 0;
  if (target.hasGrayScaleData()) {
    zoom = ReadMaxZoomFromDataName(target, [&](int level) {
      return target.getGrayScaleName(level);
    });
  }

  return zoom;
}

int ZDvidGlobal::Memo::ReadMaxLabelZoom(const ZDvidTarget &target)
{
  int zoom = 0;
  if (target.hasMultiscaleSegmentation()) {
    ZJsonObject infoJson = ReadDataInfoJson(target, target.getSegmentationName());
    ZJsonValue v = infoJson.value({"Extended", "MaxDownresLevel"});
    if (!v.isEmpty()) {
      zoom = v.toInteger();
    }
  } else if (target.hasSegmentation()) {
    zoom = ReadMaxZoomFromDataName(target, [&](int level) {
      return target.getSegmentationName(level);
    });
  }

  return zoom;
}

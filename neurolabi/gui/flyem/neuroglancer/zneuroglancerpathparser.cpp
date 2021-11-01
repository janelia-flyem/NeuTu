#include "zneuroglancerpathparser.h"

#include <QUrl>
#include <QDebug>

#include "common/utilities.h"

#include "zjsonobject.h"
#include "zjsonobjectparser.h"

#include "zneuroglancerpath.h"
#include "zneuroglancerlayerspec.h"
#include "dvid/zdvidenv.h"
#include "dvid/zdvidtarget.h"
#include "zstring.h"

ZNeuroglancerPathParser::ZNeuroglancerPathParser()
{

}


ZJsonObject ZNeuroglancerPathParser::ParseDataSpec(const QString &url)
{
  QUrl urlObj(url);
  QString dataString = urlObj.fragment(QUrl::FullyDecoded);

  ZJsonObject obj;

  if (dataString.startsWith(ZNeuroglancerPath::DATA_START_TAG)) {
    dataString = dataString.mid(strlen(ZNeuroglancerPath::DATA_START_TAG));
    obj.decode(dataString.toStdString(), true);
  }

  return obj;
}

namespace {
ZDvidTarget make_dvid_target_from_source(
    const ZString &source, std::string &dataName)
{
  ZDvidTarget target;
  if (source.startsWith("dvid://")) {
    QUrl url(source.substr(neutu::Length("dvid://")).c_str());
    target.setHost(url.host().toStdString());
    target.setPort(url.port());
    QStringList pathSegs = url.path().split("/", QString::SkipEmptyParts);
    qDebug() << pathSegs;
    if (pathSegs.size() > 0) {
      target.setUuid(pathSegs[0].toStdString());
      if (pathSegs.size() > 1) {
        dataName = pathSegs[1].toStdString();
//        target.setGrayScaleName(pathSegs[1].toStdString());
      }
    }
  }

  return target;
}
}

ZDvidEnv ZNeuroglancerPathParser::MakeDvidEnvFromUrl(const QString &url)
{
  ZDvidEnv env;

  size_t segLayerIndex = 0;
  ZJsonObject obj = ParseDataSpec(url);
  ZJsonObjectParser parser;
  if (obj.hasKey("layers")) {
    ZJsonArray layerArray(obj.value("layers"));
    for (size_t i = 0; i < layerArray.size(); ++i) {
      ZJsonObject layerObj(layerArray.value(i));
      std::string type = parser.GetValue(
            layerObj, ZNeuroglancerLayerSpec::KEY_TYPE, "");
      ZString source = parser.GetValue(
            layerObj, ZNeuroglancerLayerSpec::KEY_SOURCE, "");
      std::string dataName;
      ZDvidTarget target = make_dvid_target_from_source(source, dataName);


      if (type == ZNeuroglancerLayerSpec::TYPE_SEGMENTATION) {
        if (segLayerIndex == 0) {
          if (target.isValid() && !dataName.empty()) {
            target.setSegmentationName(dataName);
            env.setMainTarget(target);
          }
        }
        ++segLayerIndex;
      } else if (type == ZNeuroglancerLayerSpec::TYPE_GRAYSCALE) {
        if (target.isValid() && !dataName.empty()) {
          target.setGrayScaleName(dataName);
          env.appendValidDvidTarget(target, ZDvidEnv::ERole::GRAYSCALE);
        }
      }
    }
  }

  return env;
}

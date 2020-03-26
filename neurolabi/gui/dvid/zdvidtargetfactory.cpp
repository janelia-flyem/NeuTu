#include "zdvidtargetfactory.h"

#include <QUrl>
#include <QUrlQuery>

#include "zdvidtarget.h"

ZDvidTargetFactory::ZDvidTargetFactory()
{

}


ZDvidTarget ZDvidTargetFactory::MakeFromSourceString(const std::string &sourceString)
{
  ZDvidTarget target;
  target.setFromSourceString(sourceString);

  return target;
}

ZDvidTarget ZDvidTargetFactory::MakeFromSourceString(const char *sourceString)
{
  return  MakeFromSourceString(std::string(sourceString));
}

ZDvidTarget ZDvidTargetFactory::MakeFromSourceString(const QString &sourceString)
{
  return MakeFromSourceString(sourceString.toStdString());
}

ZDvidTarget ZDvidTargetFactory::MakeFromUrlSpec(const QString &urlSpec)
{
  ZDvidTarget target;

  QUrl url(urlSpec);
  QUrlQuery query(url);


  target.setScheme(url.scheme().toStdString());
  target.set(url.host().toStdString(),
             query.queryItemValue("uuid").toStdString(),
             url.port());

  std::string segmentation =
      query.queryItemValue("segmentation").toStdString();
  if (!segmentation.empty()) {
    target.setSegmentationType(ZDvidData::EType::LABELMAP);
    target.setSegmentationName(segmentation);
  }
  target.setGrayScaleName(query.queryItemValue("grayscale").toStdString());
  target.setAdminToken(query.queryItemValue("admintoken").toStdString());

  return target;
}

ZDvidTarget ZDvidTargetFactory::MakeFromUrlSpec(const std::string &urlSpec)
{
  return MakeFromUrlSpec(QString::fromStdString(urlSpec));
}

ZDvidTarget ZDvidTargetFactory::MakeFromUrlSpec(const char* urlSpec)
{
  return MakeFromUrlSpec(QString(urlSpec));
}

ZDvidTarget ZDvidTargetFactory::MakeFromSpec(const QString &spec)
{
  if (spec.startsWith("http:") && !spec.startsWith("http://")) {
    return MakeFromSourceString(spec);
  } else if (spec.startsWith("deprecated:")) {
    ZDvidTarget target;
    target.setFromUrl_deprecated(spec.right(spec.length() - 11).toStdString());
    return target;
  }

  return MakeFromUrlSpec(spec);
}

ZDvidTarget ZDvidTargetFactory::MakeFromSpec(const std::string &spec)
{
  return MakeFromSpec(QString::fromStdString(spec));
}

ZDvidTarget ZDvidTargetFactory::MakeFromSpec(const char *spec)
{
  return MakeFromSpec(QString(spec));
}

ZDvidTarget ZDvidTargetFactory::Make(const ZJsonObject &obj)
{
  ZDvidTarget target;
  target.loadJsonObject(obj);

  return target;
}

ZDvidTarget ZDvidTargetFactory::MakeFromJsonSpec(const std::string &spec)
{
  ZJsonObject obj;
  obj.decode(spec);

  return Make(obj);
}


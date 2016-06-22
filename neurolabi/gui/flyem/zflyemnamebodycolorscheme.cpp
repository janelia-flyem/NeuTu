#include "zflyemnamebodycolorscheme.h"

#include "zflyembodyannotation.h"
#include "zstring.h"

ZFlyEmNameBodyColorScheme::ZFlyEmNameBodyColorScheme()
{
  m_colorMap["KC-s"] = QColor(255, 0, 0);
  m_colorMap["KC-c"] = QColor(0, 255, 0);
  m_colorMap["KC-p"] = QColor(0, 0, 255);
  m_colorMap["MBON"] = QColor(255, 0, 255);
  m_colorMap["PPL1"] = QColor(0, 255, 255);
  m_colorMap["KC-alpha prime"] = QColor(255, 255, 0);
  m_colorMap["KC-any"] = QColor(255, 140, 0);
  m_colorMap["MB-APL"] = QColor(140, 255, 0);
  m_colorMap["MB-DPM"] = QColor(140, 255, 0);
  m_isMapReady = false;
}

ZFlyEmNameBodyColorScheme::~ZFlyEmNameBodyColorScheme()
{
  m_colorMap.clear();
  m_nameMap.clear();
}

QColor ZFlyEmNameBodyColorScheme::getColor(const ZFlyEmBodyAnnotation &annotation)
{
  QColor color(0, 0, 0, 0);

  if (m_colorMap.contains(annotation.getName().c_str())) {
    color = m_colorMap[annotation.getName().c_str()];
  }

  return color;
}

QColor ZFlyEmNameBodyColorScheme::getBodyColor(uint64_t bodyId)
{
  QColor color(0, 0, 0, 0);

//  ZFlyEmBodyAnnotation annotation = m_reader.readBodyAnnotation(bodyId);

//  color = getColor(annotation);
  const QString &name = m_nameMap[bodyId];
  if (m_colorMap.contains(name)) {
    color = m_colorMap[m_nameMap[bodyId]];
  }

  return color;
}


void ZFlyEmNameBodyColorScheme::setDvidTarget(const ZDvidTarget &target)
{
  m_reader.open(target);
}

void ZFlyEmNameBodyColorScheme::prepareNameMap(const ZJsonValue &bodyInfoObj)
{
  if (!m_isMapReady) {
    ZJsonArray bookmarks(bodyInfoObj);
    for (size_t i = 0; i < bookmarks.size(); ++i) {
      ZJsonObject bkmk(bookmarks.at(i), false);
      if (bkmk.hasKey("name")) {
        const char* name = ZJsonParser::stringValue(bkmk["name"]);
        uint64_t bodyId = ZJsonParser::integerValue(bkmk["body ID"]);
        updateNameMap(bodyId, name);
      }
    }

    m_isMapReady = true;
  }
}

void ZFlyEmNameBodyColorScheme::prepareNameMap()
{
  if (!m_isMapReady) {
    if (m_reader.getDvidTarget().isValid()) {
      QStringList annotationList = m_reader.readKeys(
            ZDvidData::GetName(ZDvidData::ROLE_BODY_ANNOTATION,
                               ZDvidData::ROLE_BODY_LABEL,
                               m_reader.getDvidTarget().getBodyLabelName()).c_str());
      foreach (const QString &idStr, annotationList) {
        uint64_t bodyId = ZString(idStr.toStdString()).firstInteger();
        ZFlyEmBodyAnnotation annotation = m_reader.readBodyAnnotation(bodyId);
        updateNameMap(bodyId, annotation.getName().c_str());
      }
    }

    m_isMapReady = true;
  }
}

void ZFlyEmNameBodyColorScheme::updateNameMap(const ZFlyEmBodyAnnotation &annotation)
{
  updateNameMap(annotation.getBodyId(), annotation.getName().c_str());
}

void ZFlyEmNameBodyColorScheme::updateNameMap(uint64_t bodyId, const QString &name)
{
  if (!name.isEmpty()) {
    QString finalName = name;

    if (name.startsWith("MBON")) {
      finalName = "MBON";
    } else if (name.startsWith("PPL1")) {
      finalName = "PPL1";
    }

    m_nameMap[bodyId] = finalName;
  }
}

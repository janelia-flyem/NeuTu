#include "zflyembodycolorscheme.h"

#include "zflyembodyannotation.h"
#include "zstring.h"

ZFlyEmBodyColorScheme::ZFlyEmBodyColorScheme()
{
  m_colorMap["KC-s"] = QColor(255, 0, 0);
  m_colorMap["KC-c"] = QColor(0, 255, 0);
  m_colorMap["KC-p"] = QColor(0, 0, 255);
  m_colorMap["MBON"] = QColor(255, 0, 255);
  m_colorMap["PPL1"] = QColor(0, 255, 255);
}

QColor ZFlyEmBodyColorScheme::getColor(const ZFlyEmBodyAnnotation &annotation)
{
  QColor color(0, 0, 0, 0);

  if (m_colorMap.contains(annotation.getName().c_str())) {
    color = m_colorMap[annotation.getName().c_str()];
  }

  return color;
}

QColor ZFlyEmBodyColorScheme::getBodyColor(uint64_t bodyId)
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


void ZFlyEmBodyColorScheme::setDvidTarget(const ZDvidTarget &target)
{
  m_reader.open(target);
}

void ZFlyEmBodyColorScheme::prepareNameMap()
{
  if (m_reader.getDvidTarget().isValid()) {
    QStringList annotationList = m_reader.readKeys(
          ZDvidData::GetName(ZDvidData::ROLE_BODY_ANNOTATION,
                             ZDvidData::ROLE_BODY_LABEL,
                             m_reader.getDvidTarget().getBodyLabelName()).c_str());
    foreach (const QString &idStr, annotationList) {
      uint64_t bodyId = ZString(idStr.toStdString()).firstInteger();
      ZFlyEmBodyAnnotation annotation = m_reader.readBodyAnnotation(bodyId);
      std::string name = annotation.getName();
      if (!name.empty()) {
        m_nameMap[bodyId] = name.c_str();
      }
    }
  }
}

void ZFlyEmBodyColorScheme::updateNameMap(const ZFlyEmBodyAnnotation &annotation)
{
  updateNameMap(annotation.getBodyId(), annotation.getName().c_str());
}

void ZFlyEmBodyColorScheme::updateNameMap(uint64_t bodyId, const QString &name)
{
  m_nameMap[bodyId] = name;
}

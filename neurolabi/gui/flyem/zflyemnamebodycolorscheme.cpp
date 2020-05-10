#include "zflyemnamebodycolorscheme.h"

#include "zflyembodyannotation.h"
#include "zstring.h"
#include "zjsonparser.h"
#include "flyemdatareader.h"

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

  m_defaultColor = QColor(0, 0, 0, 0);

  buildColorTable();
}

ZFlyEmNameBodyColorScheme::~ZFlyEmNameBodyColorScheme()
{
  m_colorMap.clear();
  m_nameMap.clear();
}

void ZFlyEmNameBodyColorScheme::buildColorTable()
{
  m_colorTable.resize(m_colorMap.size() + 1);
  m_colorTable[0] = m_defaultColor;

  int index = 1;
  for (QHash<QString, QColor>::const_iterator iter = m_colorMap.begin();
       iter != m_colorMap.end(); ++iter, ++index) {
    m_colorTable[index] = iter.value();
    m_nameIndexMap[iter.key()] = index;
  }

//  m_indexMap = getColorIndexMap();
}

int ZFlyEmNameBodyColorScheme::getBodyColorIndex(uint64_t bodyId) const
{
  return m_indexMap[bodyId];
}

QColor ZFlyEmNameBodyColorScheme::getBodyColorFromIndex(int index) const
{
  return m_colorTable[index];
}

QHash<uint64_t, int> ZFlyEmNameBodyColorScheme::getColorIndexMap() const
{
  return m_indexMap;
  /*
  QHash<QString, int> nameIndexMap;

  QHash<uint64_t, int> indexMap;
  int index = 1;
  for (QHash<QString, QColor>::const_iterator iter = m_colorMap.begin();
       iter != m_colorMap.end(); ++iter) {
    nameIndexMap[iter.key()] = index++;
  }

  for (QHash<uint64_t, QString>::const_iterator iter = m_nameMap.begin();
       iter != m_nameMap.end(); ++iter) {
    indexMap[iter.key()] = nameIndexMap[iter.value()];
  }

  return indexMap;
  */
}

QColor ZFlyEmNameBodyColorScheme::getColor(const ZFlyEmBodyAnnotation &annotation)
{
  QColor color(0, 0, 0, 0);

  if (m_colorMap.contains(annotation.getName().c_str())) {
    color = m_colorMap[annotation.getName().c_str()];
  }

  return color;
}

/*
QColor ZFlyEmNameBodyColorScheme::getBodyColor(uint64_t bodyId) const
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
*/

void ZFlyEmNameBodyColorScheme::setDvidTarget(const ZDvidTarget &target)
{
  m_reader.open(target);
}

void ZFlyEmNameBodyColorScheme::prepareNameMap(const ZJsonValue &bodyInfoObj)
{
  if (!m_isMapReady) {
    ZJsonArray bookmarks(bodyInfoObj);
    for (size_t i = 0; i < bookmarks.size(); ++i) {
      ZJsonObject bkmk(bookmarks.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
      if (bkmk.hasKey("name")) {
        std::string name = ZJsonParser::stringValue(bkmk["name"]);
        uint64_t bodyId = ZJsonParser::integerValue(bkmk["body ID"]);
        updateNameMap(bodyId, name.c_str());
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
            ZDvidData::GetName(ZDvidData::ERole::BODY_ANNOTATION,
                               ZDvidData::ERole::SPARSEVOL,
                               m_reader.getDvidTarget().getBodyLabelName()).c_str());
      foreach (const QString &idStr, annotationList) {
        uint64_t bodyId = ZString(idStr.toStdString()).firstInteger();
        ZFlyEmBodyAnnotation annotation =
            FlyEmDataReader::ReadBodyAnnotation(m_reader, bodyId);
        updateNameMap(bodyId, annotation.getName().c_str());
      }
    }

    m_isMapReady = true;
  }
}

void ZFlyEmNameBodyColorScheme::update()
{
  prepareNameMap();
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

    if (m_colorMap.contains(finalName)) {
      m_nameMap[bodyId] = finalName;
      m_indexMap[bodyId] = m_nameIndexMap[finalName];
    }
  }
}

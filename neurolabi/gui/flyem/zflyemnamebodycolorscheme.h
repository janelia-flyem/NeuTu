#ifndef ZFLYEMNAMEBODYCOLORSCHEME_H
#define ZFLYEMNAMEBODYCOLORSCHEME_H

#include <QHash>

#include "tz_stdint.h"
#include "zobjectcolorscheme.h"
#include "dvid/zdvidreader.h"
#include "zflyembodycolorscheme.h"

class ZFlyEmBodyAnnotation;

class ZFlyEmNameBodyColorScheme : public ZFlyEmBodyColorScheme
{
public:
  ZFlyEmNameBodyColorScheme();
  ~ZFlyEmNameBodyColorScheme();

  QColor getColor(const ZFlyEmBodyAnnotation &annotation);
  QColor getBodyColor(uint64_t bodyId);

  void importColorMap(const QString &filePath);

  void setDvidTarget(const ZDvidTarget &target);
  void prepareNameMap();
  void prepareNameMap(const ZJsonValue &bodyInfoObj);
  void updateNameMap(const ZFlyEmBodyAnnotation &annotation);
  void updateNameMap(uint64_t bodyId, const QString &name);

private:
  QHash<QString, QColor> m_colorMap;
  QHash<uint64_t, QString> m_nameMap;
  ZDvidReader m_reader;
  bool m_isMapReady;

};

#endif // ZFLYEMNAMEBODYCOLORSCHEME_H

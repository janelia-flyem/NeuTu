#ifndef ZFLYEMNAMEBODYCOLORSCHEME_H
#define ZFLYEMNAMEBODYCOLORSCHEME_H

#include <cstdint>
#include <QHash>

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
//  QColor getBodyColor(uint64_t bodyId) const override;

  void importColorMap(const QString &filePath);

  void setDvidTarget(const ZDvidTarget &target);
  void prepareNameMap();
  void prepareNameMap(const ZJsonValue &bodyInfoObj);
//  void updateNameMap(const ZFlyEmBodyAnnotation &annotation);
  void updateNameMap(uint64_t bodyId, const QString &name);

  QHash<uint64_t, int> getColorIndexMap() const override;
  int getBodyColorIndex(uint64_t bodyId) const override;
//  QColor getBodyColorFromIndex(int index) const override;
//  QVector<QColor> getColorList() const;
  void buildColorTable();
  void update() override;

  bool hasExplicitColor(uint64_t bodyId) const override;

private:
  QHash<QString, QColor> m_colorMap;
  QHash<uint64_t, QString> m_nameMap;
  QHash<QString, int> m_nameIndexMap;
  QHash<uint64_t, int> m_indexMap;
  ZDvidReader m_reader;
  bool m_isMapReady;
//  int m_defaultColor;
};

#endif // ZFLYEMNAMEBODYCOLORSCHEME_H

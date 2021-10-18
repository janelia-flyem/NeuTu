#ifndef ZFLYEMSEQUENCERCOLORSCHEME_H
#define ZFLYEMSEQUENCERCOLORSCHEME_H

#include <cstdint>
#include <memory>

#include <QColor>
#include <QHash>

#include "zflyembodycolorscheme.h"

class ZFlyEmSequencerColorScheme : public ZFlyEmBodyColorScheme
{
public:
  ZFlyEmSequencerColorScheme();
  //    QColor getBodyColor(uint64_t bodyId) const override;
  uint32_t getBodyColorCode(uint64_t bodyId) const override;
  QColor getBodyColorFromIndex(int index) const override;
  int getBodyColorIndex(uint64_t bodyId) const override;
  int getColorNumber() const override;
  void setDefaultColor(QColor color);
  void setBodyColor(uint64_t bodyId, QColor color);
  void clear();
  void print() const;

  QHash<uint64_t, int> getColorIndexMap() const override;

  void setDefaultColorScheme(std::shared_ptr<ZFlyEmBodyColorScheme> scheme);
  void setMainColorScheme(const ZFlyEmSequencerColorScheme &scheme);

  virtual bool hasExplicitColor(uint64_t bodyId) const override;

private:
  QHash<uint64_t, QColor> m_colorMap;
  QHash<uint64_t, int> m_indexMap;
  std::shared_ptr<ZFlyEmBodyColorScheme> m_defaultColorScheme;
  //    QColor m_defaultColor;
};




#endif // ZFLYEMSEQUENCERCOLORSCHEME_H

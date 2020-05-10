#ifndef ZFLYEMBODYIDCOLORSCHEME_H
#define ZFLYEMBODYIDCOLORSCHEME_H

#include <memory>

#include "zflyembodycolorscheme.h"

class ZFlyEmBodyIdColorScheme : public ZFlyEmBodyColorScheme
{
public:
  // The colorMap maps from each body ID to its color.
  ZFlyEmBodyIdColorScheme(const QHash<uint64_t, QColor> &colorMap);

  ~ZFlyEmBodyIdColorScheme() override;

  void setColorScheme(EColorScheme scheme) override;
  QHash<uint64_t, int> getColorIndexMap() const override;
  QColor getBodyColorFromIndex(int index) const override;
//  QColor getBodyColor(uint64_t bodyId) const override;
  int getBodyColorIndex(uint64_t bodyId) const override;
  int getColorNumber() const override;

  void setDefaultColorScheme(std::shared_ptr<ZFlyEmBodyColorScheme> scheme);

private:
//  void updateColorIndex();

private:
  QHash<uint64_t, QColor> m_colorMap;
  QHash<uint64_t, int> m_indexMap;
  std::shared_ptr<ZFlyEmBodyColorScheme> m_defaultColorScheme;
};

#endif // ZFLYEMBODYIDCOLORSCHEME_H

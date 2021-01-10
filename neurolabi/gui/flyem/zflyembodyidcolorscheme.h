#ifndef ZFLYEMBODYIDCOLORSCHEME_H
#define ZFLYEMBODYIDCOLORSCHEME_H

#include <memory>

#include "zflyembodycolorscheme.h"

class ZFlyEmBodyIdColorScheme : public ZFlyEmBodyColorScheme
{
public:
  ZFlyEmBodyIdColorScheme() {}
  // The colorMap maps from each body ID to its color.
  ZFlyEmBodyIdColorScheme(const QHash<uint64_t, QColor> &colorMap);

  ~ZFlyEmBodyIdColorScheme() override;

  void setColorScheme(EColorScheme scheme) override;
  QHash<uint64_t, int> getColorIndexMap() const override;
  QColor getBodyColorFromIndex(int index) const override;
//  QColor getBodyColor(uint64_t bodyId) const override;
  int getBodyColorIndex(uint64_t bodyId) const override;
  int getColorNumber() const override;
  uint32_t getBodyColorCode(uint64_t bodyId) const override;

//  void setDefaultColorScheme(std::shared_ptr<ZFlyEmBodyColorScheme> scheme);

  bool hasExplicitColor(uint64_t bodyId) const override;
//  bool hasOwnColor(uint64_t bodyId) const;

  bool setColor(uint64_t bodyId, uint32_t color);
  bool setColor(uint64_t bodyId, const QColor &color);
  bool removeBody(uint64_t bodyId);

  bool isEmpty() const;

  void print() const;

  static int COLOR_CAPACITY;

private:
//  void updateColorIndex();

private:
//  QHash<uint64_t, QColor> m_colorMap;
  QHash<uint64_t, int> m_bodyToIndex;
  QHash<uint32_t, int> m_colorToIndex;
//  std::shared_ptr<ZFlyEmBodyColorScheme> m_defaultColorScheme;
};

#endif // ZFLYEMBODYIDCOLORSCHEME_H

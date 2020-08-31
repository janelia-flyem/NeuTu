#ifndef ZFLYEMCOMPOSITEBODYCOLORSCHEME_H
#define ZFLYEMCOMPOSITEBODYCOLORSCHEME_H

#include <vector>
#include <memory>

#include "zflyembodycolorscheme.h"

class ZFlyEmCompositeBodyColorScheme : public ZFlyEmBodyColorScheme
{
public:
  ZFlyEmCompositeBodyColorScheme();

  void appendScheme(std::shared_ptr<ZFlyEmBodyColorScheme> scheme);

  bool hasExplicitColor(uint64_t bodyId) const override;
//  QColor getBodyColor(uint64_t bodyId) const override;
  uint32_t getBodyColorCode(uint64_t bodyId) const override;
  int getBodyColorIndex(uint64_t bodyId) const override;
  QColor getBodyColorFromIndex(int index) const override;
  void update() override;

  int getColorNumber() const override;

  bool isEmpty() const;

private:
  std::vector<std::shared_ptr<ZFlyEmBodyColorScheme>> m_schemeList;
};

#endif // ZFLYEMCOMPOSITEBODYCOLORSCHEME_H

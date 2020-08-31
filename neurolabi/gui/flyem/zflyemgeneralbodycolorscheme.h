#ifndef ZFLYEMGENERALBODYCOLORSCHEME_H
#define ZFLYEMGENERALBODYCOLORSCHEME_H

#include <functional>

#include "zflyembodycolorscheme.h"

class ZFlyEmGeneralBodyColorScheme : public ZFlyEmBodyColorScheme
{
public:
  ZFlyEmGeneralBodyColorScheme();

  int getBodyColorIndex(uint64_t bodyId) const override;
  bool hasExplicitColor(uint64_t bodyId) const override;
  uint32_t getBodyColorCode(uint64_t bodyId) const override;
  int getColorNumber() const override;

public:
  std::function<int() noexcept> _getColorNumber;
  std::function<int(uint64_t) noexcept> _getBodyColorIndex;
  std::function<uint32_t(uint64_t) noexcept> _getBodyColorCode;
  std::function<bool(uint64_t) noexcept> _hasExplicitColor =
      [](uint64_t) {return false;};
};

#endif // ZFLYEMGENERALBODYCOLORSCHEME_H

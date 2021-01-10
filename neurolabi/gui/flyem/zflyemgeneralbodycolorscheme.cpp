#include "zflyemgeneralbodycolorscheme.h"

ZFlyEmGeneralBodyColorScheme::ZFlyEmGeneralBodyColorScheme()
{
}

int ZFlyEmGeneralBodyColorScheme::getBodyColorIndex(uint64_t bodyId) const
{
  if (_getBodyColorIndex) {
    return _getBodyColorIndex(bodyId);
  }

  if (hasExplicitColor(bodyId)) {
    return 0;
  }

  return -1;
}

int ZFlyEmGeneralBodyColorScheme::getColorNumber() const
{
  if (_getColorNumber) {
    return _getColorNumber();
  }

  return 0;
}

bool ZFlyEmGeneralBodyColorScheme::hasExplicitColor(uint64_t bodyId) const
{
  if (_hasExplicitColor) {
    return _hasExplicitColor(bodyId);
  }

  return false;
}

uint32_t ZFlyEmGeneralBodyColorScheme::getBodyColorCode(uint64_t bodyId) const
{
  if (hasExplicitColor(bodyId)) {
    return _getBodyColorCode(bodyId);
  }

  return m_defaultColor;
}

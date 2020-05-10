#include "zflyembodycolorscheme.h"

ZFlyEmBodyColorScheme::ZFlyEmBodyColorScheme()
{
}

ZFlyEmBodyColorScheme::~ZFlyEmBodyColorScheme()
{

}

QHash<uint64_t, int> ZFlyEmBodyColorScheme::getColorIndexMap() const
{
  return QHash<uint64_t, int>();
}

QColor ZFlyEmBodyColorScheme::getBodyColor(uint64_t bodyId)
{
  return getBodyColorFromIndex(getBodyColorIndex(bodyId));
}

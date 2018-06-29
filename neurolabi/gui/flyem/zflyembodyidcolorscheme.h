#ifndef ZFLYEMBODYIDCOLORSCHEME_H
#define ZFLYEMBODYIDCOLORSCHEME_H

#include "zflyembodycolorscheme.h"

class ZFlyEmBodyIdColorScheme : public ZFlyEmBodyColorScheme
{
public:
  // The colorMap maps from each body ID to its color.
  ZFlyEmBodyIdColorScheme(const QHash<uint64_t, QColor> &colorMap);

 virtual ~ZFlyEmBodyIdColorScheme();

  virtual void setColorScheme(EColorScheme scheme) override;
  virtual QHash<uint64_t, int> getColorIndexMap() const override;
  virtual QColor getBodyColor(uint64_t bodyId) override;

private:
  QHash<uint64_t, QColor> m_colorMap;
  QHash<uint64_t, int> m_indexMap;
};

#endif // ZFLYEMBODYIDCOLORSCHEME_H

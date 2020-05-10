#ifndef ZFLYEMRANDOMBODYCOLORSCHEME_H
#define ZFLYEMRANDOMBODYCOLORSCHEME_H

#include "zflyembodycolorscheme.h"

class ZFlyEmRandomBodyColorScheme : public ZFlyEmBodyColorScheme
{
public:
  ZFlyEmRandomBodyColorScheme();
  virtual ~ZFlyEmRandomBodyColorScheme() override;

  QColor getBodyColorFromIndex(int index) const override;
  int getBodyColorIndex(uint64_t bodyId) const override;

  void update() override;
};

#endif // ZFLYEMRANDOMBODYCOLORSCHEME_H

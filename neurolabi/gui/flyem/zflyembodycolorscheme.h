#ifndef ZFLYEMBODYCOLORSCHEME_H
#define ZFLYEMBODYCOLORSCHEME_H

#include "zobjectcolorscheme.h"
#include "tz_stdint.h"

class ZFlyEmBodyColorScheme : public ZObjectColorScheme
{
public:
  ZFlyEmBodyColorScheme();
  virtual ~ZFlyEmBodyColorScheme();

  virtual QColor getBodyColor(uint64_t bodyId) = 0;
};

#endif // ZFLYEMBODYCOLORSCHEME_H

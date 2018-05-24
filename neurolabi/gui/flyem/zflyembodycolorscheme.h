#ifndef ZFLYEMBODYCOLORSCHEME_H
#define ZFLYEMBODYCOLORSCHEME_H

#include <QHash>
#include "zobjectcolorscheme.h"
#include "tz_stdint.h"

class ZFlyEmBodyColorScheme : public ZObjectColorScheme
{
public:
  ZFlyEmBodyColorScheme();
  virtual ~ZFlyEmBodyColorScheme();

  virtual QColor getBodyColor(uint64_t bodyId) = 0;
  virtual void update() {}


  virtual QHash<uint64_t, int> getColorIndexMap() const;
};

#endif // ZFLYEMBODYCOLORSCHEME_H

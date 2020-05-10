#ifndef ZFLYEMBODYCOLORSCHEME_H
#define ZFLYEMBODYCOLORSCHEME_H

#include <cstdint>

#include <QHash>
#include "zobjectcolorscheme.h"

/*!
 * \brief The class for managing body colors
 *
 * It assumes that body ID 0 is background and always maps to index 0, which
 * is the index of the background color.
 */
class ZFlyEmBodyColorScheme : public ZObjectColorScheme
{
public:
  ZFlyEmBodyColorScheme();
  virtual ~ZFlyEmBodyColorScheme();

  virtual QColor getBodyColor(uint64_t bodyId);
  virtual int getBodyColorIndex(uint64_t bodyId) const = 0;
  virtual QColor getBodyColorFromIndex(int index) const = 0;
  virtual void update() {}

  virtual QHash<uint64_t, int> getColorIndexMap() const;
};

#endif // ZFLYEMBODYCOLORSCHEME_H

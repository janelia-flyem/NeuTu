#ifndef ZFLYEMBODYCOLORSCHEME_H
#define ZFLYEMBODYCOLORSCHEME_H

#include <cstdint>

#include <QHash>
#include <QVector>
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

  virtual int getBodyColorIndex(uint64_t bodyId) const = 0;
  virtual bool hasExplicitColor(uint64_t bodyId) const = 0;

  virtual QColor getBodyColor(uint64_t bodyId) const;
  virtual uint32_t getBodyColorCode(uint64_t bodyId) const;
  virtual QColor getBodyColorFromIndex(int index) const;
  virtual int getBodyColorCodeFromIndex(int index) const;
  virtual void update() {}

  virtual QHash<uint64_t, int> getColorIndexMap() const;
  QVector<uint32_t> buildRgbTable() const;

  void mapColor(const uint64_t *src, uint32_t *dst, size_t v) const;

  void setDefaultColor(const QColor &color);
  void setDefaultColor(uint32_t code);

protected:
  uint32_t m_defaultColor = 0;
};

#endif // ZFLYEMBODYCOLORSCHEME_H

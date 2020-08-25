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

QColor ZFlyEmBodyColorScheme::getBodyColor(uint64_t bodyId) const
{
  return GetColorFromCode(getBodyColorCode(bodyId));
}

int ZFlyEmBodyColorScheme::getBodyColorCodeFromIndex(int index) const
{
  if (index >= 0 && index < m_colorTable.size()) {
    return m_colorTable[index];
  }

  return m_defaultColor;
}

QColor ZFlyEmBodyColorScheme::getBodyColorFromIndex(int index) const
{
  if (index >= 0 && index < m_colorTable.size()) {
    return GetColorFromCode(m_colorTable[index]);
  }

  return GetColorFromCode(m_defaultColor);
}

uint32_t ZFlyEmBodyColorScheme::getBodyColorCode(uint64_t bodyId) const
{
  return getBodyColorCodeFromIndex(getBodyColorIndex(bodyId));
}

QVector<uint32_t> ZFlyEmBodyColorScheme::buildRgbTable() const
{
  QVector<uint32_t> rgbTable(getColorNumber() - 1);
  rgbTable[0] = 0;
  for (int i = 0; i < rgbTable.size(); ++i) {
    rgbTable[i] = getBodyColorCodeFromIndex(i + 1);
//    rgbTable[i] = (164 << 24) + (color.red() << 16) + (color.green() << 8) +
//        (color.blue());
  }

  return rgbTable;
}

void ZFlyEmBodyColorScheme::mapColor(
    const uint64_t *src, uint32_t *dst, size_t v) const
{
  if (src && dst && v) {
    dst[0] = getBodyColorCode(src[0]);
    for (size_t i = 1; i < v; ++i) {
      if (src[i] == src[i - 1]) {
        dst[i] = dst[i - 1];
      } else {
        dst[i] = getBodyColorCode(src[i]);
      }
    }
  }
}

void ZFlyEmBodyColorScheme::setDefaultColor(uint32_t code)
{
  m_defaultColor = code;
}

void ZFlyEmBodyColorScheme::setDefaultColor(const QColor &color)
{
  m_defaultColor = GetIntCode(color);
}

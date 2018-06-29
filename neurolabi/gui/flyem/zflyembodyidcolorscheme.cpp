#include "zflyembodyidcolorscheme.h"

ZFlyEmBodyIdColorScheme::ZFlyEmBodyIdColorScheme(const QHash<uint64_t, QColor> &colorMap)
  : m_colorMap(colorMap)
{
  m_colorTable.append(QColor(0, 0, 0));
  for (auto it = colorMap.cbegin(); it != colorMap.cend(); ++it) {
    m_indexMap[it.key()] = m_colorTable.size();
    m_colorTable.append(it.value());
  }
}

ZFlyEmBodyIdColorScheme::~ZFlyEmBodyIdColorScheme()
{
}

void ZFlyEmBodyIdColorScheme::setColorScheme(EColorScheme scheme)
{
  ZFlyEmBodyColorScheme::setColorScheme(scheme);
}

QHash<uint64_t, int> ZFlyEmBodyIdColorScheme::getColorIndexMap() const
{
  return m_indexMap;
}

QColor ZFlyEmBodyIdColorScheme::getBodyColor(uint64_t bodyId)
{
  if (m_colorMap.contains(bodyId)) {
    return m_colorMap.value(bodyId);
  }
  return QColor(0, 0, 0);
}

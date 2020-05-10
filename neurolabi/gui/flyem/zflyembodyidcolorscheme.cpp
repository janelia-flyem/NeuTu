#include "zflyembodyidcolorscheme.h"

ZFlyEmBodyIdColorScheme::ZFlyEmBodyIdColorScheme(
    const QHash<uint64_t, QColor> &colorMap) : m_colorMap(colorMap)
{
  m_colorTable.append(QColor(0, 0, 0, 0));
  for (auto it = colorMap.cbegin(); it != colorMap.cend(); ++it) {
    m_indexMap[it.key()] = m_colorTable.size();
    m_colorTable.append(it.value());
  }
}

ZFlyEmBodyIdColorScheme::~ZFlyEmBodyIdColorScheme()
{
}

int ZFlyEmBodyIdColorScheme::getColorNumber() const
{
  int colorCount = m_colorTable.size();
  if (m_defaultColorScheme) {
    colorCount += m_defaultColorScheme->getColorNumber() - 1;
  }

  return colorCount;
}

/*
void ZFlyEmBodyIdColorScheme::updateColorIndex()
{
  if (m_defaultColorScheme) {
    int colorCount = m_defaultColorScheme->getColorNumber();
    m_colorTable.resize(m_colorMap.size() + colorCount);
  }
}
*/

void ZFlyEmBodyIdColorScheme::setColorScheme(EColorScheme scheme)
{
  ZFlyEmBodyColorScheme::setColorScheme(scheme);
}

QHash<uint64_t, int> ZFlyEmBodyIdColorScheme::getColorIndexMap() const
{
  return m_indexMap;
}

void ZFlyEmBodyIdColorScheme::setDefaultColorScheme(
    std::shared_ptr<ZFlyEmBodyColorScheme> scheme)
{
  m_defaultColorScheme = scheme;
}

int ZFlyEmBodyIdColorScheme::getBodyColorIndex(uint64_t bodyId) const
{
  int index = 0;

  if (m_indexMap.contains(bodyId)) {
    index = m_indexMap[bodyId];
  } else if (m_defaultColorScheme) {
    index = m_defaultColorScheme->getBodyColorIndex(bodyId);
    if (index > 0) { //foreground color
      index += m_colorTable.size();
    }
  }

  return index;
}

QColor ZFlyEmBodyIdColorScheme::getBodyColorFromIndex(int index) const
{
  if (index < m_colorTable.size()) {
    return m_colorTable[index];
  }

  return m_defaultColorScheme->getBodyColorFromIndex(index - m_colorTable.size());
}

/*
QColor ZFlyEmBodyIdColorScheme::getBodyColor(uint64_t bodyId) const
{
  if (m_colorMap.contains(bodyId)) {
    return m_colorMap.value(bodyId);
  } else if (m_defaultColorScheme) {
    return m_defaultColorScheme->getColor(bodyId);
  }

  return QColor(0, 0, 0, 0);
}
*/

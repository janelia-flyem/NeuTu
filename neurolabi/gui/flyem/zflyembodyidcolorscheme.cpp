#include "zflyembodyidcolorscheme.h"

#include <unordered_set>
#include <unordered_map>
#include <iostream>

int ZFlyEmBodyIdColorScheme::COLOR_CAPACITY = 65535;

ZFlyEmBodyIdColorScheme::ZFlyEmBodyIdColorScheme(
    const QHash<uint64_t, QColor> &colorMap)/* : m_colorMap(colorMap)*/
{
//  m_colorTable.append(0);
  for (auto it = colorMap.cbegin(); it != colorMap.cend(); ++it) {
    m_bodyToIndex[it.key()] = m_colorTable.size();
    uint32_t colorCode = GetIntCode(it.value());
    m_colorToIndex[colorCode] = m_colorTable.size();
    m_colorTable.append(colorCode);
  }
}

ZFlyEmBodyIdColorScheme::~ZFlyEmBodyIdColorScheme()
{
}

int ZFlyEmBodyIdColorScheme::getColorNumber() const
{
  int colorCount = m_colorTable.size();
  /*
  if (m_defaultColorScheme) {
    colorCount += m_defaultColorScheme->getColorNumber() - 1;
  }
  */

  return colorCount;
}

/*
bool ZFlyEmBodyIdColorScheme::hasOwnColor(uint64_t bodyId) const
{
  return m_bodyToIndex.contains(bodyId);
}
*/

bool ZFlyEmBodyIdColorScheme::hasExplicitColor(uint64_t bodyId) const
{
  return m_bodyToIndex.contains(bodyId);
  /*
  if (m_defaultColorScheme) {
    return m_defaultColorScheme->hasExplicitColor(bodyId);
  }
  */

//  return false;
}

bool ZFlyEmBodyIdColorScheme::removeBody(uint64_t bodyId)
{
  if (m_bodyToIndex.contains(bodyId)) {
    m_bodyToIndex.remove(bodyId);
    return true;
  }

  return false;
}

bool ZFlyEmBodyIdColorScheme::setColor(uint64_t bodyId, const QColor &color)
{
  if (hasExplicitColor(bodyId) && getBodyColor(bodyId) == color) {
    return false;
  }

  setColor(bodyId, GetIntCode(color));

  return true;
}

bool ZFlyEmBodyIdColorScheme::setColor(uint64_t bodyId, uint32_t color)
{
  if (hasExplicitColor(bodyId) && getBodyColorCode(bodyId) == color) {
    return  false;
  }

  int index = m_colorToIndex.value(color, m_colorTable.size());

  /*
  int index = m_colorTable.size();
  for (int i = 0; i < m_colorTable.size(); ++i) {
    if (color == m_colorTable[i]) {
      index = i;
      break;
    }
  }
  */

  if (index == m_colorTable.size()) {
    m_colorTable.push_back(color);
    m_colorToIndex[color] = index;
  }

  m_bodyToIndex[bodyId] = index;

  if (m_colorTable.size() > COLOR_CAPACITY) { //clear up unused colors when there list is too long
    QVector<bool> used(m_colorTable.size(), false);
    int usedColorCount = 0;
    for (int m : m_bodyToIndex) {
      if (used[m] == false) {
        ++usedColorCount;
        used[m] = true;
      }
    }

    if (usedColorCount < m_colorTable.size()) {
      QVector<uint32_t> newColorTable(usedColorCount);
      QVector<int> indexToIndex(m_colorTable.size(), 0);
      m_colorToIndex.clear();
      int newIndex = 0;
      for (int i = 0; i < m_colorTable.size(); ++i) {
        if (used[i]) {
          indexToIndex[i] = newIndex;
          m_colorToIndex[m_colorTable[i]] = newIndex;
          newColorTable[newIndex++] = m_colorTable[i];
        }
      }
      m_colorTable = newColorTable;

      for (auto iter = m_bodyToIndex.begin(); iter != m_bodyToIndex.end(); ++iter) {
        iter.value() = indexToIndex[iter.value()];
      }
    }
  }

  return true;
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
  return m_bodyToIndex;
}

/*
void ZFlyEmBodyIdColorScheme::setDefaultColorScheme(
    std::shared_ptr<ZFlyEmBodyColorScheme> scheme)
{
  m_defaultColorScheme = scheme;
}
*/

uint32_t ZFlyEmBodyIdColorScheme::getBodyColorCode(uint64_t bodyId) const
{
  if (m_bodyToIndex.contains(bodyId)) {
    return m_colorTable[m_bodyToIndex[bodyId]];
  }/* else {
    if (m_defaultColorScheme && m_defaultColorScheme->hasExplicitColor(bodyId)) {
      return m_defaultColorScheme->getBodyColorCode(bodyId);
    }
  }*/

  return m_defaultColor;
}

int ZFlyEmBodyIdColorScheme::getBodyColorIndex(uint64_t bodyId) const
{
  int index = 0;

  if (m_bodyToIndex.contains(bodyId)) {
    index = m_bodyToIndex[bodyId];
  }/* else if (m_defaultColorScheme) {
    index = m_defaultColorScheme->getBodyColorIndex(bodyId);
    if (index > 0) { //foreground color
      index += m_colorTable.size();
    }
  }*/

  return index;
}

QColor ZFlyEmBodyIdColorScheme::getBodyColorFromIndex(int index) const
{
  if (index < m_colorTable.size()) {
    return m_colorTable[index];
  }

  return m_defaultColor;
//  return m_defaultColorScheme->getBodyColorFromIndex(index - m_colorTable.size());
}

void ZFlyEmBodyIdColorScheme::print() const
{
  std::cout << "ID -> Color: " << std::endl;
  for (auto iter = m_bodyToIndex.begin(); iter != m_bodyToIndex.end(); ++iter) {
    std::cout << "  " << iter.key() << " -> "
              << m_colorTable[iter.value()] << std::endl;
  }
}

bool ZFlyEmBodyIdColorScheme::isEmpty() const
{
  return m_bodyToIndex.isEmpty();
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

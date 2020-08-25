#include "zflyemsequencercolorscheme.h"

#include <cstdint>
#include <iostream>

#include <QColor>
#include <QHash>


ZFlyEmSequencerColorScheme::ZFlyEmSequencerColorScheme()
{
    setDefaultColor(QColor(0, 0, 0, 0));
//    buildColorTable();
}

/*
QColor ZFlyEmSequencerColorScheme::getBodyColor(uint64_t bodyId) const {
    if (m_colorMap.contains(bodyId)) {
        return m_colorMap[bodyId];
    } else {
        return m_defaultColor;
    }
}

*/

uint32_t ZFlyEmSequencerColorScheme::getBodyColorCode(uint64_t bodyId) const
{
  if (m_indexMap.contains(bodyId)) {
    return m_colorTable[m_indexMap[bodyId]];
  }

  if (m_defaultColorScheme && m_defaultColorScheme->hasExplicitColor(bodyId)) {
    return m_defaultColorScheme->getBodyColorCode(bodyId);
  }

  return m_defaultColor;
}

QColor ZFlyEmSequencerColorScheme::getBodyColorFromIndex(int index) const
{
  if (index < m_colorTable.size()) {
    return m_colorTable[index];
  }

  return m_defaultColorScheme->getBodyColorFromIndex(
        index - m_colorTable.size());
}

int ZFlyEmSequencerColorScheme::getBodyColorIndex(uint64_t bodyId) const
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

void ZFlyEmSequencerColorScheme::setDefaultColor(QColor color) {
//  m_defaultColor = color;
  if (m_colorTable.isEmpty()) {
    m_colorTable.append(color.rgba());
  } else {
    m_colorTable[0] = color.rgba();
  }
}

void ZFlyEmSequencerColorScheme::setBodyColor(uint64_t bodyId, QColor color)
{
  if (m_colorMap.contains(bodyId)) {
    m_colorTable[getBodyColorIndex(bodyId)] = color.rgba();
  } else {
    m_indexMap[bodyId] = m_colorTable.size();
    m_colorTable.append(color.rgba());
  }

  m_colorMap[bodyId] = color;
}

int ZFlyEmSequencerColorScheme::getColorNumber() const
{
  int colorCount = m_colorTable.size();
  if (m_defaultColorScheme) {
    colorCount += m_defaultColorScheme->getColorNumber() - 1;
  }

  return colorCount;
}

void ZFlyEmSequencerColorScheme::clear() {
    m_colorMap.clear();
    m_indexMap.clear();
    m_colorTable.resize(1);
//    m_colorTable.clear();

    // doesn't reset default color
}

// for debugging
void ZFlyEmSequencerColorScheme::print() const {
    QHashIterator<uint64_t, QColor> colorMapIterator(m_colorMap);
    while (colorMapIterator.hasNext()) {
        colorMapIterator.next();
        std::cout << colorMapIterator.key() << ": " << colorMapIterator.value().red() << ", " << colorMapIterator.value().green()
                  << ", " << colorMapIterator.value().blue() << std::endl;
    }

}

void ZFlyEmSequencerColorScheme::setDefaultColorScheme(
    std::shared_ptr<ZFlyEmBodyColorScheme> scheme)
{
  m_defaultColorScheme = scheme;
}

void ZFlyEmSequencerColorScheme::setMainColorScheme(
    const ZFlyEmSequencerColorScheme &scheme)
{
  auto defaultScheme = m_defaultColorScheme;
  *this = scheme;
  this->setDefaultColorScheme(defaultScheme);
}

QHash<uint64_t, int> ZFlyEmSequencerColorScheme::getColorIndexMap() const
{
  return m_indexMap;
  /*
  QHash<uint64_t, int> indexMap;
  int index = 1;
  for (QHash<uint64_t, QColor>::const_iterator iter = m_colorMap.begin();
       iter != m_colorMap.end(); ++iter) {
    indexMap[iter.key()] = index++;
  }

  return indexMap;
  */
}

bool ZFlyEmSequencerColorScheme::hasExplicitColor(uint64_t bodyId) const
{
  if (m_indexMap.contains(bodyId)) {
    return true;
  }

  if (m_defaultColorScheme) {
    return m_defaultColorScheme->hasExplicitColor(bodyId);
  }

  return false;
}

/*
void ZFlyEmSequencerColorScheme::buildColorTable()
{
  m_colorTable.resize(m_colorMap.size() + 1);
  m_colorTable[0] = m_defaultColor;

  int index = 1;
  for (QHash<uint64_t, QColor>::const_iterator iter = m_colorMap.begin();
       iter != m_colorMap.end(); ++iter, ++index) {
    m_colorTable[index] = iter.value();
    m_indexMap[iter.key()] = index;
  }
}
*/

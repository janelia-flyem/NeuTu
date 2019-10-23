#include "zflyemsequencercolorscheme.h"

#include <cstdint>
#include <iostream>

#include <QColor>
#include <QHash>


ZFlyEmSequencerColorScheme::ZFlyEmSequencerColorScheme()
{
    setDefaultColor(QColor(0, 0, 0, 0));
    buildColorTable();
}

QColor ZFlyEmSequencerColorScheme::getBodyColor(uint64_t bodyId) {
    if (m_colorMap.contains(bodyId)) {
        return m_colorMap[bodyId];
    } else {
        return m_defaultColor;
    }
}

void ZFlyEmSequencerColorScheme::setDefaultColor(QColor color) {
    m_defaultColor = color;
}

void ZFlyEmSequencerColorScheme::setBodyColor(uint64_t bodyId, QColor color)
{
    m_colorMap[bodyId] = color;
}

void ZFlyEmSequencerColorScheme::clear() {
    m_colorMap.clear();

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

QHash<uint64_t, int> ZFlyEmSequencerColorScheme::getColorIndexMap() const
{
  QHash<uint64_t, int> indexMap;
  int index = 1;
  for (QHash<uint64_t, QColor>::const_iterator iter = m_colorMap.begin();
       iter != m_colorMap.end(); ++iter) {
    indexMap[iter.key()] = index++;
  }

  return indexMap;
}

void ZFlyEmSequencerColorScheme::buildColorTable()
{
  m_colorTable.resize(m_colorMap.size() + 1);
  m_colorTable[0] = m_defaultColor;

  int index = 1;
  for (QHash<uint64_t, QColor>::const_iterator iter = m_colorMap.begin();
       iter != m_colorMap.end(); ++iter, ++index) {
    m_colorTable[index] = iter.value();
  }
}

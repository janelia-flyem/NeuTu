#include "zflyemsequencercolorscheme.h"

#include <iostream>

#include <QColor>
#include <QHash>

#include "tz_stdint.h"

ZFlyEmSequencerColorScheme::ZFlyEmSequencerColorScheme()
{
    setDefaultColor(QColor(0, 0, 0, 0));
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

void ZFlyEmSequencerColorScheme::setBodyColor(uint64_t bodyId, QColor color) {
    m_colorMap[bodyId] = color;
}

void ZFlyEmSequencerColorScheme::clear() {
    m_colorMap.clear();

    // doesn't reset default color
}

// for debugging
void ZFlyEmSequencerColorScheme::print() {
    QHashIterator<uint64_t, QColor> colorMapIterator(m_colorMap);
    while (colorMapIterator.hasNext()) {
        colorMapIterator.next();
        std::cout << colorMapIterator.key() << ": " << colorMapIterator.value().red() << ", " << colorMapIterator.value().green()
                  << ", " << colorMapIterator.value().blue() << std::endl;
    }

}

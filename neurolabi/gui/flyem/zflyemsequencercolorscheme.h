#ifndef ZFLYEMSEQUENCERCOLORSCHEME_H
#define ZFLYEMSEQUENCERCOLORSCHEME_H

#include <cstdint>

#include <QColor>
#include <QHash>

#include "zflyembodycolorscheme.h"

class ZFlyEmSequencerColorScheme : public ZFlyEmBodyColorScheme
{
public:
    ZFlyEmSequencerColorScheme();
    QColor getBodyColor(uint64_t bodyId);
    void setDefaultColor(QColor color);
    void setBodyColor(uint64_t bodyId, QColor color);
    void clear();
    void print() const;

    QHash<uint64_t, int> getColorIndexMap() const;
    void buildColorTable();

private:
    QHash<uint64_t, QColor> m_colorMap;
    QColor m_defaultColor;
};




#endif // ZFLYEMSEQUENCERCOLORSCHEME_H

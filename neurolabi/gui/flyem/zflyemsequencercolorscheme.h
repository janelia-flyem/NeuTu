#ifndef ZFLYEMSEQUENCERCOLORSCHEME_H
#define ZFLYEMSEQUENCERCOLORSCHEME_H

#include <QColor>
#include <QHash>

#include "tz_stdint.h"
#include "zflyembodycolorscheme.h"

class ZFlyEmSequencerColorScheme : public ZFlyEmBodyColorScheme
{
public:
    ZFlyEmSequencerColorScheme();
    QColor getBodyColor(uint64_t bodyId);
    void setDefaultColor(QColor color);
    void setBodyColor(uint64_t bodyId, QColor color);
    void clear();
    void print();

private:
    QHash<uint64_t, QColor> m_colorMap;
    QColor m_defaultColor;

};




#endif // ZFLYEMSEQUENCERCOLORSCHEME_H

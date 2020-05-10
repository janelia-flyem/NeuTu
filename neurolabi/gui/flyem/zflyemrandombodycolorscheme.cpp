#include "zflyemrandombodycolorscheme.h"

#include "zrandomgenerator.h"

ZFlyEmRandomBodyColorScheme::ZFlyEmRandomBodyColorScheme()
{
  setColorScheme(CONV_RANDOM_COLOR);
//  m_colorTable.prepend(QColor(0, 0, 0, 0));
}

ZFlyEmRandomBodyColorScheme::~ZFlyEmRandomBodyColorScheme()
{

}

QColor ZFlyEmRandomBodyColorScheme::getBodyColorFromIndex(int index) const
{
  return m_colorTable[index];
}

int ZFlyEmRandomBodyColorScheme::getBodyColorIndex(uint64_t bodyId) const
{
  return (bodyId == 0) ? 0 : ((bodyId - 1) % (m_colorTable.size() - 1) + 1);
}

void ZFlyEmRandomBodyColorScheme::update()
{
  buildConvRandomColorTable(65535, ZRandomGenerator::UniqueSeed());
}

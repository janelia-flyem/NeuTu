#ifndef ZCOLORSCHEMETEST_H
#define ZCOLORSCHEMETEST_H

#include "ztestheader.h"

#ifdef _USE_GTEST_

#include "zcolorscheme.h"

TEST(ZColorScheme, Basic)
{
  ZColorScheme scheme;
  scheme.setColorScheme(ZColorScheme::PUNCTUM_TYPE_COLOR);

  ASSERT_EQ(15, scheme.getColorNumber());
  ASSERT_EQ(0xFF00FFFFu, scheme.getColorCode(1));
  ASSERT_EQ(0xFF00FFFF, scheme.getColorCode(uint64_t(1)));

  ASSERT_EQ(QColor(0, 255, 255, 255), scheme.getColor(1));
  ASSERT_EQ(QColor(0, 255, 255, 255), scheme.getColor(uint64_t(1)));

  scheme.setStartIndex(1);
  ASSERT_EQ(0xFF00FFFFu, scheme.getColorCode(0));
  ASSERT_EQ(0xFF00FFFF, scheme.getColorCode(uint64_t(0)));

//  scheme.printColorTable();
}

#endif


#endif // ZCOLORSCHEMETEST_H

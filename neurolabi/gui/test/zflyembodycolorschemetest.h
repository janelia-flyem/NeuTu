#ifndef ZFLYEMBODYCOLORSCHEMETEST_H
#define ZFLYEMBODYCOLORSCHEMETEST_H

#include "ztestheader.h"

#include "flyem/zflyemrandombodycolorscheme.h"
#include "flyem/zflyembodyidcolorscheme.h"
#include "flyem/zflyemnamebodycolorscheme.h"
#include "flyem/zflyemsequencercolorscheme.h"

#ifdef _USE_GTEST_

TEST(ZFlyEmRandomBodyColorScheme, Basic)
{
  ZFlyEmRandomBodyColorScheme scheme;
  ASSERT_EQ(65536, scheme.getColorNumber());
  ASSERT_EQ(0, scheme.getBodyColorIndex(0));
  ASSERT_EQ(1, scheme.getBodyColorIndex(1));
  ASSERT_EQ(65535, scheme.getBodyColorIndex(65535));
  ASSERT_EQ(scheme.getBodyColor(65536), scheme.getBodyColorFromIndex(1));
  ASSERT_EQ(2, scheme.getBodyColorIndex(2));
  ASSERT_EQ(QColor(0, 0, 0, 0), scheme.getBodyColor(0));
}

TEST(ZFlyEmBodyIdColorScheme, Basic)
{
  QHash<uint64_t, QColor> colorMap;
  colorMap[1] = Qt::red;
  colorMap[3] = Qt::green;
  colorMap[5] = Qt::blue;

  ZFlyEmBodyIdColorScheme scheme(colorMap);
//  colorScheme.printColorTable();
  ASSERT_EQ(4, scheme.getColorNumber());

  ASSERT_EQ(QColor(255, 0, 0).name().toStdString(),
            scheme.getBodyColor(1).name().toStdString());
  ASSERT_EQ(QColor(0, 255, 0).name().toStdString(),
            scheme.getBodyColor(3).name().toStdString());
  ASSERT_EQ(QColor(0, 0, 255), scheme.getBodyColor(5));
  ASSERT_EQ(QColor(0, 0, 0, 0), scheme.getBodyColor(2));
  ASSERT_EQ(QColor(0, 0, 0, 0), scheme.getBodyColor(0));

  auto defaultScheme = std::shared_ptr<ZFlyEmBodyColorScheme>(
        new ZFlyEmRandomBodyColorScheme());
  scheme.setDefaultColorScheme(defaultScheme);

  ASSERT_EQ(QColor(255, 0, 0).name().toStdString(),
            scheme.getBodyColor(1).name().toStdString());
  ASSERT_EQ(QColor(0, 255, 0).name().toStdString(),
            scheme.getBodyColor(3).name().toStdString());
  ASSERT_EQ(QColor(0, 0, 255), scheme.getBodyColor(5));
  ASSERT_EQ(QColor(0, 0, 0, 0), scheme.getBodyColor(0));
  ASSERT_EQ(65535 + 4, scheme.getColorNumber());
  ASSERT_EQ(6, scheme.getBodyColorIndex(2));

  ASSERT_EQ(defaultScheme->getBodyColor(2), scheme.getBodyColor(2));

  scheme.setDefaultColorScheme(nullptr);
  ASSERT_EQ(4, scheme.getColorNumber());

  ASSERT_EQ(QColor(255, 0, 0).name().toStdString(),
            scheme.getBodyColor(1).name().toStdString());
  ASSERT_EQ(QColor(0, 255, 0).name().toStdString(),
            scheme.getBodyColor(3).name().toStdString());
  ASSERT_EQ(QColor(0, 0, 255), scheme.getBodyColor(5));
  ASSERT_EQ(QColor(0, 0, 0, 0), scheme.getBodyColor(2));
  ASSERT_EQ(QColor(0, 0, 0, 0), scheme.getBodyColor(0));
}

TEST(ZFlyEmNameBodyColorScheme, Basic)
{
  ZFlyEmNameBodyColorScheme scheme;
  scheme.updateNameMap(1, "MBON");

//  scheme.printColorTable();
  ASSERT_EQ(QColor(0, 0, 0, 0), scheme.getBodyColor(0));
  ASSERT_EQ(QColor(255, 0, 255), scheme.getBodyColor(1));


  scheme.updateNameMap(3, "PPL1");
  scheme.updateNameMap(10, "MB-DPM");
  ASSERT_EQ(QColor(0, 255, 255), scheme.getBodyColor(3));
  ASSERT_EQ(QColor(140, 255, 0), scheme.getBodyColor(10));
}

TEST(ZFlyEmSequencerColorScheme, Basic)
{
  ZFlyEmSequencerColorScheme scheme;

  scheme.setBodyColor(1, QColor(255, 0, 0));
  scheme.setBodyColor(3, QColor(0, 255, 0));
  scheme.setBodyColor(5, QColor(0, 0, 255));
  ASSERT_EQ(QColor(255, 0, 0).name().toStdString(),
            scheme.getBodyColor(1).name().toStdString());
  ASSERT_EQ(QColor(0, 255, 0).name().toStdString(),
            scheme.getBodyColor(3).name().toStdString());
  ASSERT_EQ(QColor(0, 0, 255), scheme.getBodyColor(5));
  ASSERT_EQ(QColor(0, 0, 0, 0), scheme.getBodyColor(2));
  ASSERT_EQ(QColor(0, 0, 0, 0), scheme.getBodyColor(0));
  ASSERT_EQ(4, scheme.getColorNumber());

  scheme.setBodyColor(5, QColor(0, 0, 128));
  ASSERT_EQ(QColor(0, 0, 128), scheme.getBodyColor(5));
  ASSERT_EQ(4, scheme.getColorNumber());
}

#endif

#endif // ZFLYEMBODYCOLORSCHEMETEST_H

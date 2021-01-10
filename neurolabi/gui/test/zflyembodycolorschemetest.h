#ifndef ZFLYEMBODYCOLORSCHEMETEST_H
#define ZFLYEMBODYCOLORSCHEMETEST_H

#include "ztestheader.h"

#include "flyem/zflyemrandombodycolorscheme.h"
#include "flyem/zflyembodyidcolorscheme.h"
#include "flyem/zflyemnamebodycolorscheme.h"
#include "flyem/zflyemsequencercolorscheme.h"
#include "flyem/zflyemcompositebodycolorscheme.h"
#include "flyem/zflyemgeneralbodycolorscheme.h"

#ifdef _USE_GTEST_

class ZFLyEmMockBodyColorScheme : public ZFlyEmBodyColorScheme
{
public:
  ZFLyEmMockBodyColorScheme() {
    setColorScheme(UNIQUE_COLOR);
    _getBodyColorIndex = [this](uint64_t bodyId) {
      if (bodyId >= 10000) {
        return -1;
      }

      return int(bodyId % getColorNumber());
    };
  }

  void setMockColorTable() {
    m_colorTable.clear();
    m_colorTable.append(1);
    m_colorTable.append(2);
    m_colorTable.append(3);
  }

  void setColorTable(const QVector<uint32_t> &colorList) {
    m_colorTable = colorList;
  }

  int getBodyColorIndex(uint64_t bodyId) const override {
    return _getBodyColorIndex(bodyId);
  }

  bool hasExplicitColor(uint64_t bodyId) const override {
    return getBodyColorIndex(bodyId) >= 0;
  }

  std::function<int(uint64_t)> _getBodyColorIndex;
};

TEST(ZFlyEmBodyColorScheme, Basic)
{
  ZFLyEmMockBodyColorScheme scheme;

  ASSERT_TRUE(scheme.hasExplicitColor(0));
  ASSERT_TRUE(scheme.hasExplicitColor(1));
  ASSERT_FALSE(scheme.hasExplicitColor(20000));

  uint32_t colorCode = scheme.getBodyColorCode(2);
  ASSERT_EQ(0xFF0000FF, colorCode);
  ASSERT_EQ(QColor(0, 0, 255, 255), scheme.getBodyColor(2));

  scheme.setColorScheme(ZColorScheme::LABEL_COLOR);
  ASSERT_EQ(10, scheme.getColorNumber());

  scheme.setColorScheme(ZColorScheme::PUNCTUM_TYPE_COLOR);
  ASSERT_EQ(15, scheme.getColorNumber());
  ASSERT_EQ(0xFF00FFFF, scheme.getBodyColorCode(1));
  ASSERT_EQ(0xFF00FF00, scheme.getBodyColorCode(6));
  ASSERT_EQ(1, scheme.getBodyColorIndex(1));
  ASSERT_EQ(-1, scheme.getBodyColorIndex(20000));
  ASSERT_EQ(0, scheme.getBodyColorCode(20000));
  ASSERT_EQ(QColor(0, 0, 0, 0), scheme.getBodyColor(20000));
  ASSERT_EQ(QColor(0, 0, 0, 0), scheme.getBodyColorFromIndex(-1));

  scheme.setDefaultColor(QColor(0, 0, 0, 255));
  ASSERT_EQ(QColor(0, 0, 0, 255), scheme.getBodyColorFromIndex(-1));
  ASSERT_EQ(0xFF000000, scheme.getBodyColorCode(20000));
  ASSERT_EQ(QColor(0, 0, 0, 255), scheme.getBodyColor(20000));

  scheme.setDefaultColor(1000);
  ASSERT_EQ(1000, scheme.getBodyColorCodeFromIndex(-1));

  uint64_t src[] = {1, 1, 1, 2, 3, 20000, 20000, 6, 6};
  uint32_t dst[9];
  scheme.mapColor(src, dst, 9);
  for (size_t i = 0; i < 9; ++i) {
    ASSERT_EQ(scheme.getBodyColorCode(src[i]), dst[i]);
  }
}

TEST(ZFlyEmGeneralBodyColorScheme, Basic)
{
  ZFlyEmGeneralBodyColorScheme scheme;

  ZFLyEmMockBodyColorScheme subscheme;
  subscheme.setColorTable({1, 2, 3});
  subscheme._getBodyColorIndex = [](uint64_t bodyId) {
    if (bodyId >= 3) {
      return -1;
    }

    return int(bodyId);
  };

  scheme._hasExplicitColor = [&](uint64_t bodyId) {
    return subscheme.hasExplicitColor(bodyId);
  };
  scheme._getBodyColorCode = [&](uint64_t bodyId) {
    return subscheme.getBodyColorCode(bodyId);
  };
  scheme._getColorNumber = [&]() {
    return subscheme.getColorNumber();
  };
  scheme.setDefaultColor(100);

  ASSERT_EQ(3, scheme.getColorNumber());
  ASSERT_EQ(0, scheme.getBodyColorIndex(1));
  ASSERT_EQ(2, scheme.getBodyColorCode(1));
  ASSERT_EQ(QColor(0, 0, 2, 0), scheme.getBodyColor(1));
  ASSERT_EQ(100, scheme.getBodyColorCode(10));
  ASSERT_EQ(-1, scheme.getBodyColorIndex(3));
}

TEST(ZFlyEmCompositeBodyColorScheme, Basic)
{
  ZFlyEmCompositeBodyColorScheme scheme;
  ASSERT_EQ(0, scheme.getColorNumber());
  ASSERT_EQ(-1, scheme.getBodyColorIndex(1));

  scheme.setDefaultColor(100);
  ASSERT_EQ(100, scheme.getBodyColorCode(1));
  ASSERT_EQ(QColor(0, 0, 100, 0).name().toStdString(),
            scheme.getBodyColor(1).name().toStdString());

  ZFLyEmMockBodyColorScheme *subscheme = new ZFLyEmMockBodyColorScheme;
  scheme.appendScheme(std::shared_ptr<ZFLyEmMockBodyColorScheme>(subscheme));
  subscheme->setColorTable({1, 2, 3});
  subscheme->_getBodyColorIndex = [](uint64_t bodyId) {
    if (bodyId >= 3) {
      return -1;
    }

    return int(bodyId);
  };

  ASSERT_EQ(3, scheme.getColorNumber());
  ASSERT_EQ(1, scheme.getBodyColorIndex(1));
  ASSERT_EQ(2, scheme.getBodyColorCode(1));
  ASSERT_EQ(QColor(0, 0, 2, 0), scheme.getBodyColor(1));
  ASSERT_EQ(100, scheme.getBodyColorCode(10));
  ASSERT_EQ(-1, scheme.getBodyColorIndex(3));

  subscheme = new ZFLyEmMockBodyColorScheme;
  subscheme->setColorTable({4, 5, 6, 7});
  scheme.appendScheme(std::shared_ptr<ZFLyEmMockBodyColorScheme>(subscheme));
  subscheme->_getBodyColorIndex = [](uint64_t bodyId) {
    if (bodyId > 6) {
      return -1;
    }

    return int(bodyId - 3);
  };

  ASSERT_EQ(7, scheme.getColorNumber());
  ASSERT_EQ(1, scheme.getBodyColorIndex(1));
  ASSERT_EQ(3, scheme.getBodyColorIndex(3));
  for (uint64_t bodyId = 0; bodyId < 7; ++bodyId) {
    ASSERT_EQ(int(bodyId + 1), scheme.getBodyColorCode(bodyId));
  }

  scheme.setDefaultColor(10);
  ASSERT_EQ(10, scheme.getBodyColorCode(7));

  uint64_t src[] = {1, 1, 1, 2, 3, 20000, 20000, 6, 6};
  uint32_t dst[9];
  scheme.mapColor(src, dst, 9);
  for (size_t i = 0; i < 9; ++i) {
    ASSERT_EQ(scheme.getBodyColorCode(src[i]), dst[i]);
  }
}

TEST(ZFlyEmRandomBodyColorScheme, Basic)
{
  ZFlyEmRandomBodyColorScheme scheme;
  ASSERT_EQ(65536, scheme.getColorNumber());
  ASSERT_EQ(0, scheme.getBodyColorIndex(0));
  ASSERT_EQ(1, scheme.getBodyColorIndex(1));
  ASSERT_EQ(65535, scheme.getBodyColorIndex(65535));
  ASSERT_EQ(scheme.getBodyColor(65536), scheme.getBodyColorFromIndex(1));
  ASSERT_EQ(2, scheme.getBodyColorIndex(2));
  ASSERT_EQ(0, scheme.getBodyColorCode(0));
  ASSERT_EQ(QColor(0, 0, 0, 0).name().toStdString(),
            scheme.getBodyColor(0).name().toStdString());
}

TEST(ZFlyEmBodyIdColorScheme, Basic)
{
  {
    QHash<uint64_t, QColor> colorMap;
    colorMap[0] = Qt::transparent;
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

    /*
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

    ASSERT_EQ(defaultScheme->getBodyColorCode(2), scheme.getBodyColorCode(2));
    ASSERT_EQ(defaultScheme->getBodyColor(2), scheme.getBodyColor(2));

    scheme.setDefaultColorScheme(nullptr);
    */

    ASSERT_EQ(4, scheme.getColorNumber());

    ASSERT_EQ(QColor(255, 0, 0).name().toStdString(),
              scheme.getBodyColor(1).name().toStdString());
    ASSERT_EQ(QColor(0, 255, 0).name().toStdString(),
              scheme.getBodyColor(3).name().toStdString());
    ASSERT_EQ(QColor(0, 0, 255), scheme.getBodyColor(5));
    ASSERT_EQ(QColor(0, 0, 0, 0), scheme.getBodyColor(2));
    ASSERT_EQ(QColor(0, 0, 0, 0), scheme.getBodyColor(0));

    ASSERT_EQ(0, scheme.getBodyColorCode(100));
    scheme.setColor(100, 101);
    ASSERT_EQ(5, scheme.getColorNumber());
    ASSERT_EQ(101, scheme.getBodyColorCode(100));
    scheme.setColor(100, 102);
    ASSERT_EQ(6, scheme.getColorNumber());
    ASSERT_EQ(102, scheme.getBodyColorCode(100));
    scheme.setColor(1000, 101);
    ASSERT_EQ(6, scheme.getColorNumber());
    ASSERT_EQ(101, scheme.getBodyColorCode(1000));
  }

  {
    ZFlyEmBodyIdColorScheme scheme;
    for (int i = 0; i < ZFlyEmBodyIdColorScheme::COLOR_CAPACITY; ++i) {
      scheme.setColor(1, i);
    }
    ASSERT_EQ(ZFlyEmBodyIdColorScheme::COLOR_CAPACITY, scheme.getColorNumber());
    scheme.setColor(2, 100);
    ASSERT_EQ(ZFlyEmBodyIdColorScheme::COLOR_CAPACITY, scheme.getColorNumber());
    scheme.setColor(2, ZFlyEmBodyIdColorScheme::COLOR_CAPACITY);
    ASSERT_EQ(2, scheme.getColorNumber());
    scheme.setColor(3, 100);
    ASSERT_EQ(3, scheme.getColorNumber());
    ASSERT_EQ(ZFlyEmBodyIdColorScheme::COLOR_CAPACITY, scheme.getBodyColorCode(2));
    ASSERT_EQ(100, scheme.getBodyColorCode(3));
  }

  {
    ZFlyEmBodyIdColorScheme scheme;
    ASSERT_TRUE(scheme.setColor(1, 100));
    ASSERT_TRUE(scheme.hasExplicitColor(1));
    ASSERT_FALSE(scheme.hasExplicitColor(0));
    ASSERT_FALSE(scheme.setColor(1, 100));
    ASSERT_TRUE(scheme.setColor(1, 200));
    ASSERT_TRUE(scheme.setColor(2, 100));
    ASSERT_TRUE(scheme.hasExplicitColor(2));
    ASSERT_TRUE(scheme.removeBody(1));
    ASSERT_FALSE(scheme.hasExplicitColor(1));
    ASSERT_FALSE(scheme.removeBody(1));
    ASSERT_TRUE(scheme.removeBody(2));
  }
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
  ASSERT_EQ(0, scheme.getBodyColorCode(2));
  ASSERT_EQ(QColor(0, 0, 0, 0), scheme.getBodyColor(2));
  ASSERT_EQ(QColor(0, 0, 0, 0), scheme.getBodyColor(0));
  ASSERT_EQ(4, scheme.getColorNumber());

  scheme.setBodyColor(5, QColor(0, 0, 128));
  ASSERT_EQ(QColor(0, 0, 128), scheme.getBodyColor(5));
  ASSERT_EQ(4, scheme.getColorNumber());
}

#endif

#endif // ZFLYEMBODYCOLORSCHEMETEST_H

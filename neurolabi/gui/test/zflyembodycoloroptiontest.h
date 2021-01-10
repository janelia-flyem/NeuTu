#ifndef ZFLYEMBODYCOLOROPTIONTEST_H
#define ZFLYEMBODYCOLOROPTIONTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "flyem/zflyembodycoloroption.h"

#ifdef _USE_GTEST_

TEST(ZFlyEmBodyColorOption, Basic)
{
  ASSERT_EQ("Normal", ZFlyEmBodyColorOption::GetColorMapName(
              ZFlyEmBodyColorOption::BODY_COLOR_NORMAL));
  ASSERT_EQ("Name", ZFlyEmBodyColorOption::GetColorMapName(
              ZFlyEmBodyColorOption::BODY_COLOR_NAME));
  ASSERT_EQ("Sequencer", ZFlyEmBodyColorOption::GetColorMapName(
              ZFlyEmBodyColorOption::BODY_COLOR_SEQUENCER));
  ASSERT_EQ("Protocol", ZFlyEmBodyColorOption::GetColorMapName(
              ZFlyEmBodyColorOption::BODY_COLOR_PROTOCOL));

  ASSERT_EQ(ZFlyEmBodyColorOption::BODY_COLOR_NORMAL,
            ZFlyEmBodyColorOption::GetColorOption("Normal"));
  ASSERT_EQ(ZFlyEmBodyColorOption::BODY_COLOR_NAME,
            ZFlyEmBodyColorOption::GetColorOption("Name"));
  ASSERT_EQ(ZFlyEmBodyColorOption::BODY_COLOR_SEQUENCER,
            ZFlyEmBodyColorOption::GetColorOption("Sequencer"));
  ASSERT_EQ(ZFlyEmBodyColorOption::BODY_COLOR_SEQUENCER_NORMAL,
            ZFlyEmBodyColorOption::GetColorOption("Sequencer+Normal"));
  ASSERT_EQ(ZFlyEmBodyColorOption::BODY_COLOR_PROTOCOL,
            ZFlyEmBodyColorOption::GetColorOption("Protocol"));


  QList<ZFlyEmBodyColorOption::EColorOption> colorOptionList =
      QList<ZFlyEmBodyColorOption::EColorOption>()
      << ZFlyEmBodyColorOption::BODY_COLOR_NORMAL
      << ZFlyEmBodyColorOption::BODY_COLOR_NAME
      << ZFlyEmBodyColorOption::BODY_COLOR_SEQUENCER
      << ZFlyEmBodyColorOption::BODY_COLOR_PROTOCOL;

  foreach (ZFlyEmBodyColorOption::EColorOption option, colorOptionList) {
    ASSERT_EQ(option, ZFlyEmBodyColorOption::GetColorOption(
                ZFlyEmBodyColorOption::GetColorMapName(option)));
  }


}

#endif

#endif // ZFLYEMBODYCOLOROPTIONTEST_H

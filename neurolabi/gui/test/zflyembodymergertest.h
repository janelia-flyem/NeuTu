#ifndef ZFLYEMBODYMERGERTEST_H
#define ZFLYEMBODYMERGERTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "flyem/zflyembodymerger.h"

#ifdef _USE_GTEST_

TEST(ZFlyEmBodyMerger, Basic)
{
  ZFlyEmBodyMerger merger;
  merger.pushMap(1, 2);

  ASSERT_EQ(2, (int) merger.getFinalLabel(1));
  ASSERT_NE(3, (int) merger.getFinalLabel(1));
  ASSERT_EQ(5, (int) merger.getFinalLabel(5));

  merger.pushMap(2, 3);
  ASSERT_EQ(3, (int) merger.getFinalLabel(1));
  ASSERT_EQ(3, (int) merger.getFinalLabel(2));
  ASSERT_EQ(5, (int) merger.getFinalLabel(5));

  merger.print();

  merger.undo();
  ASSERT_EQ(2, (int) merger.getFinalLabel(1));
  ASSERT_NE(3, (int) merger.getFinalLabel(1));
  ASSERT_EQ(5, (int) merger.getFinalLabel(5));

  merger.print();

  merger.redo();
  ASSERT_EQ(3, (int) merger.getFinalLabel(1));
  ASSERT_EQ(3, (int) merger.getFinalLabel(2));
  ASSERT_EQ(5, (int) merger.getFinalLabel(5));

  merger.print();

  ZFlyEmBodyMerger::TLabelMap newMap;
  newMap[3] = 4;
  newMap[5] = 6;

  merger.pushMap(newMap);
  merger.print();
  ASSERT_EQ(4, (int) merger.getFinalLabel(1));
  ASSERT_EQ(4, (int) merger.getFinalLabel(2));
  ASSERT_EQ(6, (int) merger.getFinalLabel(5));
}

#endif

#endif // ZFLYEMBODYMERGERTEST_H

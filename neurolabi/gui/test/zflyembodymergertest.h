#ifndef ZFLYEMBODYMERGERTEST_H
#define ZFLYEMBODYMERGERTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "flyem/zflyembodymerger.h"
#include "zjsonarray.h"

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

  //merger.print();

  merger.undo();
  ASSERT_EQ(2, (int) merger.getFinalLabel(1));
  ASSERT_NE(3, (int) merger.getFinalLabel(1));
  ASSERT_EQ(5, (int) merger.getFinalLabel(5));

  //merger.print();

  merger.redo();
  ASSERT_EQ(3, (int) merger.getFinalLabel(1));
  ASSERT_EQ(3, (int) merger.getFinalLabel(2));
  ASSERT_EQ(5, (int) merger.getFinalLabel(5));

  //merger.print();

  ZFlyEmBodyMerger::TLabelMap newMap;
  newMap[3] = 4;
  newMap[5] = 6;

  merger.pushMap(newMap);
  //merger.print();
  ASSERT_EQ(4, (int) merger.getFinalLabel(1));
  ASSERT_EQ(4, (int) merger.getFinalLabel(2));
  ASSERT_EQ(6, (int) merger.getFinalLabel(5));
}

TEST(ZFlyEmBodyMerger, Json)
{
  ZFlyEmBodyMerger bodyMerger;
  bodyMerger.pushMap(1, 2);
  bodyMerger.pushMap(2, 3);
  bodyMerger.pushMap(4, 3);
  bodyMerger.pushMap(5, 3);
  bodyMerger.pushMap(3, 6);
  bodyMerger.pushMap(7, 8);

  ZJsonArray jsonArray = bodyMerger.toJsonArray();
  ZFlyEmBodyMerger bodyMerger2;
  bodyMerger2.decodeJsonString(jsonArray.dumpString());

  for (int i = 0; i < 100; ++i) {
    ASSERT_EQ(bodyMerger.getFinalLabel(i), bodyMerger2.getFinalLabel(i));
  }
  //bodyMerger2.print();

}

#endif

#endif // ZFLYEMBODYMERGERTEST_H

#ifndef ZSTACKSOURCETEST_H
#define ZSTACKSOURCETEST_H

#include "ztestheader.h"
#include "imgproc/zindexstacksource.h"

#ifdef _USE_GTEST_

TEST(ZStackSource, ZIndexStackSource)
{
  ZIndexStackSource *source = new ZIndexStackSource();
  source->setSize(3, 4, 5);
  ASSERT_EQ(0, source->getIntValue(0, 0, 0));
  ASSERT_EQ(1, source->getIntValue(1, 0, 0));
  ASSERT_EQ(4, source->getIntValue(1, 1, 0));
  ASSERT_EQ(12, source->getIntValue(0, 0, 1));
  ASSERT_EQ(0, source->getIntValue(0, 0, 6));
}

#endif

#endif // ZSTACKSOURCETEST_H

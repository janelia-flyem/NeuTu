#ifndef ZITEMCHUNKTEST_H
#define ZITEMCHUNKTEST_H

#ifdef _USE_GTEST_

#include "gtest/gtest.h"
#include "bigdata/zmockitemchunk.h"

TEST(ZItemChunk, Basic)
{
  ZMockItemChunk chunk;
  ASSERT_FALSE(chunk.isReady());
  ASSERT_TRUE(chunk.isValid());

  chunk.setReady(true);
  ASSERT_TRUE(chunk.isReady());

  ASSERT_FALSE(chunk.hasItem(1));
  std::vector<int> itemList = chunk.getItemList();
  ASSERT_TRUE(itemList.empty());

  chunk.invalidate();
  ASSERT_FALSE(chunk.isValid());
}

#endif


#endif // ZITEMCHUNKTEST_H

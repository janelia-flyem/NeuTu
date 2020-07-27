#ifndef ZDATACHUNKTEST_H
#define ZDATACHUNKTEST_H

#ifdef _USE_GTEST_

#include "gtest/gtest.h"
#include "bigdata/zdatachunk.h"

TEST(ZDataChunk, Basic)
{
  ZDataChunk chunk;
  ASSERT_FALSE(chunk.isReady());
  ASSERT_TRUE(chunk.isValid());

  ASSERT_TRUE(chunk.updateNeeded());

  chunk.setReady(true);
  ASSERT_TRUE(chunk.isReady());
  ASSERT_FALSE(chunk.updateNeeded());

  chunk.invalidate();
  ASSERT_FALSE(chunk.isValid());
  ASSERT_FALSE(chunk.updateNeeded());

  chunk.setReady(false);
  ASSERT_FALSE(chunk.updateNeeded());
}

#endif

#endif // ZDATACHUNKTEST_H

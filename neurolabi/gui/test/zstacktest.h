#ifndef ZSTACKTEST_H
#define ZSTACKTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zstackgraph.h"
#include "c_stack.h"
#include "zstack.hxx"
#include "zdebug.h"

#ifdef _USE_GTEST_
TEST(ZStack, Basic)
{
  ZStack stack;
  ASSERT_TRUE(stack.isEmpty());
  ASSERT_FALSE(stack.isVirtual());
  ASSERT_FALSE(stack.isSwc());
  ASSERT_EQ(stack.width(), 0);
  ASSERT_EQ(stack.height(), 0);
  ASSERT_EQ(stack.depth(), 0);
  ASSERT_EQ(stack.kind(), 0);
  ASSERT_EQ(stack.channelNumber(), 0);

  ZStack stack2(GREY, 10, 20, 30, 2);
  ASSERT_FALSE(stack2.isEmpty());
  ASSERT_FALSE(stack2.isVirtual());
  ASSERT_FALSE(stack2.isSwc());
  ASSERT_EQ(stack2.width(), 10);
  ASSERT_EQ(stack2.height(), 20);
  ASSERT_EQ(stack2.depth(), 30);
  ASSERT_EQ(stack2.kind(), GREY);
  ASSERT_EQ(stack2.channelNumber(), 2);

  ZStack stack3(GREY, 10, 20, 30, 2, true);
  ASSERT_FALSE(stack3.isEmpty());
  ASSERT_TRUE(stack3.isVirtual());
  ASSERT_FALSE(stack3.isSwc());
  ASSERT_EQ(stack3.width(), 10);
  ASSERT_EQ(stack3.height(), 20);
  ASSERT_EQ(stack3.depth(), 30);
}

#endif

#endif // ZSTACKTEST_H

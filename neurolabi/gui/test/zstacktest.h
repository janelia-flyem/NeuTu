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

TEST(ZStack, value) {
  ZStack stack(GREY, 1, 1, 1, 1);
  stack.setOne();
  ASSERT_EQ(1, stack.getIntValue(0, 0, 0, 0));
  ASSERT_EQ(0, stack.getIntValue(1, 0, 0, 0));
  ASSERT_EQ(0, stack.getIntValue(0, 1, 0, 0));
  ASSERT_EQ(0, stack.getIntValue(0, 0, 1, 0));
  ASSERT_EQ(0, stack.getIntValue(0, 0, 0, 1));

  ZStack stack2(GREY, 5, 4, 3, 2);
  stack2.setZero();
  stack2.setValue(0, 0, 0, 0, 1);
  stack2.setValue(0, 0, 0, 1, 2);
  stack2.setValue(0, 3, 0, 0, 3);
  ASSERT_EQ(1, stack2.getIntValue(0, 0, 0, 0));
  ASSERT_EQ(2, stack2.getIntValue(0, 0, 0, 1));
  ASSERT_EQ(3, stack2.getIntValue(0, 3, 0, 0));
  ASSERT_EQ(0, stack2.getIntValue(0, 3, 0, 2));

  ZStack stack3(COLOR, 5, 4, 3, 2);
  stack3.setZero();
  stack3.setValue(0, 0, 0, 0, 1);
  stack3.setValue(0, 0, 0, 1, 2);
  stack3.setValue(0, 3, 0, 0, 3);
  stack3.setValue(0, 3, 1, 1, 355);
  ASSERT_EQ(1, stack3.getIntValue(0, 0, 0, 0));
  ASSERT_EQ(2, stack3.getIntValue(0, 0, 0, 1));
  ASSERT_EQ(3, stack3.getIntValue(0, 3, 0, 0));
  ASSERT_EQ(0, stack3.getIntValue(0, 3, 0, 2));
  ASSERT_EQ(355, stack3.getIntValue(0, 3, 1, 1));

  ZStack stack4(COLOR, 5, 4, 3, 2);
  stack4.setZero();
  stack4.setIntValue(0, 0, 0, 0, 1);
  stack4.setIntValue(0, 0, 0, 1, 2);
  stack4.setIntValue(0, 3, 0, 0, 3);
  stack4.setIntValue(0, 3, 1, 1, 355);
  ASSERT_EQ(1, stack4.getIntValue(0, 0, 0, 0));
  ASSERT_EQ(2, stack4.getIntValue(0, 0, 0, 1));
  ASSERT_EQ(3, stack4.getIntValue(0, 3, 0, 0));
  ASSERT_EQ(0, stack4.getIntValue(0, 3, 0, 2));
  ASSERT_EQ(355, stack4.getIntValue(0, 3, 1, 1));

  tic();
  for (int i = 0; i < 1000000; ++i) {
    stack4.setIntValue(0, 3, 1, 1, 355);
  }
  ptoc();

  ZStack stack5(GREY, 5, 4, 3, 2);
  stack5.setZero();
  stack5.setIntValue(0, 0, 0, 0, 1);
  stack5.setIntValue(0, 0, 0, 1, 2);
  stack5.setIntValue(0, 3, 0, 0, 3);
  stack5.setIntValue(0, 3, 1, 1, 355);
  ASSERT_EQ(1, stack5.getIntValue(0, 0, 0, 0));
  ASSERT_EQ(2, stack5.getIntValue(0, 0, 0, 1));
  ASSERT_EQ(3, stack5.getIntValue(0, 3, 0, 0));
  ASSERT_EQ(0, stack5.getIntValue(0, 3, 0, 2));
  ASSERT_EQ(255, stack5.getIntValue(0, 3, 1, 1));

  ZStack stack6(GREY16, 5, 4, 3, 2);
  stack6.setZero();
  stack6.setIntValue(0, 0, 0, 0, 1);
  stack6.setIntValue(0, 0, 0, 1, 2);
  stack6.setIntValue(0, 3, 0, 0, 3);
  stack6.setIntValue(0, 3, 1, 1, 355);
  stack6.setIntValue(1, 3, 1, 1, 65536);
  ASSERT_EQ(1, stack6.getIntValue(0, 0, 0, 0));
  ASSERT_EQ(2, stack6.getIntValue(0, 0, 0, 1));
  ASSERT_EQ(3, stack6.getIntValue(0, 3, 0, 0));
  ASSERT_EQ(0, stack6.getIntValue(0, 3, 0, 2));
  ASSERT_EQ(355, stack6.getIntValue(0, 3, 1, 1));
  ASSERT_EQ(65535, stack6.getIntValue(1, 3, 1, 1));
}

TEST(ZStack, DataPoint)
{
  ZStack stack(GREY, 10, 20, 30, 2);

  stack.setZero();

  ASSERT_EQ(0.0, stack.max());
}

TEST(ZStack, downsample)
{
  ZStack *stack = new ZStack(GREY, 5, 5, 5, 1);
  stack->downsampleMax(1, 1, 1);
  stack->printInfo();
  ASSERT_EQ(3, stack->width());
  ASSERT_EQ(3, stack->height());
  ASSERT_EQ(3, stack->depth());
  delete stack;
}

TEST(ZStack, array)
{
  ZStack *stack = new ZStack(GREY, 3, 4, 5, 1);
  ASSERT_EQ(stack->array8(), stack->getDataPointer(0, 0, 0));
  ASSERT_EQ(stack->array8() + 1, stack->getDataPointer(1, 0, 0));
  ASSERT_EQ(stack->array8() + 2, stack->getDataPointer(2, 0, 0));
  ASSERT_EQ(stack->array8() + 3, stack->getDataPointer(0, 1, 0));
  ASSERT_EQ(stack->array8() + 12, stack->getDataPointer(0, 0, 1));

  stack->setOffset(1, 2, 3);
  ASSERT_EQ(NULL, stack->getDataPointer(0, 0, 0));
  ASSERT_EQ(stack->array8() + 1, stack->getDataPointer(2, 2, 3));
  ASSERT_EQ(stack->array8() + 2, stack->getDataPointer(3, 2, 3));
  ASSERT_EQ(stack->array8() + 3, stack->getDataPointer(1, 3, 3));
  ASSERT_EQ(stack->array8() + 12, stack->getDataPointer(1, 2, 4));

  delete stack;
}

#endif

#endif // ZSTACKTEST_H

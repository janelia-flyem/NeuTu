#ifndef ZSTACKTEST_H
#define ZSTACKTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zstackgraph.h"
#include "c_stack.h"
#include "zstack.hxx"
#include "zdebug.h"
#include "zintcuboid.h"
#include "zstackfactory.h"

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

  ASSERT_EQ(12000, (int) stack3.getByteNumber());
  ASSERT_EQ(6000, (int) stack3.getByteNumber(ZStack::SINGLE_CHANNEL));
  ASSERT_EQ(200, (int) stack3.getByteNumber(ZStack::SINGLE_PLANE));
  ASSERT_EQ(10, (int) stack3.getByteNumber(ZStack::SINGLE_ROW));

  ZStack stack4(COLOR, 10, 20, 30, 2, true);
  ASSERT_EQ(36000, (int) stack4.getByteNumber());
  ASSERT_EQ(18000, (int) stack4.getByteNumber(ZStack::SINGLE_CHANNEL));
  ASSERT_EQ(600, (int) stack4.getByteNumber(ZStack::SINGLE_PLANE));
  ASSERT_EQ(30, (int) stack4.getByteNumber(ZStack::SINGLE_ROW));
  ASSERT_EQ(3, (int) stack4.getByteNumber(ZStack::SINGLE_VOXEL));

  ASSERT_EQ(6000, (int) stack4.getVoxelNumber());
  ASSERT_EQ(6000, (int) stack4.getVoxelNumber(ZStack::SINGLE_CHANNEL));
  ASSERT_EQ(200, (int) stack4.getVoxelNumber(ZStack::SINGLE_PLANE));
  ASSERT_EQ(10, (int) stack4.getVoxelNumber(ZStack::SINGLE_ROW));
  ASSERT_EQ(1, (int) stack4.getVoxelNumber(ZStack::SINGLE_VOXEL));

  ASSERT_TRUE(stack4.sourcePath().empty());
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
  for (int i = 0; i < 100000; ++i) {
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

  ZStack stack7(FLOAT32, 5, 4, 3, 2);
  stack7.setZero();
  stack7.setIntValue(0, 0, 0, 0, 1);
  stack7.setIntValue(0, 0, 0, 1, 2);
  stack7.setIntValue(0, 3, 0, 0, 3);
  stack7.setIntValue(0, 3, 1, 1, 355);
  stack7.setIntValue(1, 3, 1, 1, 65536);
  ASSERT_EQ(1, stack7.getIntValue(0, 0, 0, 0));
  ASSERT_EQ(2, stack7.getIntValue(0, 0, 0, 1));
  ASSERT_EQ(3, stack7.getIntValue(0, 3, 0, 0));
  ASSERT_EQ(0, stack7.getIntValue(0, 3, 0, 2));
  ASSERT_EQ(355, stack7.getIntValue(0, 3, 1, 1));
  ASSERT_EQ(65536, stack7.getIntValue(1, 3, 1, 1));

  ZStack stack8(FLOAT64, 5, 4, 3, 2);
  stack8.setZero();
  stack8.setIntValue(0, 0, 0, 0, 1);
  stack8.setIntValue(0, 0, 0, 1, 2);
  stack8.setIntValue(0, 3, 0, 0, 3);
  stack8.setIntValue(0, 3, 1, 1, 355);
  stack8.setIntValue(1, 3, 1, 1, 65536);
  ASSERT_EQ(1, stack8.getIntValue(0, 0, 0, 0));
  ASSERT_EQ(2, stack8.getIntValue(0, 0, 0, 1));
  ASSERT_EQ(3, stack8.getIntValue(0, 3, 0, 0));
  ASSERT_EQ(0, stack8.getIntValue(0, 3, 0, 2));
  ASSERT_EQ(355, stack8.getIntValue(0, 3, 1, 1));
  ASSERT_EQ(65536, stack8.getIntValue(1, 3, 1, 1));

  ZIntCuboid box(1, 2, 3, 4, 5, 6);
  ZStack stack9(GREY, box, 1);
  stack9.setZero();
  stack9.setValue(0, 0, 0, 0, 1);
  stack9.setValue(0, 0, 0, 1, 2);
  stack9.setValue(0, 3, 0, 0, 3);
  ASSERT_EQ(0, stack9.getIntValue(0, 0, 0, 0));
  ASSERT_EQ(0, stack9.getIntValue(0, 0, 0, 1));
  ASSERT_EQ(0, stack9.getIntValue(0, 3, 0, 0));
  ASSERT_EQ(0, stack9.getIntValue(0, 3, 0, 2));

  stack9.setValue(1, 2, 3, 0, 1);
  stack9.setValue(1, 2, 3, 1, 2);
  stack9.setValue(1, 5, 3, 0, 3);
  ASSERT_EQ(1, stack9.getIntValue(1, 2, 3, 0));
  ASSERT_EQ(0, stack9.getIntValue(1, 2, 3, 1));
  ASSERT_EQ(3, stack9.getIntValue(1, 5, 3, 0));
  ASSERT_EQ(0, stack9.getIntValue(0, 3, 0, 2));

  ASSERT_EQ(1, stack9.getIntValueLocal(0, 0, 0, 0));
  ASSERT_EQ(0, stack9.getIntValueLocal(0, 0, 0, 1));
  ASSERT_EQ(3, stack9.getIntValueLocal(0, 3, 0, 0));
  ASSERT_EQ(0, stack9.getIntValueLocal(0, 3, 0, 2));

  ASSERT_EQ(1, stack9.value8(0));
  ASSERT_EQ(3, stack9.value8(12));
}

TEST(ZStack, DataPoint)
{
  ZStack stack(GREY, 10, 20, 30, 2);

  stack.setZero();

  ASSERT_EQ(0.0, stack.max());

  stack.setOne();
  ASSERT_EQ(1.0, stack.min());
  ASSERT_EQ(1.0, stack.max());
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

TEST(ZStack, Gradient)
{
  ZStack *stack = ZStackFactory::makeIndexStack(5, 5, 5);
  Print_Stack_Value(stack->c_stack());
  Stack *out = C_Stack::computeGradient(stack->c_stack());
  Print_Stack_Value(out);


  delete stack;
  C_Stack::kill(out);
}

TEST(ZStack, BorderShrink)
{
  ZStack *stack = ZStackFactory::makeOneStack(3, 3, 3);
  C_Stack::shrinkBorder(stack->c_stack(), 1);

  Print_Stack_Value(stack->c_stack());
}

#endif

#endif // ZSTACKTEST_H

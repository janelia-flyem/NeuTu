#ifndef ZSTACKTEST_H
#define ZSTACKTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zstackgraph.h"
#include "c_stack.h"
#include "zstack.hxx"
#include "zdebug.h"
#include "geometry/zintcuboid.h"
#include "zstackfactory.h"
#include "zstackutil.h"
#include "zstackarray.h"

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

  Mc_Stack *stackData = C_Stack::make(COLOR, 3, 4, 5, 2);
  ZStack stack5(stackData);
  ASSERT_EQ(60, (int) stack5.getVoxelNumber());
  ASSERT_EQ(360, (int) stack5.getByteNumber());

  Mc_Stack *stackData2 = C_Stack::make(GREY, 2, 4, 5, 2);
  stack5.setData(stackData2, C_Stack::kill);
  ASSERT_EQ(40, (int) stack5.getVoxelNumber());
  ASSERT_EQ(80, (int) stack5.getByteNumber());

  Mc_Stack *stackData3 = new Mc_Stack;
  stack5.setData(stackData3, C_Stack::cppDelete);

  Mc_Stack *stackData4 = (Mc_Stack*) malloc(sizeof(Mc_Stack));
  stackData4->array = NULL;
  stack5.setData(stackData4, C_Stack::systemKill);

  Stack *stackData5 = C_Stack::make(GREY, 2, 3, 4);
  stack5.consume(stackData5);
  ASSERT_EQ(24, (int) stack5.getVoxelNumber());
  ASSERT_EQ(24, (int) stack5.getByteNumber());

  ZStack *stack6 = new ZStack(GREY, 2, 3, 4, 2);
  stack5.consume(stack6);
  ASSERT_EQ(24, (int) stack5.getVoxelNumber());
  ASSERT_EQ(48, (int) stack5.getByteNumber());

  ZSingleChannelStack *sstack1 = stack5.singleChannelStack(0);
  ZSingleChannelStack *sstack2 = stack5.singleChannelStack(1);

  sstack1->getStat()->hist();
  sstack2->getStat()->hist();

  ZStack *stack7 = stack5.getSingleChannel(0);
  delete stack7;

  Stack *s1 = C_Stack::make(GREY16, 3, 4, 5);
  Stack *s2 = C_Stack::make(GREY16, 3, 4, 5);
  Stack *s3 = C_Stack::make(GREY16, 3, 4, 5);
  Mc_Stack *s4 = ZStack::MakeMcStack(s1, s2, s3);

  C_Stack::kill(s1);
  C_Stack::kill(s2);
  C_Stack::kill(s3);
  C_Stack::kill(s4);
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
  for (int i = 0; i < 10000; ++i) {
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

TEST(ZStack, IO)
{
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/benchmark/binary/3d/series.tif");
  ASSERT_EQ(200, stack.width());
  ASSERT_EQ(200, stack.height());
  ASSERT_EQ(15, stack.depth());

  std::cout << stack.min() << std::endl;
  std::cout << stack.max() << std::endl;

}

TEST(ZStack, Shape)
{
  ZIntCuboid box(1, 2, 3, 4, 5, 6);
  ZStack stack(GREY, box, 1);
  ASSERT_TRUE(stack.contains(1, 2, 3));
  ASSERT_FALSE(stack.contains(0, 2, 3));
  ASSERT_TRUE(stack.contains(ZPoint(2, 3, 4)));
  ASSERT_TRUE(stack.contains(ZIntPoint(4, 5, 6)));
  ASSERT_TRUE(stack.contains(1, 2));
  ASSERT_FALSE(stack.contains(0, 2));

  ASSERT_TRUE(stack.containsRaw(0, 2, 3));
  ASSERT_FALSE(stack.containsRaw(0, 2, 5));
  ASSERT_TRUE(stack.containsRaw(ZPoint(3, 3, 3)));

  ASSERT_EQ(box.getWidth(), stack.width());
  ASSERT_EQ(box.getHeight(), stack.height());
  ASSERT_EQ(box.getDepth(), stack.depth());

  stack.reshape(3, 9, 1);
  ASSERT_EQ(1, stack.getOffset().getX());
  ASSERT_EQ(2, stack.getOffset().getY());
  ASSERT_EQ(3, stack.getOffset().getZ());
  ASSERT_EQ(4, stack.width());

  stack.autoThreshold();

  stack.reshape(8, 4, 2);

  ZIntCuboid box2 = stack.getBoundBox();
  ASSERT_EQ(ZIntPoint(1, 2, 3), box2.getMinCorner());
  ASSERT_EQ(8, box2.getWidth());
  ASSERT_EQ(4, box2.getHeight());
  ASSERT_EQ(2, box2.getDepth());

  Cuboid_I box3;
  stack.getBoundBox(&box3);
  ASSERT_EQ(1, box3.cb[0]);
  ASSERT_EQ(2, box3.cb[1]);
  ASSERT_EQ(3, box3.cb[2]);
  ASSERT_EQ(8, box3.ce[0]);
  ASSERT_EQ(5, box3.ce[1]);
  ASSERT_EQ(4, box3.ce[2]);

  ASSERT_TRUE(stack.hasOffset());
}

TEST(ZStack, DataPoint)
{
  ZStack stack(GREY, 10, 20, 30, 2);

  stack.setZero();

  ASSERT_EQ(0.0, stack.max());

  stack.setOne();
  ASSERT_EQ(1.0, stack.min());
  ASSERT_EQ(1.0, stack.max());

  ZStack *stack2 = stack.getSingleChannel(0);
  ASSERT_EQ(1.0, stack2->min());
  ASSERT_EQ(1.0, stack2->max());

  stack2->addIntValue(1, 2, 3, 0, 1);

  ASSERT_EQ(2.0, stack2->max());
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

  uint8_t array[60];
  for (size_t i = 0; i < 60; ++i) {
    array[i] = i;
  }
  stack->copyValueFrom(array, 60);
  ASSERT_EQ(5, stack->getIntValue(5));

  delete stack;
}

TEST(ZStack, arrayValue)
{
  ZStack stack(GREY, 2, 3, 4, 1);
  stack.setZero();

  ZStack stack2(GREY, 2, 3, 4, 1);
  stack2.setOne();

  stack2.paste(&stack);
  ASSERT_EQ(1, stack.getIntValue(0));

  ZStack stack3(GREY, ZIntCuboid(1, 2, 3, 4, 5, 6), 1);
  stack3.setIntValue(1, 2, 3, 0, 2);
  stack3.setIntValue(2, 3, 4, 0, 2);
  stack3.paste(&stack);
  ASSERT_EQ(1, stack.getIntValue(0));
  ASSERT_EQ(2, stack.getIntValue(1, 2, 3));


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
  ZStack *stack = ZStackFactory::MakeOneStack(3, 3, 3);
  C_Stack::shrinkBorder(stack->c_stack(), 1);

  Print_Stack_Value(stack->c_stack());
}

TEST(ZStack, equal)
{
  ZStack *stack1 = ZStackFactory::MakeOneStack(3, 3, 3);
  ZStack *stack2 = ZStackFactory::MakeOneStack(3, 3, 3);

  ASSERT_TRUE(stack1->equals(*stack2));

  stack1->setValue(0, 0, 0);
  ASSERT_FALSE(stack1->equals(*stack2));

  ZStack *stack3 = ZStackFactory::MakeOneStack(2, 3, 4);
  ASSERT_FALSE(stack1->equals(*stack3));

  ZStack stack4;
  stack4.load(GET_TEST_DATA_DIR + "/benchmark/block.tif");

  ASSERT_TRUE(stack4.equals(stack4));
}

TEST(ZStackUtil, Basic)
{
  ZStack stack1;
  stack1.setDsIntv(1, 1, 1);

  ASSERT_EQ(8, zstack::GetDsVolume(stack1));

  ZStack stack2;
  ASSERT_EQ(1, zstack::GetDsVolume(stack2));

  ASSERT_TRUE(zstack::DsIntvGreaterThan()(stack1, stack2));

  stack2.setDsIntv(2, 1, 1);
  ASSERT_EQ(12, zstack::GetDsVolume(stack2));
  ASSERT_FALSE(zstack::DsIntvGreaterThan()(stack1, stack2));
}

class ZStackArrayTest: public ::testing::Test {

public:
  ZStackArray sa;
  ZStack *stack1 = nullptr;
  ZStack *stack2 = nullptr;
  ZStack *stack3 = nullptr;

  void SetUp() override;

  void TearDown() override {
  }
};

void ZStackArrayTest::SetUp()
{
  stack1 = ZStackFactory::MakeVirtualStack(5, 5, 5);
  stack1->setDsIntv(1, 0, 0);

  stack2 = ZStackFactory::MakeVirtualStack(5, 5, 5);
  stack2->setDsIntv(0, 1, 1);

  stack3 = ZStackFactory::MakeVirtualStack(5, 5, 5);
  stack3->setDsIntv(1, 1, 1);

  sa.append(stack1);
  sa.append(stack2);
  sa.append(stack3);
}


TEST_F(ZStackArrayTest, Basic)
{
  ASSERT_EQ(3, int(sa.size()));
}

TEST_F(ZStackArrayTest, Sort)
{
  sa.sort(zstack::DsIntvGreaterThan());
  ASSERT_EQ(stack3, sa[0].get());
  ASSERT_EQ(stack2, sa[1].get());
  ASSERT_EQ(stack1, sa[2].get());
}

#endif

#endif // ZSTACKTEST_H

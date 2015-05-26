#ifndef ZSPARSESTACKTEST_H
#define ZSPARSESTACKTEST_H

#include "ztestheader.h"
#include "zsparsestack.h"
#include "neutubeconfig.h"
#include "zstackfactory.h"

#ifdef _USE_GTEST_

TEST(ZSparseStack, basic)
{
  ZSparseStack spStack;

  ZObject3dScan *obj = new ZObject3dScan;
  obj->addStripe(0, 0);
  obj->addSegment(0, 1);
  spStack.setObjectMask(obj);

  ZStackBlockGrid *stackGrid = new ZStackBlockGrid;
  stackGrid->setBlockSize(2, 2, 2);
  stackGrid->setGridSize(3, 3, 3);

  ZStack *stack = ZStackFactory::makeOneStack(2, 2, 2);
  stackGrid->consumeStack(ZIntPoint(0, 0, 0), stack);

  stack = ZStackFactory::makeOneStack(2, 2, 2);
  stack->setOffset(2, 2, 2);
  stackGrid->consumeStack(ZIntPoint(1, 1, 1), stack);

  spStack.setGreyScale(stackGrid);
  spStack.setBaseValue(0);

  ASSERT_EQ(2, (int) spStack.getObjectVolume());
  ZStack *stack2 = spStack.getStack();
  ASSERT_EQ(1, stack2->getIntValue(0, 0, 0));
  ASSERT_EQ(1, stack2->getIntValue(1, 0, 0));
  ASSERT_EQ(0, stack2->getIntValue(0, 1, 0));

  ASSERT_EQ(0, stack2->getIntValue(2, 2, 2));
  ASSERT_EQ(0, stack2->getIntValue(2, 2, 1));
  ASSERT_EQ(0, stack2->getIntValue(2, 3, 2));

  obj->addSegment(2, 2, 2, 4);
  spStack.deprecate(ZSparseStack::STACK);
  stack2 = spStack.getStack();

  stack2->printInfo();

  ASSERT_EQ(1, stack2->getIntValue(2, 2, 2));
  ASSERT_EQ(1, stack2->getIntValue(3, 2, 2));
  ASSERT_EQ(0, stack2->getIntValue(2, 2, 1));
  ASSERT_EQ(0, stack2->getIntValue(2, 3, 2));
}

TEST(ZSparseStack, downsample)
{
  ZStackBlockGrid *stackGrid = new ZStackBlockGrid;
  stackGrid->setBlockSize(2, 2, 2);
  stackGrid->setGridSize(2, 2, 2);
  ZStack *stack = ZStackFactory::makeUniformStack(2, 2, 2, 1);
  stackGrid->consumeStack(ZIntPoint(0, 0, 0), stack);

  stack = ZStackFactory::makeUniformStack(2, 2, 2, 2);
  stackGrid->consumeStack(ZIntPoint(1, 0, 0), stack);

  stack = ZStackFactory::makeUniformStack(2, 2, 2, 3);
  stackGrid->consumeStack(ZIntPoint(0, 1, 0), stack);

  stack = ZStackFactory::makeUniformStack(2, 2, 2, 4);
  stackGrid->consumeStack(ZIntPoint(1, 1, 0), stack);

  ZStackBlockGrid *dsGrid = stackGrid->makeDownsample(1, 1, 1);

  delete dsGrid;
  delete stackGrid;

  ZSparseStack spStack;

  stackGrid = new ZStackBlockGrid;
  stackGrid->setBlockSize(1024, 1024, 512);
  stackGrid->setGridSize(2, 2, 2);
  spStack.setGreyScale(stackGrid);

  stack = ZStackFactory::makeUniformStack(1024, 1024, 512, 1);
  stackGrid->consumeStack(ZIntPoint(0, 0, 0), stack);

  stack = ZStackFactory::makeUniformStack(1024, 1024, 512, 2);
  stackGrid->consumeStack(ZIntPoint(1, 0, 0), stack);

  stack = ZStackFactory::makeUniformStack(1024, 1024, 512, 3);
  stackGrid->consumeStack(ZIntPoint(0, 1, 1), stack);

  stack = ZStackFactory::makeUniformStack(1024, 1024, 512, 4);
  stackGrid->consumeStack(ZIntPoint(1, 1, 1), stack);

  ZObject3dScan *obj = new ZObject3dScan;

  for (int i = 0; i < 2048; ++i) {
    obj->addSegment(0, i, 0, 2047);
    obj->addSegment(1023, i, 0, 2047);
  }


  /*
  obj->addSegment(0, 0, 0, 2047);
  obj->addSegment(0, 1, 0, 2047);
  obj->addSegment(1, 0, 0, 2047);
  obj->addSegment(1, 1, 0, 2047);
  obj->addSegment(0, 2, 0, 2047);
  obj->addSegment(0, 3, 0, 2047);
  obj->addSegment(1, 2, 0, 2047);
  obj->addSegment(1, 3, 0, 2047);
  obj->addSegment(1023, 1023, 0, 2047);
  */

  spStack.setObjectMask(obj);

  ZStack *stack2 = spStack.getStack();

  stack2->save(GET_TEST_DATA_DIR + "/test.tif");
}

#endif

#endif // ZSPARSESTACKTEST_H

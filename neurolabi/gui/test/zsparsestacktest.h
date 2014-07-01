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

#endif

#endif // ZSPARSESTACKTEST_H

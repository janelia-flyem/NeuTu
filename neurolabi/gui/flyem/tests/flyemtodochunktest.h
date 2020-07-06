#ifndef FLYEMTODOCHUNKTEST_H
#define FLYEMTODOCHUNKTEST_H

#ifdef _USE_GTEST_

#include "gtest/gtest.h"
#include "flyem/flyemtodochunk.h"

TEST(FlyEmTodoChunk, Basic)
{
  FlyEmTodoChunk chunk;
  ASSERT_FALSE(chunk.isReady());
  ASSERT_TRUE(chunk.isValid());

  chunk.setReady(true);
  ASSERT_TRUE(chunk.isReady());

  ASSERT_FALSE(chunk.hasItem(1, 2, 3));
  std::vector<ZFlyEmToDoItem> itemList = chunk.getItemList();
  ASSERT_TRUE(itemList.empty());

  chunk.invalidate();
  ASSERT_FALSE(chunk.isValid());
}

TEST(FlyEmTodoChunk, Edit)
{
  FlyEmTodoChunk chunk;
  ZFlyEmToDoItem item(1, 2, 3);
  chunk.addItem(item);
  ASSERT_TRUE(chunk.hasItem(1, 2, 3));

  chunk.addItem(ZFlyEmToDoItem(7, 8, 9));
  std::vector<ZFlyEmToDoItem> itemList = chunk.getItemList();
  ASSERT_EQ(2, itemList.size());

  {
    std::set<ZIntPoint> itemSet;
    chunk.forEachItem([&](const ZFlyEmToDoItem &item) {
      itemSet.insert(item.getPosition());
    });
    ASSERT_EQ(2, itemSet.size());
    ASSERT_TRUE(itemSet.count(ZIntPoint(1, 2, 3)) > 0);
    ASSERT_TRUE(itemSet.count(ZIntPoint(7, 8, 9)) > 0);
  }

  ASSERT_TRUE(chunk.removeItem(1, 2, 3));
  ASSERT_EQ(1, chunk.countItem());

  ASSERT_FALSE(chunk.removeItem(1, 2, 3));
  ASSERT_EQ(1, chunk.countItem());

  ASSERT_TRUE(chunk.removeItem(ZIntPoint(7, 8, 9)));
  ASSERT_TRUE(chunk.isEmpty());

  chunk.addItem({ZFlyEmToDoItem(1, 2, 3), ZFlyEmToDoItem(7, 8, 9)});
  ASSERT_EQ(2, chunk.countItem());

  {
    std::set<ZIntPoint> itemSet;
    chunk.forEachItem([&](const ZFlyEmToDoItem &item) {
      itemSet.insert(item.getPosition());
    });
    ASSERT_EQ(2, itemSet.size());
    ASSERT_TRUE(itemSet.count(ZIntPoint(1, 2, 3)) > 0);
    ASSERT_TRUE(itemSet.count(ZIntPoint(7, 8, 9)) > 0);
  }
}

#endif

#endif // FLYEMTODOCHUNKTEST_H

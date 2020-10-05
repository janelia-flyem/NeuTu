#ifndef FLYEMTODOBLOCKGRIDTEST_H
#define FLYEMTODOBLOCKGRIDTEST_H

#ifdef _USE_GTEST_

#include "gtest/gtest.h"
#include "flyem/flyemtodoblockgrid.h"
#include "flyem/flyemtodomocksource.h"

TEST(FlyEmTodoBlockGrid, Source)
{
  FlyEmTodoBlockGrid grid;
  grid.setBlockSize(64, 64, 64);
  ASSERT_EQ(ZIntPoint(64, 64, 64), grid.getBlockSize());

  auto source = std::shared_ptr<FlyEmTodoMockSource>(
        new FlyEmTodoMockSource);
  grid.setSource(source);

  ASSERT_EQ(ZIntPoint(32, 32, 32), grid.getBlockSize());
  ASSERT_EQ(ZIntPoint(16, 16, 16), grid.getGridSize());

//  grid.addItem(ZFlyEmToDoItem(16, 16, 16));

  {
    FlyEmTodoChunk chunk = grid.getChunk(0, 0, 0);
    ZFlyEmToDoItem item = chunk.getItem({16, 16, 16});
    ASSERT_TRUE(item.isValid());
    ASSERT_EQ(ZIntPoint(16, 16, 16), item.getPosition());

    item = chunk.getItem({15, 16, 16});
    ASSERT_FALSE(item.isValid());
  }

  {
    ZFlyEmToDoItem item = grid.getItem({160, 160, 160});
    ASSERT_FALSE(item.isValid());
    source->saveItem(ZFlyEmToDoItem(1160, 160, 160));
    ASSERT_FALSE(grid.getItem({160, 160, 160}).isValid());
    grid.syncItemToCache({160, 160, 160});
    ASSERT_TRUE(grid.getItem({160, 160, 160}).isValid());
  }

  {
    ZFlyEmToDoItem item = grid.pickCachedItem(16, 16, 16);
    ASSERT_TRUE(item.isValid());

    item = grid.pickCachedItem(15, 16, 16);
    ASSERT_FALSE(item.isValid());

    item = grid.pickClosestCachedItem(15, 16, 16, 2);
    ASSERT_TRUE(item.isValid());
    ASSERT_EQ(ZIntPoint(16, 16, 16), item.getPosition());

    grid.addItem(ZFlyEmToDoItem(15, 16, 16));
    item = grid.pickCachedItem(15, 16, 16);
    ASSERT_TRUE(item.isValid());

    std::vector<ZFlyEmToDoItem> itemList;
    grid.forEachItemInChunk(0, 0, 0, [&](const ZFlyEmToDoItem &item) {
      itemList.push_back(item);
    });
    ASSERT_EQ(2, itemList.size());

    grid.addItem(ZFlyEmToDoItem(16, 16, 28));
    grid.addItem(ZFlyEmToDoItem(16, 16, 33));
    itemList.clear();
    grid.forEachItemInChunk(0, 0, 0, [&](const ZFlyEmToDoItem &item) {
      itemList.push_back(item);
    });
    ASSERT_EQ(3, itemList.size());
    item = grid.pickClosestCachedItem(16, 16, 31, 5);
    ASSERT_TRUE(item.isValid());
    ASSERT_EQ(ZIntPoint(16, 16, 33), item.getPosition());

    grid.addItem(ZFlyEmToDoItem(29, 29, 29));
    grid.addItem(ZFlyEmToDoItem(33, 32, 32));
    grid.addItem(ZFlyEmToDoItem(29, 29, 33));
    grid.addItem(ZFlyEmToDoItem(29, 33, 33));
    grid.addItem(ZFlyEmToDoItem(33, 29, 29));
    item = grid.pickClosestCachedItem(31, 31, 31, 10);
    ASSERT_TRUE(item.isValid());
    ASSERT_EQ(ZIntPoint(33, 32, 32), item.getPosition());

    ASSERT_THROW(grid.addItem(ZFlyEmToDoItem(512, 512, 512)), std::exception);

    grid.removeItem(ZIntPoint(29, 29, 29));
    grid.removeItem(ZIntPoint(33, 32, 32));
    ASSERT_FALSE(grid.getItem(ZIntPoint(29, 29, 29)).isValid());
    ASSERT_THROW(grid.removeItem(ZIntPoint(29, 29, 29)), std::exception);

  }
}

TEST(FlyEmTodoBlockGrid, Move)
{
  FlyEmTodoBlockGrid grid;
  grid.setBlockSize(64, 64, 64);
  ASSERT_EQ(ZIntPoint(64, 64, 64), grid.getBlockSize());

  auto source = std::shared_ptr<FlyEmTodoMockSource>(
        new FlyEmTodoMockSource);
  grid.setSource(source);

  grid.addItem(ZFlyEmToDoItem(10, 20, 30));
  grid.moveItem({10, 20, 30}, {40, 50, 60});
  ASSERT_FALSE(grid.getItem({10, 20, 30}).isValid());
  ASSERT_TRUE(grid.getItem({40, 50, 60}).isValid());
}

TEST(FlyEmTodoBlockGrid, Update)
{
  FlyEmTodoBlockGrid grid;
  grid.setBlockSize(64, 64, 64);
  ASSERT_EQ(ZIntPoint(64, 64, 64), grid.getBlockSize());

  auto source = std::shared_ptr<FlyEmTodoMockSource>(
        new FlyEmTodoMockSource);
  grid.setSource(source);

  grid.addItem(ZFlyEmToDoItem(10, 20, 30));
  ASSERT_TRUE(grid.getItem({10, 20, 30}).isValid());
  source->removeItem({10, 20, 30});
  grid.syncItemToCache({10, 20, 30});
  ASSERT_FALSE(grid.getItem({10, 20, 30}).isValid());
  source->saveItem(ZFlyEmToDoItem(10, 20, 30));
  grid.syncItemToCache({10, 20, 30});
  ASSERT_TRUE(grid.getItem({10, 20, 30}).isValid());
}

TEST(FlyEmTodoBlockGrid, Selection)
{
  FlyEmTodoBlockGrid grid;
  grid.setBlockSize(64, 64, 64);
  auto source = std::shared_ptr<FlyEmTodoMockSource>(
        new FlyEmTodoMockSource);
  grid.setSource(source);

  ASSERT_FALSE(grid.setSelectionForCached(ZIntPoint(16, 16, 16), true));

  grid.addItem(ZFlyEmToDoItem(16, 16, 28));
  ZFlyEmToDoItem item = grid.getCachedItem(16, 16, 16);
  ASSERT_FALSE(item.isSelected());

  item = grid.getCachedItem(16, 16, 28);
  ASSERT_FALSE(item.isSelected());

  ASSERT_TRUE(grid.setSelectionForCached(ZIntPoint(16, 16, 16), true));
  ASSERT_TRUE(grid.setSelectionForCached(ZIntPoint(16, 16, 28), true));

 item = grid.getCachedItem(16, 16, 16);
  ASSERT_TRUE(item.isSelected());

  item = grid.getCachedItem(16, 16, 28);
  ASSERT_TRUE(item.isSelected());
}

#endif

#endif // FLYEMTODOBLOCKGRIDTEST_H

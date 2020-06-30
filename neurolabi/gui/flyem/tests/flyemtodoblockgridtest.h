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
  auto source = std::shared_ptr<FlyEmTodoMockSource>(
        new FlyEmTodoMockSource);
  grid.setSource(source);
  ASSERT_EQ(ZIntPoint(16, 16, 16), grid.getGridSize());

  FlyEmTodoChunk chunk = grid.getTodoChunk(0, 0, 0);
  ZFlyEmToDoItem item = chunk.getItem(16, 16, 16);
  ASSERT_TRUE(item.isValid());
  ASSERT_EQ(ZIntPoint(16, 16, 16), item.getPosition());

  item = chunk.getItem(15, 16, 16);
  ASSERT_FALSE(item.isValid());
  /*
  chunk.forEachItem([](const ZFlyEmToDoItem &item) {
    std::cout << item << std::endl;
  });
  */
}

#endif

#endif // FLYEMTODOBLOCKGRIDTEST_H

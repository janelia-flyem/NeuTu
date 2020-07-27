#ifndef ZBLOCKGRIDTEST_H
#define ZBLOCKGRIDTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "bigdata/zstackblockgrid.h"
#include "bigdata/zblockgrid.h"
#include "zstack.hxx"
#include "geometry/zaffinerect.h"
#include "geometry/zgeometry.h"

#ifdef _USE_GTEST_

TEST(ZBlockGrid, basic)
{
  ZBlockGrid grid;
  ASSERT_TRUE(grid.isEmpty());

  grid.setGridSize(2, 2, 2);
  ASSERT_EQ(8, grid.getBlockNumber());

  grid.setBlockSize(32, 32, 32);
  ASSERT_EQ(64, grid.getSpatialWidth());

  ASSERT_EQ(0, grid.getHashIndex(ZIntPoint(0, 0, 0)));
  ASSERT_EQ(1, grid.getHashIndex(ZIntPoint(1, 0, 0)));
  ASSERT_EQ(7, grid.getHashIndex(ZIntPoint(1, 1, 1)));
  ASSERT_EQ(-1, grid.getHashIndex(ZIntPoint(2, 0, 0)));

  ZBlockGrid::Location location = grid.getLocation(5, 6, 7);
  ASSERT_EQ(0, location.getBlockIndex().getX());
  ASSERT_EQ(0, location.getBlockIndex().getY());
  ASSERT_EQ(0, location.getBlockIndex().getZ());
  ASSERT_EQ(5, location.getLocalPosition().getX());
  ASSERT_EQ(6, location.getLocalPosition().getY());
  ASSERT_EQ(7, location.getLocalPosition().getZ());

  location = grid.getLocation(-5, -6, -7);
  ASSERT_EQ(-1, location.getBlockIndex().getX());
  ASSERT_EQ(-1, location.getBlockIndex().getY());
  ASSERT_EQ(-1, location.getBlockIndex().getZ());
  ASSERT_EQ(27, location.getLocalPosition().getX());
  ASSERT_EQ(26, location.getLocalPosition().getY());
  ASSERT_EQ(25, location.getLocalPosition().getZ());

  std::cout << "zblockgridtest: v8" << std::endl;
}

TEST(ZBlockGrid, Set)
{
  ZBlockGrid grid;
  grid.setBlockSize(32, 64, 128);
  grid.setGridByRange(ZIntCuboid(1, 2, 3, 320, 64 * 20, 128 * 30));
  ASSERT_EQ(ZIntPoint(1, 2, 3), grid.getMinPoint());
  ASSERT_EQ(ZIntPoint(32, 64, 128), grid.getBlockSize());
  ASSERT_EQ(ZIntPoint(10, 20, 30), grid.getGridSize());

  grid.setGridByRange(ZIntCuboid(0, 0, 0, 320, 64 * 20, 128 * 30));
  ASSERT_EQ(ZIntPoint(0, 0, 0), grid.getMinPoint());
  ASSERT_EQ(ZIntPoint(32, 64, 128), grid.getBlockSize());
  ASSERT_EQ(ZIntPoint(11, 21, 31), grid.getGridSize());

  grid.setGridByRange(ZIntCuboid(1, 1, 1, 320, 64 * 20, 128 * 30));
  ASSERT_EQ(ZIntPoint(1, 1, 1), grid.getMinPoint());
  ASSERT_EQ(ZIntPoint(32, 64, 128), grid.getBlockSize());
  ASSERT_EQ(ZIntPoint(10, 20, 30), grid.getGridSize());
}

TEST(ZBlockGrid, Intersect)
{
  ZBlockGrid grid;
  grid.setBlockSize(32, 32, 32);
  grid.setGridByRange(ZIntCuboid(0, 0, 0, 511, 511, 511));

  ZAffineRect rect;
  rect.setPlane(ZAffinePlane(ZPoint(0, 0, 16), ZPoint(1, 0, 0), ZPoint(0, 1, 0)));
  rect.setSize(100, 200);
  std::vector<ZIntPoint> blocks;
  grid.forEachIntersectedBlock(rect, [&](int i, int j, int k) {
    blocks.emplace_back(i, j, k);
//    ZIntCuboid box = grid.getBlockBox(ZIntPoint(i, j, k));
//    std::cout << box << std::endl;
  });
  ASSERT_EQ(8, blocks.size());
  ASSERT_EQ(ZIntPoint(0, 0, 0), blocks[0]);
  ASSERT_EQ(ZIntPoint(1, 0, 0), blocks[1]);
  ASSERT_EQ(ZIntPoint(0, 1, 0), blocks[2]);
  ASSERT_EQ(ZIntPoint(1, 1, 0), blocks[3]);


  rect.setPlane(ZAffinePlane(ZPoint(16, 0, 0), ZPoint(0, 1, 0), ZPoint(0, 0, 1)));
  rect.setSize(100, 200);
  ASSERT_FALSE(zgeom::Intersects(rect, ZIntCuboid(0, 64, 64, 31, 95, 95)));


  blocks.clear();
  grid.forEachIntersectedBlock(rect, [&](int i, int j, int k) {
    blocks.emplace_back(i, j, k);
//    ZIntCuboid box = grid.getBlockBox(ZIntPoint(i, j, k));
//    std::cout << box << std::endl;
  });

  ASSERT_EQ(8, blocks.size());
  ASSERT_EQ(ZIntPoint(0, 0, 0), blocks[0]);
  ASSERT_EQ(ZIntPoint(0, 1, 0), blocks[1]);
  ASSERT_EQ(ZIntPoint(0, 0, 1), blocks[2]);
  ASSERT_EQ(ZIntPoint(0, 1, 1), blocks[3]);
}

TEST(ZStackBlockGrid, basic)
{
  ZStackBlockGrid grid;
  ASSERT_TRUE(grid.isEmpty());

  grid.setGridSize(2, 2, 2);
  ASSERT_EQ(8, grid.getBlockNumber());

  grid.setBlockSize(1, 1, 1);
  ASSERT_EQ(2, grid.getSpatialWidth());
  ASSERT_EQ(2, grid.getSpatialHeight());
  ASSERT_EQ(2, grid.getSpatialHeight());

  ZStack *stack = new ZStack(GREY, 1, 1, 1, 1);
  stack->setValue(0, 0, 0, 0, 3);
  ASSERT_EQ(3, stack->value(0, 0, 0));
  stack->printInfo();
  grid.consumeStack(ZIntPoint(0, 0, 0), stack);

  ASSERT_EQ(3, grid.getValue(0, 0, 0));
  ASSERT_EQ(0, grid.getValue(0, 1, 0));
  ASSERT_EQ(0, grid.getValue(1, 0, 0));
  ASSERT_EQ(0, grid.getValue(0, 0, 1));

  stack = new ZStack(GREY, 1, 1, 1, 1);
  stack->setValue(0, 0, 0, 0, 4);
  grid.consumeStack(ZIntPoint(1, 0, 0), stack);
  ASSERT_EQ(3, grid.getValue(0, 0, 0));
  ASSERT_EQ(0, grid.getValue(0, 1, 0));
  ASSERT_EQ(4, grid.getValue(1, 0, 0));

  grid.clearStack();
  ASSERT_EQ(0, grid.getValue(0, 0, 0));
  ASSERT_EQ(0, grid.getValue(0, 1, 0));
  ASSERT_EQ(0, grid.getValue(1, 0, 0));

  grid.setGridSize(5, 4, 3);
  grid.setBlockSize(16, 32, 64);
  stack = new ZStack(GREY, 16, 32, 64, 1);
  stack->setZero();;
  grid.consumeStack(ZIntPoint(0, 0, 0), stack);

  stack = new ZStack(GREY, 16, 32, 64, 1);
  stack->setOne();;
  grid.consumeStack(ZIntPoint(1, 1, 0), stack);

  stack = new ZStack(GREY, 16, 32, 64, 1);
  stack->setOne();;
  grid.consumeStack(ZIntPoint(2, 2, 1), stack);

  ZStack *out = grid.toStack();
  delete out;
  out = nullptr;
//  out->save(GET_TEST_DATA_DIR + "/test.tif");

  ASSERT_EQ(1, grid.getValue(39, 78, 73));

  /*
  tic();
  for (size_t i = 0; i < 200000; ++i) {
    grid.getValue(39, 78, 73);
  }
  ptoc();
  */

  ZStackBlockGrid grid1;
  grid1.setGridSize(5, 4, 3);
  grid1.setBlockSize(16, 32, 64);
  stack = new ZStack(GREY, 16, 32, 64, 1);
  stack->setZero();;
  grid1.consumeStack(ZIntPoint(0, 0, 0), stack);

  ZStackBlockGrid grid2;
  grid2.setGridSize(5, 4, 3);
  grid2.setBlockSize(16, 32, 64);
  stack = new ZStack(GREY, 16, 32, 64, 1);
  stack->setOne();;
  grid2.consumeStack(ZIntPoint(1, 1, 0), stack);

  grid1.consume(grid2);


}
#endif

#endif // ZBLOCKGRIDTEST_H

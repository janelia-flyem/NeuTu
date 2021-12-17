#ifndef ZBLOCKGRIDTEST_H
#define ZBLOCKGRIDTEST_H

#ifdef _USE_GTEST_

#include "test/ztestheader.h"

#include "bigdata/zblockgrid.h"

TEST(ZBlockGrid, Basic)
{
  ZBlockGrid grid;
  ASSERT_TRUE(grid.isEmpty());

  ASSERT_EQ(-1, grid.getHashIndex({0, 0, 0}));
  ASSERT_FALSE(grid.getBlockIndex(0, 0, 0).isValid());
}

#endif


#endif // ZBLOCKGRIDTEST_H

#ifndef ZVOXELARRAYTEST_H
#define ZVOXELARRAYTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zvoxelarray.h"

#ifdef _USE_GTEST_

TEST(ZVoxelArray, Basic)
{
  ZVoxelArray array;
  ASSERT_TRUE(array.empty());
  ASSERT_TRUE(array.isEmpty());

  array.append(3, 4, 5, 1);
  ASSERT_EQ(1, (int) array.size());

  array.setValue(0, 0);
  ASSERT_EQ(0, array.value(0));

  ASSERT_EQ(0, (int) array.findClosest(3, 4, 5));

  array.append(4, 5, 6, 2);
  ASSERT_EQ(0, (int) array.findClosest(3, 4, 5));
  ASSERT_EQ(1, (int) array.findClosest(4, 5, 6));

  array.append(10, 0, 0, 1);
  ASSERT_EQ(0, (int) array.findClosest(3, 4, 5));
  ASSERT_EQ(1, (int) array.findClosest(4, 5, 6));

}

#endif

#endif // ZVOXELARRAYTEST_H

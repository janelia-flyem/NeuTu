#ifndef MISCUTILITYTEST_H
#define MISCUTILITYTEST_H

#include "ztestheader.h"
#include "misc/miscutility.h"
#include "geometry/zintcuboid.h"

#ifdef _USE_GTEST_

TEST(miscutility, getDsIntv)
{
  {
    ZIntPoint pt = misc::getDsIntvFor3DVolume(
          ZIntCuboid(ZIntPoint(0, 0, 0), ZIntPoint(512, 512, 512)));
    ASSERT_EQ(1, pt.getX());
    ASSERT_EQ(1, pt.getY());
    ASSERT_EQ(1, pt.getZ());
  }

  {
    ZIntPoint pt = misc::getDsIntvFor3DVolume(
          ZIntCuboid(ZIntPoint(0, 0, 0), ZIntPoint(1024, 1024, 512)));
    ASSERT_EQ(2, pt.getX());
    ASSERT_EQ(2, pt.getY());
    ASSERT_EQ(2, pt.getZ());
  }

  {
    ASSERT_EQ(1, misc::getIsoDsIntvFor3DVolume(7.0, true));
    ASSERT_EQ(4, misc::getIsoDsIntvFor3DVolume(125.0, false));
    ASSERT_EQ(7, misc::getIsoDsIntvFor3DVolume(125.0, true));
  }
}

#endif


#endif // MISCUTILITYTEST_H

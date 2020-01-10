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

  {
    ASSERT_EQ(4.0, misc::GetExpansionScale(64, 1));
    ASSERT_EQ(5.0, misc::GetExpansionScale(125, 1));
  }
}

TEST(miscutility, GetBoundBox)
{
  mylib::Dimn_Type dims[3] = {3, 4, 5};
  ZArray array(mylib::INT8_TYPE, 3, dims);
  array.setStartCoordinate(1, 2, 3);

  ZIntCuboid box = misc::GetBoundBox(&array);
  ASSERT_EQ(ZIntPoint(1, 2, 3), box.getFirstCorner());
  ASSERT_EQ(ZIntPoint(3, 5, 7), box.getLastCorner());
}

TEST(miscutility, EstimateSplitRoi)
{
  ZIntCuboid box = misc::EstimateSplitRoi(
        ZIntCuboid(ZIntPoint(0, 0, 0), ZIntPoint(512, 512, 128)));
  std::cout << box.toString() << std::endl;
  std::cout << box.getVolume() << std::endl;
  ASSERT_EQ(ZIntPoint(-314, -314, -101), box.getFirstCorner());
  ASSERT_EQ(ZIntPoint(826, 826, 229), box.getLastCorner());
}



#endif


#endif // MISCUTILITYTEST_H

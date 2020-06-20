#ifndef DATA3D_UTILITIESTEST_H
#define DATA3D_UTILITIESTEST_H


#ifdef _USE_GTEST_

#include "gtest/gtest.h"

#include "data3d/utilities.h"

TEST(data3d, Utilities)
{
  std::vector<neutu::data3d::ETarget> targetList =
      neutu::data3d::GetTargetList();
  ASSERT_EQ(neutu::data3d::TARGET_COUNT - 1, targetList.size());

  targetList = neutu::data3d::GetTarget2dList();
  ASSERT_EQ(7, targetList.size());

  targetList = neutu::data3d::GetTarget2dList({neutu::data3d::ETarget::WIDGET});
  ASSERT_EQ(6, targetList.size());

  targetList = neutu::data3d::GetTarget2dList(
  {neutu::data3d::ETarget::WIDGET,
   neutu::data3d::ETarget::CANVAS_3D,
   neutu::data3d::ETarget::ONLY_3D});
  ASSERT_EQ(6, targetList.size());


  targetList = neutu::data3d::GetTarget2dList(
  {neutu::data3d::ETarget::WIDGET,
   neutu::data3d::ETarget::STACK_CANVAS,
   neutu::data3d::ETarget::TILE_CANVAS});
  ASSERT_EQ(4, targetList.size());

  targetList = neutu::data3d::GetTarget2dObjectCanvasList();
  ASSERT_EQ(4, targetList.size());

  targetList = neutu::data3d::GetTarget2dObjectCanvasList(
    {neutu::data3d::ETarget::ROAMING_OBJECT_CANVAS});
  ASSERT_EQ(3, targetList.size());

  targetList = neutu::data3d::GetTargetSettled2dObjectCanvasList();
  ASSERT_EQ(3, targetList.size());
}

#endif


#endif // DATA3D_UTILITIESTEST_H

#ifndef ZOBJECT3DFACTORYTEST_H
#define ZOBJECT3DFACTORYTEST_H

#include "ztestheader.h"
#include "zobject3dfactory.h"
#include "zstackfactory.h"
#include "zobject3dscan.h"
#include "zobject3d.h"
#include "zobject3darray.h"

#ifdef _USE_GTEST_

TEST(ZObject3dFactory, makeObject)
{
  ZStack *stack = ZStackFactory::MakeZeroStack(3, 3, 3);
  ZObject3dArray *out = ZObject3dFactory::MakeRegionBoundary(
        *stack, ZObject3dFactory::OUTPUT_COMPACT);

  ASSERT_TRUE(out->empty());
  delete out;

  stack->setIntValue(1, 1, 1, 0, 1);
  out = ZObject3dFactory::MakeRegionBoundary(
        *stack, ZObject3dFactory::OUTPUT_COMPACT);
  ASSERT_EQ(1, (int) out->size());

  ZObject3d *obj = out->at(0);
  ASSERT_EQ(1, (int) obj->size());
  ASSERT_EQ(1, (int) obj->getLabel());
  delete out;

  stack->setIntValue(0, 1, 1, 0, 2);
  out = ZObject3dFactory::MakeRegionBoundary(
        *stack, ZObject3dFactory::OUTPUT_COMPACT);
  ASSERT_EQ(2, (int) out->size());

  delete stack;
  delete out;
}

TEST(ZObject3dFactory, CuboidObject) {
  ZIntCuboid cuboid(0, 0, 0, 1, 0, 0);

  ZObject3dScan obj = ZObject3dFactory::MakeObject3dScan(cuboid);
  ASSERT_EQ(2, (int) obj.getVoxelNumber());

  cuboid.setLastCorner(1, 1, 0);
  obj = ZObject3dFactory::MakeObject3dScan(cuboid);
  ASSERT_EQ(4, (int) obj.getVoxelNumber());

  cuboid.setFirstCorner(1, 2, 3);
  cuboid.setLastCorner(6, 5, 4);
  obj = ZObject3dFactory::MakeObject3dScan(cuboid);
  ASSERT_EQ(48, (int) obj.getVoxelNumber());
  ASSERT_TRUE(obj.contains(1, 2, 3));
}

TEST(ZObject3dFactory, MakeObject3dArray)
{
  ZStack *stack = ZStackFactory::MakeZeroStack(3, 3, 3);

  std::vector<ZObject3d*> objArray =
      ZObject3dFactory::MakeObject3dArray(*stack);
  ASSERT_TRUE(objArray.empty());

  stack->setIntValue(1, 1, 1, 0, 1);
  objArray = ZObject3dFactory::MakeObject3dArray(*stack);
  ASSERT_EQ(1, (int) objArray.size());

  stack->setIntValue(0, 1, 1, 0, 2);
  objArray = ZObject3dFactory::MakeObject3dArray(*stack);
  ASSERT_EQ(2, (int) objArray.size());
  ZObject3d *obj = objArray[0];
  ASSERT_EQ(1, (int) obj->size());
}

#endif

#endif // ZOBJECT3DFACTORYTEST_H

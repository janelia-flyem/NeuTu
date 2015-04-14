#ifndef ZOBJECT3DFACTORYTEST_H
#define ZOBJECT3DFACTORYTEST_H

#include "ztestheader.h"
#include "zobject3dfactory.h"
#include "zstackfactory.h"
#include "zobject3dscan.h"

#ifdef _USE_GTEST_

TEST(ZObject3dFactory, makeObject)
{
  ZStack *stack = ZStackFactory::makeZeroStack(3, 3, 3);
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
  ASSERT_EQ(1, obj->getLabel());
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

#endif

#endif // ZOBJECT3DFACTORYTEST_H

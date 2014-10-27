#ifndef ZOBJECT3DFACTORYTEST_H
#define ZOBJECT3DFACTORYTEST_H

#include "ztestheader.h"
#include "zobject3dfactory.h"
#include "zstackfactory.h"

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

#endif

#endif // ZOBJECT3DFACTORYTEST_H

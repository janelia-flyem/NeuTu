#ifndef ZFLYEMNEURONIMAGEFACTORYTEST_H
#define ZFLYEMNEURONIMAGEFACTORYTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "flyem/zflyemneuronimagefactory.h"
#include "flyem/zflyemdatabundle.h"
#include "zobject3dscan.h"

#ifdef _USE_GTEST_

TEST(ZFlyEmNeuronImageFactory, createImage)
{
  ZFlyEmNeuronImageFactory factory;

  ZObject3dScan obj;
  Stack *stack = factory.createImage(obj);
  ASSERT_EQ(0, stack);

  obj.addSegment(0, 0, 1, 3);
  obj.addSegment(0, 1, 1, 3);
  stack = factory.createImage(obj);
  ASSERT_EQ(3, C_Stack::width(stack));
  ASSERT_EQ(1, C_Stack::height(stack));
  ASSERT_EQ(1, C_Stack::depth(stack));

  C_Stack::kill(stack);

  obj.addSegment(3, 1, 1, 3);
  stack = factory.createImage(obj);
  ASSERT_EQ(3, C_Stack::width(stack));
  ASSERT_EQ(4, C_Stack::height(stack));
  ASSERT_EQ(1, C_Stack::depth(stack));
  C_Stack::kill(stack);

  factory.setSourceDimension(100, 200, 300);
  factory.setSizePolicy(NeuTube::Z_AXIS, ZFlyEmNeuronImageFactory::SIZE_SOURCE);
  stack = factory.createImage(obj);

  ASSERT_EQ(3, C_Stack::width(stack));
  ASSERT_EQ(300, C_Stack::height(stack));
  ASSERT_EQ(1, C_Stack::depth(stack));

  C_Stack::kill(stack);
}

#endif

#endif // ZFLYEMNEURONIMAGEFACTORYTEST_H

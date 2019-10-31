#ifndef ZDVIDSPARSESTACK_TEST_H
#define ZDVIDSPARSESTACK_TEST_H

#include "ztestheader.h"

#include "neutubeconfig.h"
#include "zstack.hxx"
#include "dvid/zdvidsparsestack.h"

#ifdef _USE_GTEST_

TEST(ZDvidSparseStack, loadBody) {
  ZDvidTarget target("127.0.0.1", "4280", 1600);
  target.setGrayScaleName("grayscale");
  target.setSegmentationName("segmentation");

  {
    ZDvidReader reader;
    if (reader.open(target)) {
      ZDvidSparseStack spStack;

      spStack.setDvidTarget(target);
      spStack.setLabel(1536879184);
      spStack.setLabelType(neutu::EBodyLabelType::BODY);
      spStack.loadBody(1536879184);

      if (1) {
        ZStack *stack = spStack.getStack();
        ZIntCuboid box = stack->getBoundBox();
        ASSERT_EQ(ZIntPoint(671, 847, 1043), box.getFirstCorner());
        ASSERT_EQ(ZIntPoint(48, 74, 69), box.getSize());
//        stack->save(GET_TEST_DATA_DIR + "/test.tif");
      }

      if (1) {
        ZDvidReader grayscaleReader;
        ZDvidTarget grayTarget("127.0.0.1", "970f", 1600);
        grayTarget.setGrayScaleName("grayscale");
        if (grayscaleReader.open(grayTarget)) {
          spStack.setGrayscaleReader(grayscaleReader);
        }

        ZStack *stack = spStack.getStack();
        ZIntCuboid box = stack->getBoundBox();
        ASSERT_EQ(ZIntPoint(671, 847, 1043), box.getFirstCorner());
        ASSERT_EQ(ZIntPoint(48, 74, 69), box.getSize());
//        stack->save(GET_TEST_DATA_DIR + "/test2.tif");
      }

      if (1) {
        spStack.setGrayscaleReader(reader);

        ZStack *stack = spStack.getStack();
        ZIntCuboid box = stack->getBoundBox();
        ASSERT_EQ(ZIntPoint(671, 847, 1043), box.getFirstCorner());
        ASSERT_EQ(ZIntPoint(48, 74, 69), box.getSize());
//        stack->save(GET_TEST_DATA_DIR + "/test3.tif");
      }

      {
        ZStack* slice = spStack.makeSlice(1050);
        ZIntCuboid box = slice->getBoundBox();
        ASSERT_EQ(ZIntPoint(673, 897, 1050), box.getFirstCorner());
        ASSERT_EQ(ZIntPoint(36, 23, 1), box.getSize());
        slice->save(GET_TEST_DATA_DIR + "/test.tif");
      }

//      stack->save(GET_TEST_DATA_DIR + "/test.tif");
    }
  }

}

TEST(ZDvidSparseStack, loadBodyAsync) {
  ZDvidTarget target("127.0.0.1", "4280", 1600);
  target.setGrayScaleName("grayscale");
  target.setSegmentationName("segmentation");

  {
    ZDvidReader reader;
    if (reader.open(target)) {
      ZDvidSparseStack spStack;

      spStack.setDvidTarget(target);
      spStack.setLabel(1536879184);
      spStack.setLabelType(neutu::EBodyLabelType::BODY);
      spStack.loadBodyAsync(1536879184);

      if (1) {
        ZStack *stack = spStack.getStack();
        ZIntCuboid box = stack->getBoundBox();
        ASSERT_EQ(ZIntPoint(671, 847, 1043), box.getFirstCorner());
        ASSERT_EQ(ZIntPoint(48, 74, 69), box.getSize());
        stack->save(GET_TEST_DATA_DIR + "/test.tif");
      }
    }
  }
}

#endif

#endif // ZDVIDSPARSESTACK_TEST_H

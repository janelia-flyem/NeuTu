#ifndef ZFLYEMQUALITYCONTROLTEST_H
#define ZFLYEMQUALITYCONTROLTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "flyem/zflyemqualityanalyzer.h"
#include "zintcuboidarray.h"
#include "zswcgenerator.h"
#include "zswctree.h"

#ifdef _USE_GTEST_

TEST(ZFlyEmQualityAnalyzer, isStitchedOrphanBody)
{
  ZObject3dScan obj;
  obj.addSegment(0, 1, 3, 3);
  obj.addSegment(0, 2, 2, 2);
  obj.addSegment(0, 3, 1, 1);

  ZIntCuboidArray roi;
  roi.append(0, 0, 0, 5, 5, 1);
  roi.append(5, 0, 0, 5, 5, 1);

  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(roi);
  EXPECT_FALSE(analyzer.isStitchedOrphanBody(obj));

  roi.clear();
  roi.append(0, 0, -1, 5, 5, 3);
  roi.append(5, 0, -1, 5, 5, 3);
  analyzer.setSubstackRegion(roi);

  obj.addSegment(0, 1, 5, 5);
  obj.addSegment(0, 2, 6, 6);
  EXPECT_TRUE(analyzer.isStitchedOrphanBody(obj));

  obj.addSegment(0, 3, 0, 0);
  EXPECT_FALSE(analyzer.isStitchedOrphanBody(obj));

  obj.clear();
  obj.addSegment(0, 2, 1, 3);
  obj.addSegment(0, 3, 3, 3);
  obj.addSegment(0, 3, 3, 3);
  obj.addSegment(0, 5, 4, 4);
  obj.addSegment(0, 6, 4, 4);
  obj.addSegment(0, 7, 4, 6);

  roi.clear();
  roi.append(0, 0, -1, 5, 5, 3);
  roi.append(3, 5, -1, 5, 5, 3);
  analyzer.setSubstackRegion(roi);
  EXPECT_TRUE(analyzer.isStitchedOrphanBody(obj));

  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(GET_TEST_DATA_DIR + "/benchmark/block.txt");
  EXPECT_EQ(216, (int) blockArray.size());

  Cuboid_I boundBox = blockArray.getBoundBox();

  boundBox.ce[2] = 2999;
  blockArray.intersect(boundBox);
  EXPECT_EQ(54, (int) blockArray.size());

  boundBox = blockArray.getBoundBox();

  EXPECT_EQ(2469, boundBox.cb[0]);
  EXPECT_EQ(2232, boundBox.cb[1]);
  EXPECT_EQ(1500, boundBox.cb[2]);

  //blockArray.print();

  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);

  //blockArray.print();

  boundBox = blockArray.getBoundBox();
  EXPECT_EQ(0, boundBox.cb[0]);
  EXPECT_EQ(0, boundBox.cb[1]);
  EXPECT_EQ(0, boundBox.cb[2]);

  blockArray.translate(0, 0, 10);


  obj.load(GET_TEST_DATA_DIR + "/benchmark/29.sobj");
  analyzer.setSubstackRegion(blockArray);
  //EXPECT_FALSE(analyzer.isStitchedOrphanBody(obj));


  /*
  blockArray.translate(0, 0, -10);
  blockArray.rescale(0.5);
  blockArray.exportSwc(GET_TEST_DATA_DIR + "/flyem/FIB/skeletonization/session9/block.swc");
  */

  roi.clear();
  roi.append(0, 0, 0, 5, 5, 5);
  roi.append(0, 0, 5, 3, 3, 3);

  obj.clear();
  obj.addSegment(2, 1, 1, 1);
  obj.addSegment(3, 1, 1, 1);
  obj.addSegment(4, 1, 1, 1);
  obj.addSegment(5, 1, 1, 1);
  obj.addSegment(6, 1, 1, 1);

  analyzer.setSubstackRegion(roi);
  obj.print();
  ASSERT_TRUE(analyzer.isStitchedOrphanBody(obj));
  ASSERT_FALSE(analyzer.isOrphanBody(obj));

  obj.clear();
  obj.addSegment(2, 1, 1, 2);
  obj.addSegment(3, 1, 1, 2);
  obj.addSegment(4, 1, 1, 2);
  obj.addSegment(5, 1, 1, 2);
  obj.addSegment(6, 1, 1, 2);
  EXPECT_FALSE(analyzer.isStitchedOrphanBody(obj));
}

TEST(ZFlyEmQualityAnalyzer, touchingGlobalBoundary) {
  ZIntCuboidArray blockArray;
  blockArray.append(0, 0, 0, 100, 100, 100);
  blockArray.append(100, 0, 0, 100, 100, 100);
  blockArray.append(0, 100, 0, 100, 100, 100);
  //blockArray.append(100, 100, 0, 100, 100, 100);
  blockArray.append(50, 50, 100, 100, 100, 100);

  //blockArray.exportSwc(GET_TEST_DATA_DIR + "/test.swc");

  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray);

#if 0
  ZIntCuboidFaceArray faceArray = blockArray.getBorderFace();
  ZSwcTree *tree = ZSwcGenerator::createSwc(faceArray, 3.0);
  tree->save(GET_TEST_DATA_DIR + "/test.swc");
#endif

  ZObject3dScan obj;
  obj.addSegment(0, 0, 0, 0);
  ASSERT_TRUE(analyzer.touchingGlobalBoundary(obj));

  obj.clear();
  obj.addSegment(1, 1, 1, 1);
  ASSERT_FALSE(analyzer.touchingGlobalBoundary(obj));

  obj.addSegment(1, 99, 99, 99);
  ASSERT_FALSE(analyzer.touchingGlobalBoundary(obj));

  obj.addSegment(1, 99, 100, 100);
  ASSERT_TRUE(analyzer.touchingGlobalBoundary(obj));

  obj.clear();
  obj.addSegment(99, 99, 99, 99);
  ASSERT_FALSE(analyzer.touchingGlobalBoundary(obj));

  obj.addSegment(100, 100, 99, 99);
  ASSERT_FALSE(analyzer.touchingGlobalBoundary(obj));

  obj.addSegment(99, 100, 99, 99);
  ASSERT_TRUE(analyzer.touchingGlobalBoundary(obj));

  obj.clear();
  obj.addSegment(100, 100, 100, 100);
  ASSERT_TRUE(analyzer.touchingGlobalBoundary(obj));

  obj.clear();
  obj.addSegment(100, 10, 10, 10);
  ASSERT_FALSE(analyzer.touchingGlobalBoundary(obj));

  obj.clear();
  obj.addSegment(99, 10, 10, 10);
  ASSERT_TRUE(analyzer.touchingGlobalBoundary(obj));

  obj.clear();
  obj.addSegment(10, 10, 10, 150);
  //obj.print();
  ASSERT_FALSE(analyzer.touchingGlobalBoundary(obj));

  obj.addSegment(99, 10, 10, 150);
  ASSERT_TRUE(analyzer.touchingGlobalBoundary(obj));

  obj.clear();
  obj.addSegment(0, 1, 1, 1);
  ASSERT_TRUE(analyzer.touchingGlobalBoundary(obj));

  obj.clear();
  obj.addSegment(200, 50, 50, 50);
  ASSERT_FALSE(analyzer.touchingGlobalBoundary(obj));

  obj.clear();
  obj.addSegment(199, 50, 50, 50);
  ASSERT_TRUE(analyzer.touchingGlobalBoundary(obj));
}

TEST(ZFlyEmQualityAnalyzer, touchingSideBoundary) {
  ZIntCuboidArray blockArray;
  blockArray.append(0, 0, 0, 100, 100, 100);
  blockArray.append(100, 0, 0, 100, 100, 100);
  blockArray.append(0, 100, 0, 100, 100, 100);
  //blockArray.append(100, 100, 0, 100, 100, 100);
  blockArray.append(50, 50, 100, 100, 100, 100);

  //blockArray.exportSwc(GET_TEST_DATA_DIR + "/test.swc");

  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray);

  ZObject3dScan obj;
  obj.addSegment(0, 0, 0, 0);
  ASSERT_TRUE(analyzer.touchingSideBoundary(obj));

  obj.clear();
  obj.addSegment(1, 1, 1, 1);
  ASSERT_FALSE(analyzer.touchingSideBoundary(obj));

  obj.addSegment(1, 99, 99, 99);
  ASSERT_FALSE(analyzer.touchingSideBoundary(obj));

  obj.addSegment(1, 99, 100, 100);
  ASSERT_TRUE(analyzer.touchingSideBoundary(obj));

  obj.clear();
  obj.addSegment(99, 99, 99, 99);
  ASSERT_FALSE(analyzer.touchingSideBoundary(obj));

  obj.addSegment(100, 100, 99, 99);
  ASSERT_FALSE(analyzer.touchingSideBoundary(obj));

  obj.addSegment(99, 100, 99, 99);
  ASSERT_TRUE(analyzer.touchingSideBoundary(obj));

  obj.clear();
  obj.addSegment(100, 100, 100, 100);
  ASSERT_TRUE(analyzer.touchingSideBoundary(obj));

  obj.clear();
  obj.addSegment(100, 10, 10, 10);
  ASSERT_TRUE(analyzer.touchingSideBoundary(obj));

  obj.clear();
  obj.addSegment(10, 10, 10, 150);
  //obj.print();
  ASSERT_FALSE(analyzer.touchingSideBoundary(obj));

  obj.addSegment(99, 10, 10, 150);
  ASSERT_TRUE(analyzer.touchingSideBoundary(obj));

  obj.clear();
  obj.addSegment(0, 1, 1, 1);
  ASSERT_FALSE(analyzer.touchingSideBoundary(obj));

  obj.clear();
  obj.addSegment(200, 50, 50, 50);
  ASSERT_FALSE(analyzer.touchingSideBoundary(obj));
}


#endif

#endif // ZFLYEMQUALITYCONTROLTEST_H

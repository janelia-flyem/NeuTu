#ifndef ZCUBOIDTEST_H
#define ZCUBOIDTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zintcuboidarray.h"
#if defined(_FLYEM_)
#include "flyem/zintcuboidcomposition.h"
#endif
#include "zcuboid.h"
#include "zintcuboidface.h"
#include "zsharedpointer.h"

#ifdef _USE_GTEST_

TEST(ZIntCuboidArray, basic)
{
  ZIntCuboidArray blockArray;

  blockArray.append(0, 0, 0, 500, 500, 500);
  blockArray.append(500, 500, 500, 500, 500, 500);

  EXPECT_EQ(2, (int) blockArray.size());

  int index = blockArray.hitTest(1, 1, 1);
  EXPECT_EQ(0, index);

  blockArray.loadSubstackList(GET_TEST_DATA_DIR + "/benchmark/flyem/block.txt");
  EXPECT_EQ(216, (int) blockArray.size());

  for (size_t i = 0; i < blockArray.size(); ++i) {
    int width, height, depth;
    Cuboid_I_Size(&(blockArray[i]), &width, &height, &depth);
    EXPECT_EQ(500, width);
    EXPECT_EQ(500, height);
    EXPECT_EQ(500, depth);
  }

  blockArray.translate(100, 200, 300);
  for (size_t i = 0; i < blockArray.size(); ++i) {
    int width, height, depth;
    Cuboid_I_Size(&(blockArray[i]), &width, &height, &depth);
    EXPECT_EQ(500, width);
    EXPECT_EQ(500, height);
    EXPECT_EQ(500, depth);
  }
}

TEST(ZIntCuboidArray, boundBox)
{
  ZIntCuboidArray blockArray;

  blockArray.append(10, 20, 30, 10, 10, 10);
  Cuboid_I boundBox = blockArray.getBoundBox();
  EXPECT_EQ(10, boundBox.cb[0]);
  EXPECT_EQ(20, boundBox.cb[1]);
  EXPECT_EQ(30, boundBox.cb[2]);
  EXPECT_EQ(19, boundBox.ce[0]);
  EXPECT_EQ(29, boundBox.ce[1]);
  EXPECT_EQ(39, boundBox.ce[2]);

  blockArray.append(20, 30, 40, 10, 10, 10);
  boundBox = blockArray.getBoundBox();
  EXPECT_EQ(10, boundBox.cb[0]);
  EXPECT_EQ(20, boundBox.cb[1]);
  EXPECT_EQ(30, boundBox.cb[2]);
  EXPECT_EQ(29, boundBox.ce[0]);
  EXPECT_EQ(39, boundBox.ce[1]);
  EXPECT_EQ(49, boundBox.ce[2]);
}

TEST(ZIntCuboidArray, exportSwc)
{
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(GET_TEST_DATA_DIR + "/benchmark/flyem/block.txt");


  //blockArray.exportSwc(GET_TEST_DATA_DIR + "/test.swc");
}

TEST(ZIntCuboidArray, removeInvalidCublid)
{
  ZIntCuboidArray blockArray;
  blockArray.append(1, 1, 1, 0, 0, 0);
  blockArray.removeInvalidCuboid();
  EXPECT_EQ(0, (int) blockArray.size());

  blockArray.append(0, 0, 0, 1, 1, 1);
  blockArray.append(0, 0, 0, 1, 1, 1);
  blockArray.append(1, 1, 1, 0, 0, 0);
  blockArray.append(1, 1, 1, 0, 0, 0);
  blockArray.append(0, 0, 0, 1, 1, 1);
  blockArray.append(0, 0, 0, 1, 1, 1);
  blockArray.removeInvalidCuboid();
  EXPECT_EQ(4, (int) blockArray.size());
}

TEST(ZIntCuboidArray, range)
{
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(GET_TEST_DATA_DIR + "/benchmark/flyem/block.txt");

  Cuboid_I boundBox = blockArray.getBoundBox();
  //std::cout << blockArray.size() << std::endl;

  boundBox.ce[2] = 4499;
  blockArray.intersect(boundBox);
  //blockArray.print();
  EXPECT_EQ(108, (int) blockArray.size());

  boundBox.ce[2] = 2999;
  blockArray.intersect(boundBox);
  EXPECT_EQ(54, (int) blockArray.size());

  boundBox = blockArray.getBoundBox();
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);

  boundBox = blockArray.getBoundBox();
  EXPECT_EQ(0, boundBox.cb[0]);
  EXPECT_EQ(0, boundBox.cb[1]);
  EXPECT_EQ(0, boundBox.cb[2]);
}

TEST(ZIntCuboidArray, face)
{
  ZIntCuboidArray blockArray;
  blockArray.append(0, 0, 0, 3, 3, 3);
  ZIntCuboidArray face = blockArray.getFace();

  EXPECT_EQ(6, (int) face.size());

  ZIntCuboidArray face2 = blockArray.getInnerFace();
  EXPECT_TRUE(face2.empty());

  blockArray.append(3, 0, 0, 3, 3, 3);
  face = blockArray.getFace();
  EXPECT_EQ(12, (int) face.size());

  face2 = blockArray.getInnerFace();
  //face2.print();
  EXPECT_EQ(2, (int) face2.size());
  Cuboid_I cuboid = face2[0];
  EXPECT_EQ(3, cuboid.cb[0]);
  EXPECT_EQ(3, cuboid.ce[0]);
  EXPECT_EQ(0, cuboid.cb[1]);
  EXPECT_EQ(2, cuboid.ce[1]);
  EXPECT_EQ(0, cuboid.cb[2]);
  EXPECT_EQ(2, cuboid.ce[2]);

  blockArray.append(1, 3, 0, 4, 3, 3);
  face = blockArray.getFace();
  EXPECT_EQ(18, (int) face.size());

  face2 = blockArray.getInnerFace();
  EXPECT_EQ(6, (int) face2.size());
}
#if defined(_FLYEM_)
TEST(ZIntCuboidComposition, hitTest)
{
  FlyEm::ZIntCuboidComposition cuboid;
  cuboid.setSingular(0, 0, 0, 3, 3, 3);

  EXPECT_TRUE(cuboid.hitTest(0, 0, 0));
  EXPECT_TRUE(cuboid.hitTest(1, 1, 1));
  EXPECT_TRUE(cuboid.hitTest(2, 2, 2));
  EXPECT_FALSE(cuboid.hitTest(3, 3, 3));

  ZSharedPointer<FlyEm::ZIntCuboidComposition> comp1(
        new FlyEm::ZIntCuboidComposition);
  comp1->setSingular(0, 0, 0, 3, 3, 3);

  ZSharedPointer<FlyEm::ZIntCuboidComposition> comp2(
        new FlyEm::ZIntCuboidComposition);
  comp2->setSingular(0, 0, 0, 3, 3, 3);

  ZSharedPointer<FlyEm::ZIntCuboidComposition> comp3(
        new FlyEm::ZIntCuboidComposition);
  comp3->setComposition(comp1, comp2, FlyEm::ZIntCuboidComposition::OR);
  EXPECT_TRUE(comp3->hitTest(0, 0, 0));
  EXPECT_TRUE(comp3->hitTest(1, 1, 1));
  EXPECT_TRUE(comp3->hitTest(2, 2, 2));
  EXPECT_FALSE(comp3->hitTest(3, 3, 3));

  comp3->setComposition(comp1, comp2, FlyEm::ZIntCuboidComposition::AND);
  EXPECT_TRUE(comp3->hitTest(0, 0, 0));
  EXPECT_TRUE(comp3->hitTest(1, 1, 1));
  EXPECT_TRUE(comp3->hitTest(2, 2, 2));
  EXPECT_FALSE(comp3->hitTest(3, 3, 3));

  comp3->setComposition(comp1, comp2, FlyEm::ZIntCuboidComposition::XOR);
  EXPECT_FALSE(comp3->hitTest(0, 0, 0));
  EXPECT_FALSE(comp3->hitTest(1, 1, 1));
  EXPECT_FALSE(comp3->hitTest(2, 2, 2));
  EXPECT_FALSE(comp3->hitTest(3, 3, 3));
}
#endif

TEST(ZCuboid, distance) {
  ZCuboid box1;
  box1.set(0, 0, 0, 10, 20, 30);
  ZCuboid box2;
  box2.set(10, 20, 40, 20, 30, 50);

  ASSERT_DOUBLE_EQ(box1.computeDistance(box2), 10.0);

  box2.set(10, 20, 30, 20, 30, 50);
  ASSERT_DOUBLE_EQ(box1.computeDistance(box2), 0.0);
}

TEST(ZIntCuboidFace, basic)
{
  ZIntCuboidFaceArray faceArray;

  ZIntCuboidFace face;
  //face.print();
  ASSERT_EQ(0, face.getLowerBound(0));
  faceArray.appendValid(face);

  face.set(ZIntCuboidFace::Corner(10, 20), ZIntCuboidFace::Corner(40, 80));
  ASSERT_EQ(10, face.getLowerBound(0));
  //face.print();
  faceArray.appendValid(face);
  ASSERT_EQ(2, (int) faceArray.size());

  face.set(10, 20, 40, 80);
  //face.print();

  faceArray.appendValid(face);

  //faceArray.print();

  ASSERT_FALSE(faceArray[0].hasOverlap(faceArray[1]));
  ASSERT_TRUE(faceArray[1].hasOverlap(faceArray[2]));

  ZIntCuboidFaceArray faceArray2 = faceArray[0].cropBy(faceArray[1]);
  //faceArray2.print();

  faceArray2 = faceArray[1].cropBy(faceArray[0]);
  //faceArray2.print();

  faceArray2 = faceArray[1].cropBy(faceArray[2]);
  //faceArray2.print();

  ASSERT_TRUE(faceArray[1].isWithin(faceArray[2]));

  face.set(20, 30, 100, 90);
  faceArray.appendValid(face);
  //faceArray.print();

  faceArray2 = faceArray[2].cropBy(faceArray[3]);
  //faceArray2.print();

  face.set(20, 30, 30, 60);
  faceArray.appendValid(face);
  faceArray2 = faceArray[2].cropBy(faceArray[4]);
  //faceArray2.print();
}

TEST(ZIntCuboidFace, dist)
{
  ZIntCuboidFace face;
  face.set(10, 20, 40, 80);
  //face.print();

  ASSERT_DOUBLE_EQ(0.0, face.computeDistance(10, 20, 0));
  ASSERT_TRUE(face.contains(10, 20, 0));

  ASSERT_DOUBLE_EQ(10.0, face.computeDistance(10, 20, 10));
  ASSERT_FALSE(face.contains(10, 20, 10));
  ASSERT_DOUBLE_EQ(10.0, face.computeDistance(10, 10, 0));
  ASSERT_DOUBLE_EQ(10.0, face.computeDistance(0, 20, 0));
  ASSERT_DOUBLE_EQ(sqrt(300.0), face.computeDistance(0, 10, 10));
  ASSERT_DOUBLE_EQ(sqrt(300.0), face.computeDistance(50, 90, -10));

  face.setNormal(NeuTube::X_AXIS);
  ASSERT_DOUBLE_EQ(0.0, face.computeDistance(0, 10, 20));
  ASSERT_DOUBLE_EQ(10.0, face.computeDistance(10, 10, 20));
  ASSERT_DOUBLE_EQ(10.0, face.computeDistance(0, 10, 10));
  ASSERT_DOUBLE_EQ(10.0, face.computeDistance(0, 0, 20));
  ASSERT_DOUBLE_EQ(sqrt(300.0), face.computeDistance(10, 0, 10));
  ASSERT_DOUBLE_EQ(sqrt(300.0), face.computeDistance(-10, 50, 90));

  ZIntCuboidFaceArray faceArray;
  Cuboid_I cuboid;
  Cuboid_I_Set_S(&cuboid, 10, 20, 30, 40, 50, 60);
  faceArray.append(&cuboid);
  //faceArray.print();
  ASSERT_TRUE(faceArray.contains(10, 20, 30));
  ASSERT_FALSE(faceArray.contains(40, 50, 60));
  ASSERT_TRUE(faceArray.contains(49, 69, 89));
  ASSERT_TRUE(faceArray.contains(15, 25, 30));
  ASSERT_TRUE(faceArray.contains(25, 69, 50));
  ASSERT_FALSE(faceArray.contains(25, 69, 90));
  ASSERT_TRUE(faceArray.contains(25, 69, 89));
  ASSERT_FALSE(faceArray.contains(25, 59, 79));
}

TEST(ZIntCuboidFaceArray, basic)
{
  ZIntCuboidFaceArray faceArray;
  Cuboid_I cuboid;
  Cuboid_I_Set_S(&cuboid, 10, 20, 30, 40, 50, 60);
  faceArray.append(&cuboid);
  ASSERT_EQ(6, (int) faceArray.size());
  //faceArray.print();

  ZIntCuboidFaceArray faceArray2;
  Cuboid_I_Set_S(&cuboid, 50, 30, 40, 40, 50, 60);
  faceArray2.append(&cuboid);
  ASSERT_EQ(6, (int) faceArray2.size());
  //faceArray2.print();

  for (ZIntCuboidFaceArray::iterator iter = faceArray2.begin();
       iter != faceArray2.end(); ++iter) {
    ZIntCuboidFace &face = *iter;
    face.moveBackward(1);
  }

  ZIntCuboidFaceArray faceArray3;
  for (ZIntCuboidFaceArray::const_iterator iter = faceArray.begin();
       iter != faceArray.end(); ++iter) {
    const ZIntCuboidFace &face = *iter;
    faceArray3.append(face.cropBy(faceArray2));
  }

  faceArray3.print();

  ASSERT_EQ(7, (int) faceArray3.size());
}

TEST(ZIntCuboid, hasOverlap)
{
  ZIntCuboid box;
  box.setFirstCorner(ZIntPoint(0, 1, 2));
  box.setLastCorner(ZIntPoint(4, 6, 9));

  ZIntCuboid box2;
  box2.setFirstCorner(ZIntPoint(0, 1, 2));
  box2.setLastCorner(ZIntPoint(4, 6, 9));
  ASSERT_TRUE(box.hasOverlap(box2));
  ASSERT_TRUE(box2.hasOverlap(box));

  box2.setFirstCorner(ZIntPoint(0, 1, 2));
  box2.setLastCorner(ZIntPoint(4, 6, 9));
  ASSERT_TRUE(box.hasOverlap(box2));
  ASSERT_TRUE(box2.hasOverlap(box));

  box2.setFirstCorner(ZIntPoint(0, 1, 2));
  box2.setLastCorner(ZIntPoint(3, 4, 5));
  ASSERT_TRUE(box.hasOverlap(box2));
  ASSERT_TRUE(box2.hasOverlap(box));

  box2.setFirstCorner(ZIntPoint(0, 0, 0));
  box2.setLastCorner(ZIntPoint(3, 4, 9));
  ASSERT_TRUE(box.hasOverlap(box2));
  ASSERT_TRUE(box2.hasOverlap(box));

  box2.setFirstCorner(ZIntPoint(1, 0, 0));
  box2.setLastCorner(ZIntPoint(3, 4, 9));
  ASSERT_TRUE(box.hasOverlap(box2));
  ASSERT_TRUE(box2.hasOverlap(box));

  box2.setFirstCorner(ZIntPoint(-2, -3, -5));
  box2.setLastCorner(ZIntPoint(-1, 0, 0));
  ASSERT_FALSE(box.hasOverlap(box2));
  ASSERT_FALSE(box2.hasOverlap(box));

  box.setFirstCorner(ZIntPoint(5, 6, 7));
  box.setLastCorner(ZIntPoint(10, 12, 14));

  box2.setFirstCorner(ZIntPoint(5, 6, 7) + ZIntPoint(1, 0, 0));
  box2.setLastCorner(ZIntPoint(10, 12, 14) + ZIntPoint(1, 0, 0));
  ASSERT_TRUE(box.hasOverlap(box2));
  ASSERT_TRUE(box2.hasOverlap(box));

  box2.setFirstCorner(ZIntPoint(5, 6, 7) + ZIntPoint(10, 0, 0));
  box2.setLastCorner(ZIntPoint(10, 12, 14) + ZIntPoint(10, 0, 0));
  ASSERT_FALSE(box.hasOverlap(box2));
  ASSERT_FALSE(box2.hasOverlap(box));

  box2.setFirstCorner(ZIntPoint(5, 6, 7) + ZIntPoint(0, 1, 0));
  box2.setLastCorner(ZIntPoint(10, 12, 14) + ZIntPoint(0, 1, 0));
  ASSERT_TRUE(box.hasOverlap(box2));
  ASSERT_TRUE(box2.hasOverlap(box));

  box2.setFirstCorner(ZIntPoint(5, 6, 7) + ZIntPoint(0, 10, 0));
  box2.setLastCorner(ZIntPoint(10, 12, 14) + ZIntPoint(0, 10, 0));
  ASSERT_FALSE(box.hasOverlap(box2));
  ASSERT_FALSE(box2.hasOverlap(box));

  box2.setFirstCorner(ZIntPoint(5, 6, 8));
  box2.setLastCorner(ZIntPoint(10, 12, 15));
  ASSERT_TRUE(box.hasOverlap(box2));
  ASSERT_TRUE(box2.hasOverlap(box));

  box2.setFirstCorner(ZIntPoint(5, 6, 17));
  box2.setLastCorner(ZIntPoint(10, 12, 24));
  ASSERT_FALSE(box.hasOverlap(box2));
  ASSERT_FALSE(box2.hasOverlap(box));

  box2.setFirstCorner(ZIntPoint(5, 6, 7) + ZIntPoint(1, 1, 1));
  box2.setLastCorner(ZIntPoint(10, 12, 14) + ZIntPoint(2, 2, 2));
  ASSERT_TRUE(box.hasOverlap(box2));
  ASSERT_TRUE(box2.hasOverlap(box));

  box2.setFirstCorner(ZIntPoint(5, 6, 7) + ZIntPoint(-1, -1, -1));
  box2.setLastCorner(ZIntPoint(10, 12, 14) + ZIntPoint(2, -2, -2));
  ASSERT_TRUE(box.hasOverlap(box2));
  ASSERT_TRUE(box2.hasOverlap(box));

  box.setFirstCorner(ZIntPoint(231, 150, 291));
  box.setLastCorner(ZIntPoint(246, 158, 291));

  box2.setFirstCorner(ZIntPoint(232, 144, 291));
  box2.setLastCorner(ZIntPoint(242, 159, 291));
  ASSERT_TRUE(box.hasOverlap(box2));
  ASSERT_TRUE(box2.hasOverlap(box));
}

TEST(ZCuboid, intersectLine)
{
  ZCuboid cuboid(0, 0, 0, 0, 0, 0);
  ZLineSegment seg;
  ASSERT_FALSE(cuboid.intersectLine(ZPoint(0, 0, 0), ZPoint(0, 0, 0), NULL));
  ASSERT_FALSE(cuboid.intersectLine(ZPoint(1, 0, 0), ZPoint(1, 1, 1), NULL));
  ASSERT_FALSE(cuboid.intersectLine(ZPoint(1, 0, 0), ZPoint(0, 1, 0), NULL));
  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0, 0), ZPoint(1, 0, 0), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().z());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0, 0), ZPoint(0, 1, 0), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().z());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0, 0), ZPoint(0, 0, 1), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().z());


  cuboid.set(0, 0, 0, 1, 1, 1);
  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0, 0), ZPoint(1, 0, 0), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().z());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0, 0), ZPoint(0, 1, 0), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().z());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0, 0), ZPoint(0, 0, 1), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().z());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0.5, 0.6), ZPoint(1, 0, 0), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0.5, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0.6, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(0.5, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(0.6, seg.getEndPoint().z());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0.5, 10, 0.6), ZPoint(0, 1, 0), &seg));
  ASSERT_DOUBLE_EQ(0.5, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0.6, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0.5, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(0.6, seg.getEndPoint().z());


  cuboid.set(0.5, 0, 0, 1, 1, 1);
  ASSERT_TRUE(cuboid.intersectLine(ZPoint(10, 0.5, 0.5), ZPoint(1, 0, 0), &seg));
  ASSERT_DOUBLE_EQ(0.5, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0.5, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0.5, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(0.5, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(0.5, seg.getEndPoint().z());

  ASSERT_FALSE(cuboid.intersectLine(ZPoint(0, 2.5, 0.5), ZPoint(1, 0, 0), &seg));

  cuboid.set(0, 0, 0, 1, 1, 1);
  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0, 0), ZPoint(1, 1, 0), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().z());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0.5, 0, 0), ZPoint(1, 1, 0), &seg));
  ASSERT_DOUBLE_EQ(1, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0.5, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0.5, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().z());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0.5, 0), ZPoint(1, 1, 0), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0.5, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0.5, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().z());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0.2, 0), ZPoint(0.5, 0.5, 0), &seg));
  //seg.print();
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0.2, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().z());


  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0.3, 0.2, 0), ZPoint(0.5, 0.5, 0), &seg));
  //seg.print();
  ASSERT_DOUBLE_EQ(1, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().z());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0.3, 0.2, 0.5), ZPoint(0.1, 0.5, 0.0), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0.5, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(0.5, seg.getEndPoint().z());
  //seg.print();

  cuboid.set(0, 0, 0, 1, 1, 1);
  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0, 0), ZPoint(1, 0, 1), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().z());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0.5, 0, 0), ZPoint(1, 0, 1), &seg));
  ASSERT_DOUBLE_EQ(1, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0.5, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0.5, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().y());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0, 0.5), ZPoint(1, 0, 1), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0.5, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0.5, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().y());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0, 0.2), ZPoint(0.5, 0, 0.5), &seg));
  //seg.print();
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0.2, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().y());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0.3, 0, 0.2), ZPoint(0.5, 0, 0.5), &seg));
  //seg.print();
  ASSERT_DOUBLE_EQ(1, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().y());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0.3, 0.5, 0.2), ZPoint(0.1, 0.0, 0.5), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0.5, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().z());
  ASSERT_DOUBLE_EQ(0.5, seg.getEndPoint().y());
  //seg.print();

  cuboid.set(0, 0, 0, 1, 1, 1);
  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0, 0), ZPoint(1, 0, 1), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().z());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0.5, 0, 0), ZPoint(1, 0, 1), &seg));
  ASSERT_DOUBLE_EQ(1, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0.5, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0.5, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().y());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0, 0.5), ZPoint(1, 0, 1), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0.5, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0.5, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().y());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0, 0.2), ZPoint(0.5, 0, 0.5), &seg));
  //seg.print();
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0.2, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().y());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0.3, 0, 0.2), ZPoint(0.5, 0, 0.5), &seg));
  //seg.print();
  ASSERT_DOUBLE_EQ(1, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().y());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0.3, 0.5, 0.2), ZPoint(0.1, 0.0, 0.5), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0.5, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().z());
  ASSERT_DOUBLE_EQ(0.5, seg.getEndPoint().y());
  //seg.print();

  //Y-Z
  cuboid.set(0, 0, 0, 1, 1, 1);
  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0, 0), ZPoint(0, 1, 1), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().z());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0.5, 0), ZPoint(0, 1, 1), &seg));
  ASSERT_DOUBLE_EQ(1, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0.5, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0.5, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().x());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0, 0.5), ZPoint(0, 1, 1), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0.5, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0.5, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().x());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0, 0.2), ZPoint(0, 0.5, 0.5), &seg));
  //seg.print();
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0.2, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().x());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0.3, 0.2), ZPoint(0, 0.5, 0.5), &seg));
  //seg.print();
  ASSERT_DOUBLE_EQ(1, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().z());
  ASSERT_DOUBLE_EQ(0, seg.getEndPoint().x());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0.5, 0.3, 0.2), ZPoint(0.0, 0.1, 0.5), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(0.5, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().z());
  ASSERT_DOUBLE_EQ(0.5, seg.getEndPoint().x());
  //seg.print();

  cuboid.set(0, 0, 0, 1, 1, 1);
  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0, 0, 0), ZPoint(1, 1, 1), &seg));
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().x());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().y());
  ASSERT_DOUBLE_EQ(0, seg.getStartPoint().z());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().y());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().x());
  ASSERT_DOUBLE_EQ(1, seg.getEndPoint().z());

  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0.1, 0.2, 0.3), ZPoint(1, 1, 1), &seg));
  ASSERT_TRUE(cuboid.intersectLine(ZPoint(0.1, 0.2, 0.3), ZPoint(0.1, 10, 3), &seg));

  for (double z = 0.0; z <= 1.0; z += 0.1) {
    for (double y = 0.0; y <= 1.0; y += 0.1) {
      for (double x = 0.0; x <= 1.0; x += 0.1) {
        for (double nx = 0.0; nx < 10.0; nx += 1.0) {
          ASSERT_TRUE(cuboid.intersectLine(ZPoint(x, y, z ),
                                           ZPoint(nx, 10, 3), &seg));
        }
      }
    }
  }

  std::cout << "v2" << std::endl;
}

#endif

#endif // ZCUBOIDTEST_H

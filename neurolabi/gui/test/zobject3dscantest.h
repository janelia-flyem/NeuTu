#ifndef ZOBJECT3DSCANTEST_H
#define ZOBJECT3DSCANTEST_H

#include "ztestheader.h"
#include "zobject3dscan.h"
#include "neutubeconfig.h"
#include "zgraph.h"
#include "tz_iarray.h"
#include "zdebug.h"
#include "zdoublevector.h"
#include "zstack.hxx"
#include "zstackfactory.h"

#ifdef _USE_GTEST_

static void createStripe(ZObject3dStripe *stripe)
{
  stripe->clearSegment();
  stripe->setY(3);
  stripe->setZ(5);
  stripe->addSegment(0, 1);
  stripe->addSegment(3, 5);
}

static void createStripe2(ZObject3dStripe *stripe)
{
  stripe->clearSegment();
  stripe->setY(3);
  stripe->setZ(5);
  stripe->addSegment(3, 5, false);
  stripe->addSegment(0, 1, false);
  stripe->addSegment(3, 1, false);
}

static void createStripe3(ZObject3dStripe *stripe)
{
  stripe->clearSegment();
  stripe->setY(3);
  stripe->setZ(5);
  stripe->addSegment(3, 5);
  stripe->addSegment(0, 1);
  stripe->addSegment(3, 1);
}

static void createStripe4(ZObject3dStripe *stripe)
{
  stripe->clearSegment();
  stripe->setY(3);
  stripe->setZ(5);
  stripe->addSegment(3, 5);
  stripe->addSegment(0, 1);
  stripe->addSegment(3, 1, false);
}

TEST(ZObject3dStripe, TestGetProperty) {
  ZObject3dStripe stripe;
  createStripe(&stripe);
  EXPECT_EQ(stripe.getMinX(), 0);
  EXPECT_EQ(stripe.getMaxX(), 5);
  EXPECT_EQ(stripe.getSegmentNumber(), 2);
  EXPECT_EQ((int) stripe.getSize(), 2);
  EXPECT_EQ(stripe.getY(), 3);
  EXPECT_EQ(stripe.getZ(), 5);
  EXPECT_EQ((int) stripe.getVoxelNumber(), 5);

  createStripe2(&stripe);
  EXPECT_EQ(stripe.getSegmentNumber(), 2);
  EXPECT_EQ((int) stripe.getSize(), 2);
  EXPECT_EQ(stripe.getY(), 3);
  EXPECT_EQ(stripe.getZ(), 5);
  EXPECT_EQ((int) stripe.getVoxelNumber(), 7);

  stripe.canonize();
  EXPECT_EQ(stripe.getMinX(), 0);
  EXPECT_EQ(stripe.getMaxX(), 5);
  EXPECT_EQ(stripe.getSegmentNumber(), 1);
  EXPECT_EQ((int) stripe.getSize(), 1);
  EXPECT_EQ(stripe.getY(), 3);
  EXPECT_EQ(stripe.getZ(), 5);
  EXPECT_EQ((int) stripe.getVoxelNumber(), 6);

  createStripe3(&stripe);
  EXPECT_EQ(stripe.getMinX(), 0);
  EXPECT_EQ(stripe.getMaxX(), 5);
  EXPECT_EQ(stripe.getSegmentNumber(), 1);
  EXPECT_EQ((int) stripe.getSize(), 1);
  EXPECT_EQ(stripe.getY(), 3);
  EXPECT_EQ(stripe.getZ(), 5);
  EXPECT_EQ((int) stripe.getVoxelNumber(), 6);

  createStripe4(&stripe);
  EXPECT_EQ(stripe.getSegmentNumber(), 3);
  EXPECT_EQ((int) stripe.getSize(), 3);
  EXPECT_EQ(stripe.getY(), 3);
  EXPECT_EQ(stripe.getZ(), 5);

  stripe.canonize();
  EXPECT_EQ(stripe.getMinX(), 0);
  EXPECT_EQ(stripe.getMaxX(), 5);
  EXPECT_EQ(stripe.getSegmentNumber(), 1);
  EXPECT_EQ((int) stripe.getSize(), 1);
  EXPECT_EQ(stripe.getY(), 3);
  EXPECT_EQ(stripe.getZ(), 5);
  EXPECT_EQ((int) stripe.getVoxelNumber(), 6);
}

TEST(ZObject3dStripe, TestUnify) {
  ZObject3dStripe stripe;
  stripe.setY(3);
  stripe.setZ(5);
  stripe.addSegment(3, 5);

  ZObject3dStripe stripe2;
  stripe2.setY(3);
  stripe2.setZ(5);
  stripe2.addSegment(6, 7);

  EXPECT_FALSE(stripe.equalsLiterally(stripe2));

  EXPECT_TRUE(stripe.unify(stripe2));
  EXPECT_EQ(1, stripe.getSegmentNumber());
  EXPECT_EQ(3, stripe.getMinX());
  EXPECT_EQ(7, stripe.getMaxX());
  EXPECT_EQ(5, (int) stripe.getVoxelNumber());

  stripe2.setY(4);
  EXPECT_FALSE(stripe.unify(stripe2));
  EXPECT_EQ(1, stripe.getSegmentNumber());
  EXPECT_EQ(3, stripe.getMinX());
  EXPECT_EQ(7, stripe.getMaxX());
  EXPECT_EQ(5, (int) stripe.getVoxelNumber());

  stripe2.setY(3);
  stripe2.setZ(4);
  EXPECT_FALSE(stripe.unify(stripe2));
  EXPECT_EQ(1, stripe.getSegmentNumber());
  EXPECT_EQ(3, stripe.getMinX());
  EXPECT_EQ(7, stripe.getMaxX());
  EXPECT_EQ(5, (int) stripe.getVoxelNumber());

  stripe2.clearSegment();
  stripe2.setY(3);
  stripe2.setZ(5);
  stripe2.addSegment(1, 7);
  EXPECT_TRUE(stripe.unify(stripe2));
  EXPECT_EQ(1, stripe.getSegmentNumber());
  EXPECT_EQ(1, stripe.getMinX());
  EXPECT_EQ(7, stripe.getMaxX());
  EXPECT_EQ(7, (int) stripe.getVoxelNumber());

  stripe2.clearSegment();
  stripe2.setY(3);
  stripe2.setZ(5);
  stripe2.addSegment(9, 10);
  EXPECT_TRUE(stripe.unify(stripe2));
  EXPECT_EQ(2, stripe.getSegmentNumber());
  EXPECT_EQ(1, stripe.getMinX());
  EXPECT_EQ(10, stripe.getMaxX());
  EXPECT_EQ(9, (int) stripe.getVoxelNumber());
}

TEST(ZObject3dStripe, TestIO) {
  FILE *fp = fopen((GET_TEST_DATA_DIR + "/test.sobj").c_str(), "w");
  ZObject3dStripe stripe;
  createStripe2(&stripe);
  stripe.write(fp);

  fclose(fp);

  fp = fopen((GET_TEST_DATA_DIR + "/test.sobj").c_str(), "r");
  ZObject3dStripe stripe2;
  stripe2.read(fp);
  fclose(fp);

  EXPECT_TRUE(stripe.equalsLiterally(stripe));
  EXPECT_TRUE(stripe.equalsLiterally(stripe2));
}

bool isSorted(const ZObject3dStripe &stripe)
{
  if (!stripe.isEmpty()) {
    for (int i = 0; i < stripe.getSegmentNumber() - 1; ++i) {
      if (stripe.getSegmentStart(i) > stripe.getSegmentStart(i + 1)) {
        return false;
      } else if (stripe.getSegmentStart(i) == stripe.getSegmentStart(i + 1)) {
        if (stripe.getSegmentEnd(i) > stripe.getSegmentEnd(i + 1)) {
          return false;
        }
      }
    }
  }

  return true;
}

TEST(ZObject3dStripe, TestSort) {
  ZObject3dStripe stripe;
  createStripe2(&stripe);
  EXPECT_FALSE(isSorted(stripe));

  stripe.sort();
  EXPECT_TRUE(isSorted(stripe));

  stripe.clearSegment();
  stripe.addSegment(1, 2);
  stripe.addSegment(4, 5, false);
  stripe.addSegment(1, 2, false);
  stripe.addSegment(3, 5, false);
  EXPECT_FALSE(isSorted(stripe));

  stripe.sort();
  EXPECT_TRUE(isSorted(stripe));

  stripe.clearSegment();
  stripe.setY(0);
  stripe.setZ(1);
  stripe.addSegment(0, 1, false);
  stripe.addSegment(4, 5, false);
  stripe.addSegment(1, 0, false);
  stripe.addSegment(3, 9, false);
  stripe.addSegment(3, 5, false);

  stripe.sort();

  EXPECT_TRUE(isSorted(stripe));
}

TEST(ZObject3dStripe, TestCanonize) {
  ZObject3dStripe stripe;
  createStripe2(&stripe);
  EXPECT_FALSE(stripe.isCanonized());

  stripe.canonize();
  EXPECT_TRUE(isSorted(stripe));

  stripe.clearSegment();
  stripe.addSegment(1, 2);
  stripe.addSegment(3, 4, false);
  stripe.addSegment(3, 9, false);
  stripe.addSegment(3, 5, false);
  EXPECT_TRUE(stripe.isCanonized());

  stripe.canonize();
  EXPECT_TRUE(stripe.isCanonized());

  stripe.clearSegment();
  stripe.addSegment(1, 2);
  stripe.addSegment(4, 5, false);
  stripe.addSegment(7, 8, false);
  EXPECT_TRUE(stripe.isCanonized());

  stripe.clearSegment();
  stripe.addSegment(7, 8, false);
  stripe.addSegment(1, 2, false);
  EXPECT_FALSE(stripe.isCanonized());

  stripe.clearSegment();
  stripe.addSegment(4, 5);
  stripe.addSegment(1, 7, false);
  stripe.addSegment(7, 8, false);
  EXPECT_FALSE(stripe.isCanonized());

  stripe.canonize();
  EXPECT_TRUE(stripe.isCanonized());

  stripe.clearSegment();
  stripe.addSegment(4, 5);
  stripe.addSegment(4, 7, false);
  stripe.addSegment(7, 8, false);
  stripe.addSegment(4, 8, false);
  stripe.addSegment(5, 6, false);
  stripe.addSegment(10, 15, false);
  stripe.addSegment(19, 15, false);
  EXPECT_TRUE(stripe.isCanonized());
  EXPECT_EQ(2, stripe.getSegmentNumber());
  EXPECT_EQ(15, (int) stripe.getVoxelNumber());
}

static void createObject(ZObject3dScan *obj)
{
  obj->clear();
  obj->addStripe(0, 0, false);
  obj->addSegment(0, 1, false);
  obj->addSegment(4, 5, false);
  obj->addSegment(7, 8, false);
  obj->addStripe(0, 1, false);
  obj->addSegment(0, 1, false);
  obj->addSegment(3, 3, false);
  obj->addSegment(5, 7, false);
}

static void createObject2(ZObject3dScan *obj)
{
  obj->clear();
  obj->addStripe(0, 0, false);
  obj->addSegment(0, 1, false);
  obj->addSegment(0, 5, false);
  obj->addSegment(0, 8, false);
  obj->addStripe(0, 1, false);
  obj->addSegment(3, 3, false);
  obj->addSegment(0, 1, false);
  obj->addStripe(0, 1, false);
  obj->addSegment(5, 7, false);
}

static void createObject3(ZObject3dScan *obj)
{
  obj->clear();
  obj->addStripe(0, 0);
  obj->addSegment(0, 1);
  obj->addSegment(0, 5);
  obj->addSegment(0, 8);
  obj->addStripe(0, 1);
  obj->addSegment(3, 3);
  obj->addSegment(0, 1);
  obj->addStripe(0, 1);
  obj->addSegment(5, 7);
}

TEST(ZObject3dScan, TestGetProperty) {
  ZObject3dScan obj;
  createObject(&obj);
  obj.print();
  ASSERT_EQ((int) obj.getStripeNumber(), 2);
  ZIntCuboid box = obj.getBoundBox();
  ASSERT_EQ(box.getFirstCorner().getX(), 0);
  ASSERT_EQ(box.getFirstCorner().getY(), 0);
  ASSERT_EQ(box.getFirstCorner().getZ(), 0);

  ASSERT_EQ(box.getLastCorner().getX(), 8);
  ASSERT_EQ(box.getLastCorner().getY(), 1);
  ASSERT_EQ(box.getLastCorner().getZ(), 0);

  ASSERT_EQ((int) obj.getVoxelNumber(), 12);

  obj.canonize();
  ASSERT_EQ((int) obj.getStripeNumber(), 2);
  box = obj.getBoundBox();
  ASSERT_EQ(box.getFirstCorner().getX(), 0);
  ASSERT_EQ(box.getFirstCorner().getY(), 0);
  ASSERT_EQ(box.getFirstCorner().getZ(), 0);

  ASSERT_EQ(box.getLastCorner().getX(), 8);
  ASSERT_EQ(box.getLastCorner().getY(), 1);
  ASSERT_EQ(box.getLastCorner().getZ(), 0);

  ASSERT_EQ((int) obj.getVoxelNumber(), 12);

  createObject2(&obj);
  ASSERT_EQ((int) obj.getStripeNumber(), 2);
  box = obj.getBoundBox();
  ASSERT_EQ(box.getFirstCorner().getX(), 0);
  ASSERT_EQ(box.getFirstCorner().getY(), 0);
  ASSERT_EQ(box.getFirstCorner().getZ(), 0);

  ASSERT_EQ(box.getLastCorner().getX(), 8);
  ASSERT_EQ(box.getLastCorner().getY(), 1);
  ASSERT_EQ(box.getLastCorner().getZ(), 0);

  ASSERT_EQ((int) obj.getVoxelNumber(), 15);

  obj.print();
  obj.canonize();
  obj.print();
  ASSERT_EQ((int) obj.getStripeNumber(), 2);
  box = obj.getBoundBox();
  ASSERT_EQ(box.getFirstCorner().getX(), 0);
  ASSERT_EQ(box.getFirstCorner().getY(), 0);
  ASSERT_EQ(box.getFirstCorner().getZ(), 0);

  ASSERT_EQ(box.getLastCorner().getX(), 8);
  ASSERT_EQ(box.getLastCorner().getY(), 1);
  ASSERT_EQ(box.getLastCorner().getZ(), 0);

  ASSERT_EQ((int) obj.getVoxelNumber(), 15);

  createObject3(&obj);
  ASSERT_EQ((int) obj.getStripeNumber(), 2);
  box = obj.getBoundBox();
  ASSERT_EQ(box.getFirstCorner().getX(), 0);
  ASSERT_EQ(box.getFirstCorner().getY(), 0);
  ASSERT_EQ(box.getFirstCorner().getZ(), 0);

  ASSERT_EQ(box.getLastCorner().getX(), 8);
  ASSERT_EQ(box.getLastCorner().getY(), 1);
  ASSERT_EQ(box.getLastCorner().getZ(), 0);

  ASSERT_EQ((int) obj.getVoxelNumber(), 15);

//  obj.load(GET_TEST_DATA_DIR + "/benchmark/29.sobj");
//  size_t area = obj.getSurfaceArea();
//  std::cout << area << std::endl;
//  std::cout << obj.getVoxelNumber() << std::endl;

}

TEST(ZObject3dScan, TestAddSegment) {
  ZObject3dScan obj;
  obj.addStripe(1, 0);
  obj.addSegment(1, 2, false);
  obj.addSegment(3, 4, false);

  EXPECT_TRUE(obj.isCanonized());

  obj.addStripe(1, 0);
  obj.addSegment(5, 6, false);
  obj.addSegment(7, 8, false);
  EXPECT_TRUE(obj.isCanonized());

  obj.addSegment(5, 6, false);
  EXPECT_TRUE(obj.isCanonized());

  obj.addSegment(3, 6, false);
  EXPECT_TRUE(obj.isCanonized());

  obj.clear();
  obj.addStripe(1, 0);
  obj.addSegment(5, 6, false);
  obj.addSegment(7, 8, false);
  obj.addSegment(3, 6, false);
  EXPECT_FALSE(obj.isCanonized());

  obj.clear();
  obj.addStripe(1, 0);
  obj.addSegment(1, 2, false);
  obj.addSegment(1, 0, 3, 4, false);

  EXPECT_TRUE(obj.isCanonized());

  obj.clear();
  obj.addSegment(1, 0, 5, 6, false);
  obj.addSegment(1, 0, 7, 8, false);
  obj.addSegment(1, 0, 3, 6, false);
  EXPECT_FALSE(obj.isCanonized());

  obj.clear();
  obj.addSegment(0, 0, 0, 1, false);
  obj.addSegment(0, 0, 0, 5, false);
  obj.addSegment(0, 0, 0, 8, false);
  obj.addSegment(0, 1, 3, 3, false);
  obj.addSegment(0, 1, 0, 1, false);
  obj.addSegment(0, 1, 5, 7, false);
  EXPECT_EQ(2, (int) obj.getStripeNumber());
  EXPECT_EQ(15, (int) obj.getVoxelNumber());

  obj.clear();
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 0, 0, 5);
  obj.addSegment(0, 0, 0, 8);
  obj.addSegment(0, 1, 3, 3);
  obj.addSegment(0, 1, 0, 1);
  obj.addSegment(0, 1, 5, 7);
  EXPECT_EQ(2, (int) obj.getStripeNumber());
  EXPECT_EQ(15, (int) obj.getVoxelNumber());
}

TEST(ZObject3dScan, downsample) {
  ZObject3dScan obj;
  createObject(&obj);

  obj.downsample(1, 1, 1);
  EXPECT_EQ(1, (int) obj.getStripeNumber());
  EXPECT_EQ(3, (int) obj.getVoxelNumber());

  createObject(&obj);
  obj.print();
  obj.downsampleMax(1, 1, 1);
  obj.print();
  EXPECT_EQ(1, (int) obj.getStripeNumber());
  EXPECT_EQ(5, (int) obj.getVoxelNumber());

  createObject(&obj);
  obj.downsampleMax(1, 0, 0);
  EXPECT_EQ(2, (int) obj.getStripeNumber());
  EXPECT_EQ(8, (int) obj.getVoxelNumber());
}

TEST(ZObject3dScan, TestObjectSize){
  ZObject3dScan obj;

  std::vector<size_t> sizeArray = obj.getConnectedObjectSize();
  EXPECT_TRUE(sizeArray.empty());

  createObject(&obj);
  obj.print();
  sizeArray = obj.getConnectedObjectSize();
  EXPECT_EQ(2, (int) sizeArray.size());
  EXPECT_EQ(8, (int) sizeArray[0]);
  EXPECT_EQ(4, (int) sizeArray[1]);

  obj.clear();
  Stack *stack = C_Stack::readSc(GET_TEST_DATA_DIR +
        "/benchmark/binary/2d/disk_n2.tif");
  obj.loadStack(stack);
  EXPECT_TRUE(obj.isCanonized());
  sizeArray = obj.getConnectedObjectSize();
  EXPECT_EQ(2, (int) sizeArray.size());
  EXPECT_EQ(489, (int) sizeArray[0]);
  EXPECT_EQ(384, (int) sizeArray[1]);

  C_Stack::kill(stack);

  obj.clear();
  stack = C_Stack::readSc(
        GET_TEST_DATA_DIR +
        "/benchmark/binary/2d/ring_n10.tif");
  obj.loadStack(stack);

  EXPECT_TRUE(obj.isCanonized());

  sizeArray = obj.getConnectedObjectSize();

  EXPECT_EQ(10, (int) sizeArray.size());
  EXPECT_EQ(616, (int) sizeArray[0]);
  EXPECT_EQ(572, (int) sizeArray[1]);
  EXPECT_EQ(352, (int) sizeArray[2]);
  EXPECT_EQ(296, (int) sizeArray[3]);
  EXPECT_EQ(293, (int) sizeArray[4]);
  EXPECT_EQ(279, (int) sizeArray[5]);
  EXPECT_EQ(208, (int) sizeArray[6]);
  EXPECT_EQ(125, (int) sizeArray[7]);
  EXPECT_EQ(112, (int) sizeArray[8]);
  EXPECT_EQ(112, (int) sizeArray[9]);

  C_Stack::kill(stack);

  /*
  obj.clear();
  obj.load(GET_TEST_DATA_DIR +
           "/benchmark/432.sobj");
  sizeArray = obj.getConnectedObjectSize();
  EXPECT_EQ(77, (int) sizeArray.size());
  std::cout << sizeArray[0] << std::endl;

  int offset[3];
  stack = obj.toStack(offset);
  offset[0] = -offset[0];
  offset[1] = -offset[1];
  offset[2] = -offset[2];
  obj.labelStack(stack, 2, offset);
  C_Stack::write(GET_TEST_DATA_DIR +
                 "/test.tif", stack);
                 */
  //EXPECT_EQ(616, (int) sizeArray[0]);
}

TEST(ZObject3dScan, TestBuildGraph) {
  ZObject3dScan obj;
  //createObject(&obj);
  obj.addStripe(0, 0, false);
  obj.addSegment(0, 1, false);

  ZGraph *graph = obj.buildConnectionGraph();

  EXPECT_EQ(0, graph->getEdgeNumber());

  delete graph;

  obj.addSegment(3, 4);
  graph = obj.buildConnectionGraph();
  EXPECT_EQ(0, graph->getEdgeNumber());
  delete graph;

  obj.addStripe(0, 1, false);
  obj.addSegment(0, 1, false);
  graph = obj.buildConnectionGraph();
  EXPECT_EQ(1, graph->getEdgeNumber());
  delete graph;

  obj.addSegment(2, 2, false);
  graph = obj.buildConnectionGraph();
  EXPECT_EQ(2, graph->getEdgeNumber());
  delete graph;

  obj.addStripe(1, 0);
  obj.addSegment(2, 2);
  graph = obj.buildConnectionGraph();
  EXPECT_EQ(5, graph->getEdgeNumber());

  const std::vector<ZGraph*> &subGraph = graph->getConnectedSubgraph();
  EXPECT_EQ(1, (int) subGraph.size());
  delete graph;

  obj.clear();
  Stack *stack = C_Stack::readSc(
        GET_TEST_DATA_DIR +
        "/benchmark/binary/2d/ring_n10.tif");
  obj.loadStack(stack);
  graph = obj.buildConnectionGraph();
  const std::vector<ZGraph*> &subGraph2 = graph->getConnectedSubgraph();
  EXPECT_EQ(10, (int) subGraph2.size());
  delete graph;
  C_Stack::kill(stack);

#if 1
  obj.clear();
  stack = C_Stack::readSc(
        GET_TEST_DATA_DIR +
        "/benchmark/binary/3d/diadem_e1.tif");
  obj.loadStack(stack);
  graph = obj.buildConnectionGraph();
  const std::vector<ZGraph*> &subGraph3 = graph->getConnectedSubgraph();
  EXPECT_EQ(4, (int) subGraph3.size());
  delete graph;
#endif

  obj.clear();
  obj.addStripe(1, 2);
  obj.addSegment(2, 2);
  obj.addStripe(2, 1);
  obj.addSegment(1, 1);
  obj.addSegment(3, 3);
  obj.addStripe(2, 3);
  obj.addSegment(1, 1);
  obj.addSegment(3, 3);
  obj.addStripe(3, 0);
  obj.addSegment(0, 0);
  obj.addSegment(2, 2);
  obj.addSegment(4, 4);
  obj.addStripe(3, 2);
  obj.addSegment(0, 0);
  obj.addSegment(4, 4);
  obj.addStripe(3, 4);
  obj.addSegment(0, 0);
  obj.addSegment(2, 2);
  obj.addSegment(4, 4);
  graph = obj.buildConnectionGraph();
  const std::vector<ZGraph*> &subGraph5 = graph->getConnectedSubgraph();
  EXPECT_EQ(16, graph->getEdgeNumber());
  EXPECT_EQ(1, (int) subGraph5.size());
  delete graph;


  obj.clear();
  stack = C_Stack::readSc(
        GET_TEST_DATA_DIR +
        "/benchmark/binary/3d/series.tif");
  obj.loadStack(stack);
  graph = obj.buildConnectionGraph();
  const std::vector<ZGraph*> &subGraph4 = graph->getConnectedSubgraph();
  EXPECT_EQ(15, (int) subGraph4.size());
  delete graph;

  obj.clear();
  stack = C_Stack::readSc(
        GET_TEST_DATA_DIR +
        "/benchmark/binary/3d/block/test.tif");
  obj.loadStack(stack);
  graph = obj.buildConnectionGraph();
  EXPECT_EQ(1, (int) graph->getConnectedSubgraph().size());
  delete graph;
}

/*
static void createObject(ZObject3dScan *obj)
{
  obj->clear();
  obj->addStripe(0, 0, false);
  obj->addSegment(0, 1, false);
  obj->addSegment(4, 5, false);
  obj->addSegment(7, 8, false);
  obj->addStripe(0, 1, false);
  obj->addSegment(0, 1, false);
  obj->addSegment(3, 3, false);
  obj->addSegment(5, 7, false);
}
*/

TEST(ZObject3dScan, TestGetSegment) {
  ZObject3dScan obj;
  createObject(&obj);
  int z, y, x1, x2;
  obj.getSegment(0, &z, &y, &x1, &x2);
  EXPECT_EQ(0, z);
  EXPECT_EQ(0, y);
  EXPECT_EQ(0, x1);
  EXPECT_EQ(1, x2);

  obj.getSegment(1, &z, &y, &x1, &x2);
  EXPECT_EQ(0, z);
  EXPECT_EQ(0, y);
  EXPECT_EQ(4, x1);
  EXPECT_EQ(5, x2);

  obj.getSegment(2, &z, &y, &x1, &x2);
  EXPECT_EQ(0, z);
  EXPECT_EQ(0, y);
  EXPECT_EQ(7, x1);
  EXPECT_EQ(8, x2);

  obj.getSegment(3, &z, &y, &x1, &x2);
  EXPECT_EQ(0, z);
  EXPECT_EQ(1, y);
  EXPECT_EQ(0, x1);
  EXPECT_EQ(1, x2);

  obj.getSegment(4, &z, &y, &x1, &x2);
  EXPECT_EQ(0, z);
  EXPECT_EQ(1, y);
  EXPECT_EQ(3, x1);
  EXPECT_EQ(3, x2);

  obj.getSegment(5, &z, &y, &x1, &x2);
  EXPECT_EQ(0, z);
  EXPECT_EQ(1, y);
  EXPECT_EQ(5, x1);
  EXPECT_EQ(7, x2);
}

TEST(ZObject3dScan, TestGetConnectedComponent) {
  ZObject3dScan obj;
  createObject(&obj);

  std::vector<ZObject3dScan> objArray =
      obj.getConnectedComponent(ZObject3dScan::ACTION_NONE);
  EXPECT_EQ(2, (int) objArray.size());
  EXPECT_EQ(4, (int) objArray[0].getVoxelNumber());
  EXPECT_EQ(8, (int) objArray[1].getVoxelNumber());

  obj.clear();
  Stack *stack = C_Stack::readSc(
        GET_TEST_DATA_DIR +
        "/benchmark/binary/2d/ring_n10.tif");
  obj.loadStack(stack);

  objArray = obj.getConnectedComponent(ZObject3dScan::ACTION_NONE);
  EXPECT_EQ(10, (int) objArray.size());
  EXPECT_EQ(352, (int) objArray[0].getVoxelNumber());
  EXPECT_EQ(279, (int) objArray[1].getVoxelNumber());
  EXPECT_EQ(125, (int) objArray[2].getVoxelNumber());
  EXPECT_EQ(112, (int) objArray[3].getVoxelNumber());
  EXPECT_EQ(616, (int) objArray[4].getVoxelNumber());
  EXPECT_EQ(112, (int) objArray[5].getVoxelNumber());
  EXPECT_EQ(296, (int) objArray[6].getVoxelNumber());
  EXPECT_EQ(293, (int) objArray[7].getVoxelNumber());
  EXPECT_EQ(572, (int) objArray[8].getVoxelNumber());
  EXPECT_EQ(208, (int) objArray[9].getVoxelNumber());

  C_Stack::kill(stack);

  obj.clear();
  stack = C_Stack::readSc(
        GET_TEST_DATA_DIR +
        "/benchmark/binary/3d/diadem_e1.tif");
  obj.loadStack(stack);
  objArray = obj.getConnectedComponent(ZObject3dScan::ACTION_NONE);
  EXPECT_EQ(43, (int) objArray.size());
  EXPECT_EQ(2, (int) objArray[0].getVoxelNumber());
  EXPECT_EQ(68236, (int) objArray[1].getVoxelNumber());
  EXPECT_EQ(2, (int) objArray[2].getVoxelNumber());
  EXPECT_EQ(3, (int) objArray[3].getVoxelNumber());

  /*
  for (size_t i = 0; i < objArray.size(); ++i) {
    std::cout<< objArray[i].getVoxelNumber() << std::endl;
  }
*/
}

TEST(ZObject3dScan, duplicateAcrossZ)
{
  ZObject3dScan obj;
  obj.addSegment(0, 0, 1, 2);
  obj.duplicateSlice(3);

  EXPECT_EQ(3, (int) obj.getStripeNumber());
  EXPECT_EQ(6, (int) obj.getVoxelNumber());

  obj.duplicateSlice(2);
  EXPECT_EQ(2, (int) obj.getStripeNumber());
  EXPECT_EQ(4, (int) obj.getVoxelNumber());

  obj.addSegment(0, 1, 3, 4);
  obj.duplicateSlice(3);

  EXPECT_EQ(6, (int) obj.getStripeNumber());
  EXPECT_EQ(12, (int) obj.getVoxelNumber());

  //obj.print();
}

TEST(ZObject3dScan, TestScanArray) {
  Stack *stack = C_Stack::readSc(GET_TEST_DATA_DIR +
                                 "/benchmark/binary/3d/diadem_e1.tif");

  std::map<uint64_t, ZObject3dScan*> *objSet = ZObject3dScan::extractAllObject(
        stack->array, C_Stack::width(stack), C_Stack::height(stack),
        C_Stack::depth(stack), 0, 1, NULL);

  EXPECT_EQ(2, (int) objSet->size());
  EXPECT_TRUE((*objSet)[0]->isCanonizedActually());
  EXPECT_TRUE((*objSet)[1]->isCanonizedActually());

  (*objSet)[1]->save(GET_TEST_DATA_DIR + "/test.sobj");
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/test.sobj");
  EXPECT_TRUE(obj.isCanonizedActually());


  ZStack stack2;
  stack2.load(GET_TEST_DATA_DIR + "/benchmark/block.tif");
  std::vector<ZObject3dScan*> objArray =
      ZObject3dScan::extractAllObject(stack2);
  ASSERT_EQ(24, (int) objArray.size());
  for (size_t i = 0; i < objArray.size(); ++i) {
    ZObject3dScan *obj = objArray[i];
    ASSERT_EQ(1, (int) obj->getVoxelNumber());
  }

  stack2.setOffset(ZIntPoint(30, 40, 50));
  objArray = ZObject3dScan::extractAllObject(stack2);
  ASSERT_EQ(24, (int) objArray.size());
  for (size_t i = 0; i < objArray.size(); ++i) {
    ZObject3dScan *obj = objArray[i];
//    obj->print();
    ASSERT_EQ(1, (int) obj->getVoxelNumber());
  }

  //obj.scanArray(array, )
}

TEST(ZObject3dScan, TestIO) {
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/benchmark/29.sobj");

  EXPECT_TRUE(obj.isCanonizedActually());
}

TEST(ZObject3dScan, dilate) {
  ZObject3dScan obj;

  obj.addSegment(0, 0, 0, 1);

  obj.dilate();
  EXPECT_EQ(5, (int) obj.getStripeNumber());
  EXPECT_EQ(12, (int) obj.getVoxelNumber());

  obj.clear();
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 0, 3, 4);

  obj.dilate();

  EXPECT_EQ(5, (int) obj.getStripeNumber());
  EXPECT_EQ(23, (int) obj.getVoxelNumber());

  obj.clear();
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 1, 0, 1);
  obj.dilate();
  //obj.print();
  EXPECT_EQ(8, (int) obj.getStripeNumber());
  EXPECT_EQ(20, (int) obj.getVoxelNumber());

  obj.clear();;
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 0, 4, 5);
  obj.dilate();
  EXPECT_EQ(5, (int) obj.getStripeNumber());
  EXPECT_EQ(24, (int) obj.getVoxelNumber());

  obj.clear();
  obj.addSegment(0, -1, 2, 3);
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 0, 4, 5);
  obj.addSegment(0, 1, 2, 3);
  //obj.print();
  obj.dilate();
  //obj.print();
  EXPECT_EQ(11, (int) obj.getStripeNumber());
  EXPECT_EQ(13, (int) obj.getSegmentNumber());
  EXPECT_EQ(40, (int) obj.getVoxelNumber());

  obj.clear();
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(1, 0, 0, 1);
  obj.dilate();
  EXPECT_EQ(8, (int) obj.getStripeNumber());
  EXPECT_EQ(20, (int) obj.getVoxelNumber());
}

TEST(ZObject3dScan, overlap)
{
  Stack *stack = C_Stack::make(GREY, 3, 3, 3);
  Zero_Stack(stack);
  stack->array[0] = 1;

  ZObject3dScan obj;
  obj.addSegment(0, 0, 0, 1);

  EXPECT_EQ(1, (int) obj.countForegroundOverlap(stack));
}

class ZObject3dScanTestF1 : public ::testing::Test {
protected:
  virtual void SetUp() {
    m_obj.clear();
    m_obj.addSegment(0, 1, 0, 2);
    m_obj.addSegment(1, 0, 1, 1);
    m_obj.addSegment(1, 1, 0, 2);
    m_obj.addSegment(1, 2, 1, 1);
    m_obj.addSegment(2, 0, 1, 1);
    m_obj.addSegment(2, 1, 1, 1);
    m_obj.addSegment(2, 2, 1, 1);
  }

  virtual void TearDown() {}

  ZObject3dScan m_obj;
};

TEST_F(ZObject3dScanTestF1, Slice)
{
  ZObject3dScan obj = m_obj;
  obj.addSegment(0, 1, 0, 2);
  obj.addSegment(1, 0, 1, 1);
  obj.addSegment(1, 1, 0, 2);
  obj.addSegment(1, 2, 1, 1);
  obj.addSegment(2, 0, 1, 1);
  obj.addSegment(2, 1, 1, 1);
  obj.addSegment(2, 2, 1, 1);

  ZObject3dScan slice = obj.getSlice(0);
  EXPECT_EQ(3, (int) slice.getVoxelNumber());

  slice = obj.getSlice(1);
  EXPECT_EQ(5, (int) slice.getVoxelNumber());

  slice = obj.getSlice(2);
  EXPECT_EQ(3, (int) slice.getVoxelNumber());

  slice = obj.getSlice(0, 1);
//  slice.print();
  EXPECT_EQ(8, (int) slice.getVoxelNumber());

  slice = obj.getSlice(0, 2);
  EXPECT_EQ(11, (int) slice.getVoxelNumber());
}

TEST_F(ZObject3dScanTestF1, Statistics)
{
  ZObject3dScan obj = m_obj;
  obj.addSegment(0, 1, 0, 2);
  obj.addSegment(1, 0, 1, 1);
  obj.addSegment(1, 1, 0, 2);
  obj.addSegment(1, 2, 1, 1);
  obj.addSegment(2, 0, 1, 1);
  obj.addSegment(2, 1, 1, 1);
  obj.addSegment(2, 2, 1, 1);

  ZPoint center = obj.getCentroid();
  EXPECT_DOUBLE_EQ(1.0, center.x());
  EXPECT_DOUBLE_EQ(1.0, center.y());
  EXPECT_DOUBLE_EQ(1.0, center.z());
}

TEST_F(ZObject3dScanTestF1, equal)
{
  ZObject3dScan obj = m_obj;
  EXPECT_TRUE(m_obj.equalsLiterally(obj));
  obj.canonize();
  EXPECT_TRUE(m_obj.equalsLiterally(obj));

  obj.addSegment(2, 1, 1, 1, false);
  EXPECT_FALSE(m_obj.equalsLiterally(obj));

  obj.canonize();
  //obj.print();
  //m_obj.print();
  EXPECT_TRUE(m_obj.equalsLiterally(obj));
}

TEST_F(ZObject3dScanTestF1, complement)
{
  ZObject3dScan obj = m_obj;
  ZObject3dScan compObj = obj.getComplementObject();

  int offset[3];
  Stack *stack = obj.toStack(offset);
  iarray_neg(offset, 3);
  ASSERT_EQ(0, (int) compObj.countForegroundOverlap(stack, offset));

  ASSERT_EQ(obj.getVoxelNumber() + compObj.getVoxelNumber(),
            C_Stack::voxelNumber(stack));

  C_Stack::kill(stack);
}

TEST(ZObject3dScanTest, findHole)
{
  ZObject3dScan obj;
  Stack *stack = C_Stack::make(GREY, 3, 3, 3);
  One_Stack(stack);
  C_Stack::setPixel(stack, 1, 1, 1, 0, 0);

  obj.loadStack(stack);
  //obj.print();

  ZObject3dScan hole = obj.findHoleObject();

  ASSERT_EQ(1, (int) hole.getVoxelNumber());

  C_Stack::kill(stack);

  stack = C_Stack::make(GREY, 5, 5, 5);
  One_Stack(stack);
  C_Stack::setPixel(stack, 0, 0, 0, 0, 0);
  C_Stack::setPixel(stack, 2, 2, 2, 0, 0);
  obj.loadStack(stack);

  //obj.print();

  hole = obj.findHoleObject();

  ASSERT_EQ(1, (int) hole.getVoxelNumber());

  C_Stack::setPixel(stack, 2, 2, 3, 0, 0);
  obj.loadStack(stack);
  hole = obj.findHoleObject();
  ASSERT_EQ(2, (int) hole.getVoxelNumber());

  C_Stack::kill(stack);
}

TEST(ZObject3dScan, Cov)
{
  ZObject3dScan obj;
  obj.addSegment(0, 0, 1, 1);
  std::vector<double> cov = obj.getPlaneCov();
  ASSERT_DOUBLE_EQ(0.0, cov[0]);
  ASSERT_DOUBLE_EQ(0.0, cov[1]);
  ASSERT_DOUBLE_EQ(0.0, cov[2]);

  obj.addSegment(0, 0, 1, 2);
  cov = obj.getPlaneCov();
  ASSERT_DOUBLE_EQ(0.5, cov[0]);
  ASSERT_DOUBLE_EQ(0.0, cov[1]);
  ASSERT_DOUBLE_EQ(0.0, cov[2]);

  obj.addSegment(0, 0, 1, 3);
  obj.addSegment(0, 1, 1, 3);
  cov = obj.getPlaneCov();
  ASSERT_DOUBLE_EQ(0.8, cov[0]);
  ASSERT_DOUBLE_EQ(0.3, cov[1]);
  ASSERT_DOUBLE_EQ(0.0, cov[2]);


  obj.addSegment(0, 0, 1, 3);
  obj.addSegment(0, 1, 1, 3);
  obj.addSegment(0, 2, 3, 6);
  cov = obj.getPlaneCov();
  ASSERT_DOUBLE_EQ(2.666666666666667, cov[0]);
  ASSERT_DOUBLE_EQ(0.76666666666666639, cov[1]);
  ASSERT_DOUBLE_EQ(1.0, cov[2]);

  obj.clear();
  obj.addSegment(0, 0, 1, 3);
  obj.addSegment(1, 1, 1, 3);
  obj.addSegment(2, 2, 3, 6);
  cov = obj.getPlaneCov();
  ASSERT_DOUBLE_EQ(2.666666666666667, cov[0]);
  ASSERT_DOUBLE_EQ(0.76666666666666639, cov[1]);
  ASSERT_DOUBLE_EQ(1.0, cov[2]);

  obj.clear();
  obj.addSegment(0, 0, 1, 5);
  obj.addSegment(0, 1, 1, 5);
  obj.addSegment(0, 2, 1, 5);
  std::cout << obj.getSpread(0) << std::endl;
  //ZDebugPrintDoubleArray(cov, 0, 2);
}

TEST(ZObject3dScan, contains)
{
  ZObject3dStripe stripe;
  stripe.setY(0);
  stripe.setZ(0);
  stripe.addSegment(0, 1);

  ASSERT_TRUE(stripe.containsX(0));
  ASSERT_FALSE(stripe.containsX(2));

  stripe.addSegment(3, 4);
  stripe.addSegment(6, 9);
  ASSERT_TRUE(stripe.containsX(0));
  ASSERT_FALSE(stripe.containsX(2));
  ASSERT_TRUE(stripe.containsX(7));
  ASSERT_FALSE(stripe.containsX(10));

  ZObject3dScan obj;
  obj.addSegment(0, 0, 0, 1);
  ASSERT_TRUE(obj.contains(0, 0, 0));
  ASSERT_FALSE(obj.contains(2, 0, 0));

  obj.addSegment(0, 0, 3, 4);
  obj.addSegment(0, 0, 6, 9);
  ASSERT_TRUE(obj.contains(7, 0, 0));
  ASSERT_FALSE(obj.contains(10, 0, 0));

  obj.addSegment(0, 1, 0, 1);
  ASSERT_TRUE(obj.contains(0, 1, 0));
  ASSERT_FALSE(obj.contains(0, 0, 1));

  obj.addSegment(3, 4, 0, 10);
  obj.addSegment(6, 8, 1, 10);
  ASSERT_TRUE(obj.contains(0, 4, 3));
  ASSERT_FALSE(obj.contains(0, 8, 6));

  obj.clear();
  obj.load(GET_TEST_DATA_DIR + "/benchmark/tower3.sobj");
  ASSERT_TRUE(obj.contains(1, 1, 0));
  ASSERT_FALSE(obj.contains(0, 0, 0));
  ASSERT_FALSE(obj.contains(0, 0, 2));
  ASSERT_TRUE(obj.contains(1, 0, 2));
  ASSERT_TRUE(obj.contains(1, 1, 1));
  ASSERT_FALSE(obj.contains(2, 0, 1));
  ASSERT_TRUE(obj.contains(2, 1, 2));
  ASSERT_TRUE(obj.contains(1, 2, 2));
  ASSERT_FALSE(obj.contains(2, 2, 2));

//  obj.addSegment(0, 0, 3, 4);
//  obj.addSegment(0, 0, 6, 9);
}

TEST(ZObject3dScan, component)
{
  ZObject3dScan obj;
  obj.addSegment(0, 0, 1, 1);

  std::map<int, size_t> &vs = obj.getSlicewiseVoxelNumber();
  ASSERT_EQ(1, (int) vs[0]);
  ASSERT_EQ(0, (int) vs.count(2));

  obj.addSegment(0, 0, 1, 2);
  vs = obj.getSlicewiseVoxelNumber();
  ASSERT_EQ(2, (int) vs[0]);
  ASSERT_EQ(0, (int) vs.count(2));

  vs = obj.getSlicewiseVoxelNumber();
  ASSERT_EQ(2, (int) vs[0]);
  ASSERT_EQ(0, (int) vs.count(2));

  obj.addSegment(1, 1, 2, 4);
  vs = obj.getSlicewiseVoxelNumber();
  ASSERT_EQ(2, (int) vs[0]);
  ASSERT_EQ(3, (int) vs[1]);
  ASSERT_EQ(0, (int) vs.count(2));

  obj.addSegment(5, 1, 0, 4);
  vs = obj.getSlicewiseVoxelNumber();
  ASSERT_EQ(2, (int) vs[0]);
  ASSERT_EQ(3, (int) vs[1]);
  ASSERT_EQ(5, (int) vs[5]);
  ASSERT_EQ(0, (int) vs.count(2));

  ASSERT_EQ(2, (int) vs[0]);
  ASSERT_EQ(3, (int) vs[1]);
  ASSERT_EQ(5, (int) vs[5]);
  ASSERT_EQ(0, (int) vs.count(2));
}

TEST(ZObject3dScan, load)
{
  std::vector<int> array;
  array.push_back(1);
  array.push_back(0);
  array.push_back(0);
  array.push_back(1);
  array.push_back(0);
  array.push_back(1);

  ZObject3dScan obj;
  ASSERT_TRUE(obj.load(&(array[0]), array.size()));
  //obj.print();
  ASSERT_EQ(2, (int) obj.getVoxelNumber());
}

TEST(ZObject3dScan, relation)
{
  ZObject3dScan obj1;
  obj1.addStripe(0, 0);
  obj1.addSegment(0, 2);

  ZObject3dScan obj2;
  obj2.addStripe(1, 0);
  obj2.addSegment(0, 1);

  ASSERT_FALSE(obj1.hasOverlap(obj2));

  obj2.addSegment(0, 0, 0, 1);

  //obj2.print();
  ASSERT_TRUE(obj1.hasOverlap(obj2));

  obj2.clear();
  ASSERT_FALSE(obj1.hasOverlap(obj2));

  obj1.clear();
  ASSERT_FALSE(obj1.hasOverlap(obj2));

  obj1.addStripe(10, 20);
  obj1.addSegment(30, 40);

  obj2.addStripe(10, 20);
  obj2.addSegment(30, 30);
  ASSERT_TRUE(obj1.hasOverlap(obj2));

  obj2.clear();
  obj2.addStripe(10, 20);
  obj2.addSegment(29, 29);
  ASSERT_FALSE(obj1.hasOverlap(obj2));
}

TEST(ZObject3dScan, upSample)
{
  ZObject3dScan obj;
  createObject(&obj);
  //obj.print();

  std::cout << "Upsampling" << std::endl;
  obj.upSample(1, 0, 0);
  ASSERT_TRUE(obj.contains(0, 0, 0));
  ASSERT_TRUE(obj.contains(15, 0, 0));
  ASSERT_FALSE(obj.contains(15, 0, 1));
  ASSERT_TRUE(obj.contains(15, 1, 0));

  //obj.print();

//  std::cout << "Upsampling" << std::endl;
//  obj.upSample(1, 1, 1);
//  obj.print();

//  obj.downsampleMax(1, 0, 0);
//  obj.print();


//  ZObject3dScan obj1;
//  obj1.addStripe(1, 1);
//  obj1.addSegment(0, 2);

//  std::cout << "Upsampling" << std::endl;
//  obj1.upSample(1, 1, 1);
//  obj1.print();

}

TEST(ZObject3dScan, Stack)
{
  ZObject3dScan obj;
  obj.addStripe(1, 1);
  obj.addSegment(1, 1);
  ZStack *stack = obj.toStackObject(1);
  stack->printInfo();

  std::vector<ZObject3dScan*> objArray =
      ZObject3dScan::extractAllObject(*stack);
  ASSERT_EQ(1, (int) objArray.size());
  //objArray[0]->print();

  obj.addStripe(1, 0, false);
  obj.addSegment(0, 0);
  obj.addStripe(1, 2, false);
  obj.addSegment(2, 2);

  obj.canonize();

  obj.print();

  delete stack;
  stack = obj.toStackObject(2);
  Print_Stack_Value(stack->c_stack());
  objArray = ZObject3dScan::extractAllObject(*stack);
  ASSERT_EQ(1, (int) objArray.size());
  objArray[0]->print();

  ZObject3dScan obj2;
  obj2.addStripe(1, 1);
  obj2.addSegment(1, 1);

  ZObject3dScan obj3 = obj.subtract(obj2);
  obj.print();
  obj2.print();
  obj3.print();

  obj.clear();
  obj.addStripe(1, 1);
  obj.addSegment(1, 1);
  obj.addSegment(1, 3);
  obj.addStripe(0, 0, false);
  obj.addSegment(1, 3);

  obj.print();

  delete stack;
  stack = ZStackFactory::makeIndexStack(3, 3, 3);

  obj.maskStack(stack);

  Print_Stack_Value(stack->c_stack());
}

TEST(ZObject3dScan, Intersect)
{
  ZObject3dScan obj;
  obj.addStripe(1, 1);
  obj.addSegment(1, 1);

  ZObject3dScan obj2;
  obj2.addStripe(1, 1);
  obj2.addSegment(1, 1);

  ZObject3dScan obj3 = obj.intersect(obj2);

  ASSERT_TRUE(obj3.equalsLiterally(obj));

  obj2.addSegment(1, 2, 0, 5);
  obj2.addSegment(1, 1, 0, 1);
  obj3 = obj.intersect(obj2);

  ASSERT_TRUE(obj3.equalsLiterally(obj));

  obj.addSegment(2, 1, 0, 5);
  obj3 = obj.intersect(obj2);

  ASSERT_EQ(1, (int) obj3.getVoxelNumber());
  ASSERT_TRUE(obj3.contains(1, 1, 1));
}

#endif

#endif // ZOBJECT3DSCANTEST_H

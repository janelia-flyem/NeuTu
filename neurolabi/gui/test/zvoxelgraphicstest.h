#ifndef ZVOXELGRAPHICSTEST_H
#define ZVOXELGRAPHICSTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zvoxelgraphics.h"
#include "zobject3d.h"

#ifdef _USE_GTEST_

TEST(ZVoxelGraphics, paint)
{
  ZIntPoint v1(0, 0, 0);
  ZIntPoint v2(0, 0, 0);
  ZObject3d *line = ZVoxelGraphics::createLineObject(v1, v2);
  ASSERT_EQ(1, (int) line->size());
  ASSERT_EQ(0, line->getX(0));
  ASSERT_EQ(0, line->getY(0));
  ASSERT_EQ(0, line->getZ(0));
  delete line;

  v2.set(1, 0, 0);
  line = ZVoxelGraphics::createLineObject(v1, v2);
  ASSERT_EQ(2, (int) line->size());
  ASSERT_EQ(0, line->getX(0));
  ASSERT_EQ(0, line->getY(0));
  ASSERT_EQ(0, line->getZ(0));

  ASSERT_EQ(1, line->getX(1));
  ASSERT_EQ(0, line->getY(1));
  ASSERT_EQ(0, line->getZ(1));
  delete line;

  v1.set(1, 1, 1);
  v2.set(5, 5, 5);
  line = ZVoxelGraphics::createLineObject(v1, v2);

  //line->print();

  ASSERT_EQ(5, (int) line->size());
  ASSERT_EQ(1, line->getX(0));
  ASSERT_EQ(1, line->getY(0));
  ASSERT_EQ(1, line->getZ(0));

  ASSERT_EQ(5, line->getX(4));
  ASSERT_EQ(5, line->getY(4));
  ASSERT_EQ(5, line->getZ(4));
  delete line;

  v1.set(5, 5, 5);
  v2.set(1, 1, 1);
  line = ZVoxelGraphics::createLineObject(v1, v2);

  //line->print();

  ASSERT_EQ(5, (int) line->size());
  ASSERT_EQ(5, line->getX(0));
  ASSERT_EQ(5, line->getY(0));
  ASSERT_EQ(5, line->getZ(0));

  ASSERT_EQ(1, line->getX(4));
  ASSERT_EQ(1, line->getY(4));
  ASSERT_EQ(1, line->getZ(4));
  delete line;

  v1.set(0, 0, 0);
  ZPoint vec1(1, 0, 0);
  double len1 = 2;
  ZPoint vec2(0, 1, 0);
  double len2 = 2;
  ZObject3d *plane = ZVoxelGraphics::createPlaneObject(
        v1, vec1, len1, vec2, len2);
  //plane->print();
  ASSERT_EQ(9, (int) plane->size());
  ASSERT_EQ(0, plane->getX(0));
  ASSERT_EQ(0, plane->getY(0));
  ASSERT_EQ(0, plane->getZ(0));

  ASSERT_EQ(2, plane->getX(8));
  ASSERT_EQ(2, plane->getY(8));
  ASSERT_EQ(0, plane->getZ(8));
  delete plane;

  std::vector<ZIntPoint> polyline;
  polyline.push_back(ZIntPoint(0, 0, 0));
  polyline.push_back(ZIntPoint(1, 1, 1));
  line = ZVoxelGraphics::createPolylineObject(polyline);
  line->print();

  ASSERT_EQ(2, (int) line->size());
  ASSERT_EQ(0, line->getX(0));
  ASSERT_EQ(0, line->getY(0));
  ASSERT_EQ(0, line->getZ(0));

  ASSERT_EQ(1, line->getX(1));
  ASSERT_EQ(1, line->getY(1));
  ASSERT_EQ(1, line->getZ(1));

  delete line;

  polyline.push_back(ZIntPoint(3, 4, 5));
  line = ZVoxelGraphics::createPolylineObject(polyline);
  line->print();
  ASSERT_EQ(6, (int) line->size());
  ASSERT_EQ(0, line->getX(0));
  ASSERT_EQ(0, line->getY(0));
  ASSERT_EQ(0, line->getZ(0));

  ASSERT_EQ(1, line->getX(1));
  ASSERT_EQ(1, line->getY(1));
  ASSERT_EQ(1, line->getZ(1));

  delete line;

  plane = ZVoxelGraphics::createPolyPlaneObject(polyline, vec2, len2);
  Stack *stack = plane->toStack();
  //C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", stack);
  C_Stack::kill(stack);

  delete plane;

  ZObject3d *triangle = ZVoxelGraphics::createTriangleObject(
        ZIntPoint(0, 0, 0), ZIntPoint(0, 0, 0), ZIntPoint(0, 0, 0));
  triangle->print();
  ASSERT_EQ(1, (int) triangle->size());
  ASSERT_EQ(0, triangle->getX(0));
  ASSERT_EQ(0, triangle->getY(0));
  ASSERT_EQ(0, triangle->getZ(0));

  delete triangle;

  triangle = ZVoxelGraphics::createTriangleObject(
          ZIntPoint(22, 9, 0), ZIntPoint(3, 28, 10), ZIntPoint(28, 30, 20));

  //triangle->print();
  stack = triangle->toStack();
  //C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", stack);
  C_Stack::kill(stack);

  delete triangle;

  ZObject3d *quad = ZVoxelGraphics::createQuadrangleObject(
        ZIntPoint(0, 10, 0), ZIntPoint(60, 0, 80),
        ZIntPoint(-50, 100, 30), ZIntPoint(150, 100, 50));
  stack = quad->toStack();
  //C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", stack);
  C_Stack::kill(stack);

  delete quad;

  std::vector<ZIntPoint> polyline1;
  std::vector<ZIntPoint> polyline2;
  polyline1.push_back(ZIntPoint(0, 10, 0));
  polyline1.push_back(ZIntPoint(50, 0, 80));
  polyline1.push_back(ZIntPoint(100, 0, 30));
  polyline2.push_back(ZIntPoint(-50, 100, 30));
  polyline2.push_back(ZIntPoint(150, 100, 50));
  polyline2.push_back(ZIntPoint(250, 200, 0));
  quad = ZVoxelGraphics::createPolyPlaneObject(polyline1, polyline2);
  stack = quad->toStack();
  //C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", stack);
  C_Stack::kill(stack);


  delete quad;
}

#endif

#endif // ZVOXELGRAPHICSTEST_H

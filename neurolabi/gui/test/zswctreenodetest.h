#ifndef ZSWCTREENODETEST_H
#define ZSWCTREENODETEST_H

#include "ztestheader.h"

#include "tz_constant.h"
#include "swctreenode.h"

#ifdef _USE_GTEST_

TEST(SwcTreeNode, Attribute)
{
  Swc_Tree_Node *tn = SwcTreeNode::MakePointer(ZPoint(1, 2, 3), 4);
  EXPECT_EQ(1.0, SwcTreeNode::x(tn));
  EXPECT_EQ(2.0, SwcTreeNode::y(tn));
  EXPECT_EQ(3.0, SwcTreeNode::z(tn));
  EXPECT_EQ(4.0, SwcTreeNode::radius(tn));
  EXPECT_TRUE(SwcTreeNode::isRegular(tn));
  EXPECT_TRUE(SwcTreeNode::isParentIdConsistent(tn));
  SwcTreeNode::kill(tn);

  //EXPECT_DEATH(SwcTreeNode::kill(tn), "pointer being freed was not allocated");

  tn = SwcTreeNode::MakePointer(1, 2, 3, 4);
  EXPECT_EQ(1.0, SwcTreeNode::x(tn));
  EXPECT_EQ(2.0, SwcTreeNode::y(tn));
  EXPECT_EQ(3.0, SwcTreeNode::z(tn));
  EXPECT_EQ(4.0, SwcTreeNode::radius(tn));
  EXPECT_TRUE(SwcTreeNode::isParentIdConsistent(tn));
  SwcTreeNode::kill(tn);

  tn = SwcTreeNode::MakePointer(5, 6, 1, 2, 3, 4, 7);
  EXPECT_EQ(1.0, SwcTreeNode::x(tn));
  EXPECT_EQ(2.0, SwcTreeNode::y(tn));
  EXPECT_EQ(3.0, SwcTreeNode::z(tn));
  EXPECT_EQ(4.0, SwcTreeNode::radius(tn));
  EXPECT_EQ(5, SwcTreeNode::id(tn));
  EXPECT_EQ(6, SwcTreeNode::type(tn));
  EXPECT_EQ(7, SwcTreeNode::parentId(tn));
  EXPECT_FALSE(SwcTreeNode::isParentIdConsistent(tn));
  SwcTreeNode::kill(tn);

  tn = SwcTreeNode::MakePointer(5, 6, ZPoint(1, 2, 3), 4, 7);
  EXPECT_EQ(1.0, SwcTreeNode::x(tn));
  EXPECT_EQ(2.0, SwcTreeNode::y(tn));
  EXPECT_EQ(3.0, SwcTreeNode::z(tn));
  EXPECT_EQ(4.0, SwcTreeNode::radius(tn));
  EXPECT_EQ(5, SwcTreeNode::id(tn));
  EXPECT_EQ(6, SwcTreeNode::type(tn));
  EXPECT_EQ(7, SwcTreeNode::parentId(tn));
  EXPECT_FALSE(SwcTreeNode::isParentIdConsistent(tn));

  Swc_Tree_Node *tn2 = SwcTreeNode::MakePointer();

  SwcTreeNode::copyProperty(tn, tn2);

  EXPECT_EQ(1.0, SwcTreeNode::x(tn2));
  EXPECT_EQ(2.0, SwcTreeNode::y(tn2));
  EXPECT_EQ(3.0, SwcTreeNode::z(tn2));
  EXPECT_EQ(4.0, SwcTreeNode::radius(tn2));
  EXPECT_EQ(5, SwcTreeNode::id(tn2));
  EXPECT_EQ(6, SwcTreeNode::type(tn2));
  EXPECT_EQ(7, SwcTreeNode::parentId(tn2));

  SwcTreeNode::kill(tn);

  tn = SwcTreeNode::makeVirtualNode();
  EXPECT_TRUE(SwcTreeNode::isVirtual(tn));
  SwcTreeNode::kill(tn);

  tn = SwcTreeNode::MakePointer(5, 6, ZPoint(1, 2, 3), 4, 7);
  ZCuboid box = SwcTreeNode::boundBox(tn);
  EXPECT_EQ(1, box.center().x());
  EXPECT_EQ(2, box.center().y());
  EXPECT_EQ(3, box.center().z());
  EXPECT_EQ(8, box.width());
  EXPECT_EQ(8, box.height());
  EXPECT_EQ(8, box.depth());

  EXPECT_FALSE(SwcTreeNode::hasChild(tn));
  EXPECT_EQ(0, SwcTreeNode::childNumber(tn));
  EXPECT_EQ(0, SwcTreeNode::minChildLabel(tn));
  EXPECT_FALSE(SwcTreeNode::isBranchPoint(tn));
  EXPECT_FALSE(SwcTreeNode::isLeaf(tn));
  EXPECT_EQ(1, SwcTreeNode::downstreamSize(tn));
  EXPECT_EQ(1, SwcTreeNode::singleTreeSize(tn));
  EXPECT_EQ(NULL, SwcTreeNode::nextSibling(tn));
  EXPECT_EQ(NULL, SwcTreeNode::prevSibling(tn));
  EXPECT_EQ(NULL, SwcTreeNode::lastChild(tn));
  EXPECT_TRUE(SwcTreeNode::isRoot(tn));




  SwcTreeNode::kill(tn);
  SwcTreeNode::kill(tn2);
}

TEST(SwcTreeNode, Structure)
{
  Swc_Tree_Node *tn1 = SwcTreeNode::MakePointer(ZPoint(1, 2, 3), 4);
  EXPECT_EQ(tn1, SwcTreeNode::regularRoot(tn1));
  EXPECT_EQ(tn1, SwcTreeNode::root(tn1));

  Swc_Tree_Node *tn2 = SwcTreeNode::MakePointer(ZPoint(4, 5, 6), 1);
  EXPECT_EQ(NULL, SwcTreeNode::commonAncestor(tn1, tn2));

  SwcTreeNode::setParent(tn1, tn2);
  EXPECT_EQ(tn2, SwcTreeNode::parent(tn1));

  EXPECT_TRUE(SwcTreeNode::isAncestor(tn2, tn1));
  EXPECT_TRUE(SwcTreeNode::isConnected(tn2, tn1));

  Swc_Tree_Node *tn3 = SwcTreeNode::MakePointer(ZPoint(7, 8, 9), 1);
  SwcTreeNode::setParent(tn3, tn2);
  EXPECT_TRUE(SwcTreeNode::isAncestor(tn2, tn3));
  EXPECT_TRUE(SwcTreeNode::isConnected(tn2, tn3));
  EXPECT_EQ(tn2, SwcTreeNode::commonAncestor(tn1, tn3));

  SwcTreeNode::kill(tn1);
  SwcTreeNode::kill(tn2);
  SwcTreeNode::kill(tn3);
}

TEST(SwcTreeNode, Geometry)
{
  Swc_Tree_Node *tn1 = SwcTreeNode::MakePointer(0, 0, 0, 1);
  Swc_Tree_Node *tn2 = SwcTreeNode::MakePointer(0, 0, 0, 1);

  double dist = SwcTreeNode::distance(tn1, tn2);
  EXPECT_DOUBLE_EQ(0.0, dist);

  dist = SwcTreeNode::distance(tn1, tn2, SwcTreeNode::EUCLIDEAN_SURFACE);
  EXPECT_DOUBLE_EQ(-2.0, dist);

  dist = SwcTreeNode::distance(tn1, tn2, SwcTreeNode::GEODESIC);
  std::cout << dist << std::endl;
  EXPECT_TRUE(tz_isinf(dist));

  SwcTreeNode::setParent(tn1, tn2);
  dist = SwcTreeNode::distance(tn1, tn2, SwcTreeNode::GEODESIC);
  EXPECT_DOUBLE_EQ(0.0, dist);

  dist = SwcTreeNode::distance(tn1, 0, 0, 0, SwcTreeNode::EUCLIDEAN_SURFACE);
  EXPECT_DOUBLE_EQ(-1.0, dist);

  dist = SwcTreeNode::distance(tn1, 0, 0, 0);
  EXPECT_DOUBLE_EQ(0.0, dist);

  dist = SwcTreeNode::distance(tn1, 1, 0, 0);
  EXPECT_DOUBLE_EQ(1.0, dist);

  dist = SwcTreeNode::scaledDistance(tn1, tn2, 1, 2, 3);
  EXPECT_DOUBLE_EQ(0.0, dist);

  dist = SwcTreeNode::scaledSurfaceDistance(tn1, tn2, 1, 2, 3);
  EXPECT_DOUBLE_EQ(-2.0, dist);

  Swc_Tree_Node *tn3 = SwcTreeNode::MakePointer(1, 2, 3, 2);

  dist = SwcTreeNode::scaledDistance(tn1, tn3, 1, 2, 3);
  EXPECT_DOUBLE_EQ(9.8994949366116654, dist);

  dist = SwcTreeNode::scaledSurfaceDistance(tn1, tn3, 1, 2, 3);
  EXPECT_DOUBLE_EQ(6.8994949366116654, dist);

}

TEST(SwcTreeNode, Crossover)
{
  Swc_Tree_Node *center = SwcTreeNode::MakePointer(0, 0, 0, 1);
  Swc_Tree_Node *tn = SwcTreeNode::MakePointer(10, 0, 0, 1);
  SwcTreeNode::setParent(tn, center);
  tn = SwcTreeNode::MakePointer(-10, 0, 0, 1);
  SwcTreeNode::setParent(tn, center);
  tn = SwcTreeNode::MakePointer(0, 10, 0, 1);
  SwcTreeNode::setParent(tn, center);
  tn = SwcTreeNode::MakePointer(0, -10, 0, 1);
  SwcTreeNode::setParent(tn, center);

  std::map<Swc_Tree_Node*, Swc_Tree_Node*> matched =
      SwcTreeNode::crossoverMatch(center, TZ_PI_2);

  EXPECT_EQ(2, (int) matched.size());

  for (std::map<Swc_Tree_Node*, Swc_Tree_Node*>::const_iterator
       iter = matched.begin(); iter != matched.end(); ++iter) {
    std::cout << iter->first << " " << iter->second << std::endl;
  }
}

TEST(SwcTreeNode, neighbor)
{
  Swc_Tree_Node *center = SwcTreeNode::MakePointer(0, 0, 0, 1);
  Swc_Tree_Node *tn1 = SwcTreeNode::MakePointer(10, 0, 0, 1);
  SwcTreeNode::setParent(center, tn1);
  Swc_Tree_Node *tn2 =SwcTreeNode::MakePointer(20, 0, 0, 1);
  SwcTreeNode::setParent(tn1, tn2);
  Swc_Tree_Node *tn3 = SwcTreeNode::MakePointer(30, 0, 0, 1);
  SwcTreeNode::setParent(tn2, tn3);

  Swc_Tree_Node *tn = SwcTreeNode::continuousAncestor(center, 25.0);
  ASSERT_EQ(tn, tn3);
  tn = SwcTreeNode::continuousAncestor(center, 5.0);
  ASSERT_EQ(tn, tn1);

  tn = SwcTreeNode::continuousDescendent(tn1, 5.0);
  ASSERT_EQ(tn, center);

  tn = SwcTreeNode::continuousAncestor(center, 25.0);
  ASSERT_EQ(tn, tn3);
  tn = SwcTreeNode::continuousDescendent(tn3, 25.0);
  ASSERT_EQ(tn, center);

  tn = SwcTreeNode::continuousAncestor(center, 35.0);
  ASSERT_TRUE(tn == NULL);

  Swc_Tree_Node *tn4 = SwcTreeNode::MakePointer(30, 10, 0, 1);
  SwcTreeNode::setParent(tn4, tn3);
  tn = SwcTreeNode::continuousAncestor(center, 25.0);
  ASSERT_EQ(tn, tn3);

  tn = SwcTreeNode::continuousAncestor(center, 35.0);
  ASSERT_TRUE(tn == NULL);

  Swc_Tree_Node *tn5 = SwcTreeNode::MakePointer(20, 10, 0, 1);
  SwcTreeNode::setParent(tn5, tn2);
  tn = SwcTreeNode::continuousAncestor(center, 25.0);
  ASSERT_TRUE(tn == NULL);

  Swc_Tree_Node *tn6 = SwcTreeNode::MakePointer(0, 0, 10, 1);
  Swc_Tree_Node *tn7 = SwcTreeNode::MakePointer(0, 0, 20, 1);
  Swc_Tree_Node *tn8 = SwcTreeNode::MakePointer(0, 0, 30, 1);
  Swc_Tree_Node *tn9 = SwcTreeNode::MakePointer(0, 0, 40, 1);
  Swc_Tree_Node *tn10 = SwcTreeNode::MakePointer(0, 0, 50, 1);

  SwcTreeNode::setParent(tn6, center);
  SwcTreeNode::setParent(tn7, tn6);
  SwcTreeNode::setParent(tn8, tn7);
  SwcTreeNode::setParent(tn9, tn8);
  SwcTreeNode::setParent(tn10, tn8);

  tn = SwcTreeNode::continuousDescendent(center, 25.0);
  ASSERT_EQ(tn, tn8);

  tn = SwcTreeNode::continuousDescendent(center, 35.0);
  ASSERT_TRUE(tn == NULL);
}

TEST(SwcTreeNode, pathLength)
{
  Swc_Tree_Node *center = SwcTreeNode::MakePointer(0, 0, 0, 1);
  Swc_Tree_Node *tn1 = SwcTreeNode::MakePointer(10, 0, 0, 1);
  SwcTreeNode::setParent(tn1, center);
  Swc_Tree_Node *tn2 =SwcTreeNode::MakePointer(20, 0, 0, 1);
  SwcTreeNode::setParent(tn2, center);
  Swc_Tree_Node *tn3 = SwcTreeNode::MakePointer(30, 0, 0, 1);
  Swc_Tree_Node *tn4 = SwcTreeNode::MakePointer(30, 0, 0, 1);

  ASSERT_DOUBLE_EQ(10.0, SwcTreeNode::pathLength(tn1, center));
  ASSERT_DOUBLE_EQ(30.0, SwcTreeNode::pathLength(tn1, tn2));
  ASSERT_DOUBLE_EQ(0.0, SwcTreeNode::pathLength(tn1, tn1));

  ASSERT_DOUBLE_EQ(0.5, SwcTreeNode::pathLengthRatio(tn3, tn4, center));
  ASSERT_DOUBLE_EQ(0.25, SwcTreeNode::pathLengthRatio(center, tn2, tn1));
  ASSERT_DOUBLE_EQ(1.0, SwcTreeNode::pathLengthRatio(tn3, tn2, tn1));
  ASSERT_DOUBLE_EQ(0.0, SwcTreeNode::pathLengthRatio(tn2, tn3, tn1));
}

TEST(SwcTreeNode, Connection)
{
  std::set<Swc_Tree_Node*> nodeSet;
  ASSERT_TRUE(SwcTreeNode::isAllConnected(nodeSet));

  Swc_Tree_Node *center = SwcTreeNode::MakePointer(0, 0, 0, 1);
  Swc_Tree_Node *tn1 = SwcTreeNode::MakePointer(10, 0, 0, 1);

  nodeSet.insert(center);
  nodeSet.insert(tn1);
  ASSERT_FALSE(SwcTreeNode::isAllConnected(nodeSet));

  SwcTreeNode::setParent(tn1, center);
  ASSERT_TRUE(SwcTreeNode::isAllConnected(nodeSet));

  Swc_Tree_Node *tn2 = SwcTreeNode::MakePointer(10, 0, 0, 1);
  nodeSet.insert(tn2);
  ASSERT_FALSE(SwcTreeNode::isAllConnected(nodeSet));

  SwcTreeNode::setParent(tn2, tn1);
  ASSERT_TRUE(SwcTreeNode::isAllConnected(nodeSet));
}

#endif


#endif // ZSWCTREENODETEST_H

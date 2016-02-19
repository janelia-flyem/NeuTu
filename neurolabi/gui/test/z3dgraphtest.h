#ifndef Z3DGRAPHTEST_H
#define Z3DGRAPHTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "z3dgraph.h"
#include "zobject3d.h"

#ifdef _USE_GTEST_

TEST(Z3DGraph, basic)
{
  Z3DGraph graph;
  ASSERT_TRUE(graph.isEmpty());

  ZPointNetwork pointNetwork;
  pointNetwork.addPoint(ZPoint(0, 0, 0), 1.0);
  pointNetwork.addPoint(ZPoint(1, 1, 1), 2.0);
  pointNetwork.addEdge(0, 1, 1.0);
  graph.importPointNetwork(pointNetwork);

  ASSERT_EQ(1, (int) graph.getEdgeNumber());
  ASSERT_EQ(2, (int) graph.getNodeNumber());

  Z3DGraph graph2;
  pointNetwork.clear();
  pointNetwork.addPoint(ZPoint(2, 2, 2), 1.0);
  pointNetwork.addPoint(ZPoint(3, 4, 5), 2.0);
  pointNetwork.addEdge(0, 1, 1.0);
  graph2.importPointNetwork(pointNetwork);

  graph.append(graph2);
  ASSERT_EQ(2, (int) graph.getEdgeNumber());
  ASSERT_EQ(4, (int) graph.getNodeNumber());

  graph.clear();
  ZObject3d obj;
  obj.append(1, 2, 3);
  obj.append(4, 5, 6);
  graph.importObject3d(obj, 1.0, 1);
  ASSERT_EQ(0, (int) graph.getEdgeNumber());
  ASSERT_EQ(2, (int) graph.getNodeNumber());

}

#endif

#endif // Z3DGRAPHTEST_H

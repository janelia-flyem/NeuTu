#include "z3dgraphfactory.h"
#include "zcuboid.h"
#include "z3dgraph.h"
#include "zrect2d.h"

Z3DGraphFactory::Z3DGraphFactory()
{
}

Z3DGraph* Z3DGraphFactory::MakeGrid(
    const ZRect2d &rect, int ntick, double lineWidth)
{
  Z3DGraph *graph = new Z3DGraph;
  Z3DGraphNode node(
        rect.getFirstX(), rect.getFirstY(), rect.getZ(), lineWidth / 2.0);
  node.setColor(QColor(255, 255, 255));
  graph->addNode(node);

  node.set(rect.getFirstX(), rect.getLastY(), rect.getZ(), node.radius());
  graph->addNode(node);

  node.set(rect.getLastX(), rect.getFirstY(), rect.getZ(), node.radius());
  graph->addNode(node);

  node.set(rect.getLastX(), rect.getLastY(), rect.getZ(), node.radius());
  graph->addNode(node);

  Z3DGraphEdge edge;
  edge.useNodeColor(true);
  edge.setShape(GRAPH_CYLINDER);
  edge.setWidth(lineWidth);

  edge.setConnection(0, 1);
  graph->addEdge(edge);

  edge.setConnection(0, 2);
  graph->addEdge(edge);

  edge.setConnection(1, 3);
  graph->addEdge(edge);

  edge.setConnection(2, 3);
  graph->addEdge(edge);

  double interval = std::max(rect.getWidth(), rect.getHeight()) / (ntick + 1);

  //Create x ticks
  Z3DGraphNode node1;
  Z3DGraphNode node2;
  int v = 200;
  node1.setColor(QColor(v, v, v));
  node2.setColor(QColor(v, v, v));
  node1.setRadius(lineWidth / 4.0);
  node2.setRadius(lineWidth / 4.0);
  node1.setCenter(rect.getFirstX(), rect.getFirstY(), rect.getZ());
  node2.setCenter(rect.getLastX(), rect.getFirstY(), rect.getZ());

  for (double y = rect.getFirstY() + interval; y < rect.getLastY();
       y += interval) {
    node1.setY(y);
    node2.setY(y);
    graph->addEdge(node1, node2);
  }

  node1.setCenter(rect.getFirstX(), rect.getFirstY(), rect.getZ());
  node2.setCenter(rect.getFirstX(), rect.getLastY(), rect.getZ());

  for (double x = rect.getFirstX() + interval; x < rect.getLastX();
       x += interval) {
    node1.setX(x);
    node2.setX(x);
    graph->addEdge(node1, node2);
  }

  return graph;
}

Z3DGraph* Z3DGraphFactory::MakeBox(const ZCuboid &box, double radius)
{
  Z3DGraph *graph = new Z3DGraph;

  for (int i = 0; i < 8; ++i) {
    Z3DGraphNode node(box.corner(i), radius);
    node.setColor(QColor(128, 0, 128));
    graph->addNode(node);
  }

  Z3DGraphEdge edge;
  edge.useNodeColor(true);
  edge.setShape(GRAPH_CYLINDER);
//  edge.setStartColor(QColor(128, 128, 0));
//  edge.setEndColor(QColor(128, 128, 0));
  edge.setWidth(radius);

  edge.setConnection(0, 1);
  graph->addEdge(edge);

  edge.setConnection(0, 2);
  graph->addEdge(edge);

  edge.setConnection(2, 3);
  graph->addEdge(edge);

  edge.setConnection(1, 3);
  graph->addEdge(edge);

  edge.setConnection(4, 5);
  graph->addEdge(edge);

  edge.setConnection(5, 7);
  graph->addEdge(edge);

  edge.setConnection(6, 7);
  graph->addEdge(edge);

  edge.setConnection(4, 6);
  graph->addEdge(edge);

  edge.setConnection(0, 4);
  graph->addEdge(edge);

  edge.setConnection(1, 5);
  graph->addEdge(edge);

  edge.setConnection(2, 6);
  graph->addEdge(edge);

  edge.setConnection(3, 7);
  graph->addEdge(edge);

  return graph;
}

#include "z3dgraphfactory.h"
#include "geometry/zcuboid.h"
#include "z3dgraph.h"
#include "zrect2d.h"
#include "geometry/zintcuboid.h"
#include "zswctree.h"
#include "znormcolormap.h"

Z3DGraphFactory::Z3DGraphFactory()
{
  init();
}

void Z3DGraphFactory::init()
{
  m_shapeHint = GRAPH_LINE;
  m_nodeRadiusHint = 1.0;
  m_edgeWidthHint = 1.0;
  m_nodeColorHint = QColor(128, 128, 128, 255);
  m_edgeColorHint = QColor(128, 128, 128, 255);
}

Z3DGraph* Z3DGraphFactory::MakeGrid(
    const ZRect2d &rect, int ntick, double lineWidth)
{
  Z3DGraph *graph = new Z3DGraph;
  Z3DGraphNode node(
        rect.getMinX(), rect.getMinY(), rect.getZ(), lineWidth / 2.0);
  node.setColor(QColor(255, 255, 255));
  graph->addNode(node);

  node.set(rect.getMinX(), rect.getMaxY(), rect.getZ(), node.radius());
  graph->addNode(node);

  node.set(rect.getMaxX(), rect.getMinY(), rect.getZ(), node.radius());
  graph->addNode(node);

  node.set(rect.getMaxX(), rect.getMaxY(), rect.getZ(), node.radius());
  graph->addNode(node);

  Z3DGraphEdge edge;
  edge.useNodeColor(true);
  edge.setShape(GRAPH_LINE);
  edge.setWidth(lineWidth / 4.0);

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
  node1.setCenter(rect.getMinX(), rect.getMinY(), rect.getZ());
  node2.setCenter(rect.getMaxX(), rect.getMinY(), rect.getZ());

  for (double y = rect.getMinY() + interval; y < rect.getMaxY();
       y += interval) {
    node1.setY(y);
    node2.setY(y);
    graph->addEdge(node1, node2, GRAPH_LINE);
//    node1.setText(QString("%1").arg(y));
  }

  node1.setCenter(rect.getMinX(), rect.getMinY(), rect.getZ());
  node2.setCenter(rect.getMinX(), rect.getMaxY(), rect.getZ());

  for (double x = rect.getMinX() + interval; x < rect.getMaxX();
       x += interval) {
    node1.setX(x);
    node2.setX(x);
    graph->addEdge(node1, node2, GRAPH_LINE);

//    node1.setText(QString("%1").arg(x));
  }

//  Z3DGraphNode textNode;
//  textNode.setCenter(rect.get, rect.getFirstY(), rect.getZ());
//  textNode.setText(QString("Grid interval: %1 px").arg(interval));
//  graph->addNode(textNode);


  return graph;
}

Z3DGraph* Z3DGraphFactory::MakeQuadDiag(
    const ZPoint &pt1, const ZPoint &pt2,
    const ZPoint &pt3, const ZPoint &pt4, Z3DGraph *graph)
{
  if (graph == nullptr) {
    graph = new Z3DGraph;
  }

  int startIndex = graph->getNodeNumber();

  Z3DGraphNode node(pt1, 0.0);
  node.setColor(QColor(0, 0, 255));
  graph->addNode(node);

  node.setCenter(pt2);
  graph->addNode(node);

  node.setCenter(pt3);
  graph->addNode(node);

  node.setCenter(pt4);
  graph->addNode(node);

  Z3DGraphEdge edge;
  edge.useNodeColor(true);
  edge.setShape(GRAPH_LINE);
  edge.setWidth(2.0);

  edge.setRelativeConnection(startIndex, 0, 1);
  graph->addEdge(edge);

  edge.setRelativeConnection(startIndex, 1, 2);
  graph->addEdge(edge);

  edge.setRelativeConnection(startIndex, 2, 3);
  graph->addEdge(edge);

  edge.setRelativeConnection(startIndex, 0, 3);
  graph->addEdge(edge);

  edge.setRelativeConnection(startIndex, 0, 2);
  graph->addEdge(edge);

  edge.setRelativeConnection(startIndex, 1, 3);
  graph->addEdge(edge);

  return graph;
}

Z3DGraph* Z3DGraphFactory::MakeQuadCross(const ZPoint &pt1, const ZPoint &pt2,
                                         const ZPoint &pt3, const ZPoint &pt4)
{
  Z3DGraph *graph = new Z3DGraph;
  Z3DGraphNode node(pt1, 0.0);
  node.setColor(QColor(0, 0, 255));
  graph->addNode(node);

  node.setCenter(pt2);
  graph->addNode(node);

  node.setCenter(pt3);
  graph->addNode(node);

  node.setCenter(pt4);
  graph->addNode(node);

  node.setCenter((pt1 + pt2) * 0.5);
  graph->addNode(node);

  node.setCenter((pt2 + pt3) * 0.5);
  graph->addNode(node);

  node.setCenter((pt3 + pt4) * 0.5);
  graph->addNode(node);

  node.setCenter((pt4 + pt1) * 0.5);
  graph->addNode(node);


  Z3DGraphEdge edge;
  edge.useNodeColor(true);
  edge.setShape(GRAPH_LINE);
  edge.setWidth(2.0);

  edge.setConnection(0, 1);
  graph->addEdge(edge);

  edge.setConnection(1, 2);
  graph->addEdge(edge);

  edge.setConnection(2, 3);
  graph->addEdge(edge);

  edge.setConnection(0, 3);
  graph->addEdge(edge);

  edge.setWidth(5.0);
  edge.setConnection(4, 6);
  graph->addEdge(edge);

  edge.setConnection(5, 7);
  graph->addEdge(edge);

  return graph;
}

namespace {

void add_box_edge(Z3DGraph *graph, Z3DGraphEdge edge, int startIndex)
{
  edge.setRelativeConnection(startIndex, 0, 1);
  graph->addEdge(edge);

  edge.setRelativeConnection(startIndex, 0, 2);
  graph->addEdge(edge);

  edge.setRelativeConnection(startIndex, 2, 3);
  graph->addEdge(edge);

  edge.setRelativeConnection(startIndex, 1, 3);
  graph->addEdge(edge);

  edge.setRelativeConnection(startIndex, 4, 5);
  graph->addEdge(edge);

  edge.setRelativeConnection(startIndex, 5, 7);
  graph->addEdge(edge);

  edge.setRelativeConnection(startIndex, 6, 7);
  graph->addEdge(edge);

  edge.setRelativeConnection(startIndex, 4, 6);
  graph->addEdge(edge);

  edge.setRelativeConnection(startIndex, 0, 4);
  graph->addEdge(edge);

  edge.setRelativeConnection(startIndex, 1, 5);
  graph->addEdge(edge);

  edge.setRelativeConnection(startIndex, 2, 6);
  graph->addEdge(edge);

  edge.setRelativeConnection(startIndex, 3, 7);
  graph->addEdge(edge);
}

}

Z3DGraph* Z3DGraphFactory::MakeBox(
    const ZCuboid &box, double radius, Z3DGraph *graph)
{
  if (graph == nullptr) {
    graph = new Z3DGraph;
  }

  int startIndex = graph->getNodeNumber();

  for (int i = 0; i < 8; ++i) {
    Z3DGraphNode node(box.getCorner(i), radius);
    node.setColor(QColor(128, 0, 128));
    graph->addNode(node);
  }

  if (radius > 0.0) {
    Z3DGraphEdge edge;
    edge.useNodeColor(true);
    edge.setShape(GRAPH_CYLINDER);
    //  edge.setStartColor(QColor(128, 128, 0));
    //  edge.setEndColor(QColor(128, 128, 0));
    edge.setWidth(radius);

    add_box_edge(graph, edge, startIndex);
  }

  return graph;
}

Z3DGraph* Z3DGraphFactory::MakeBox(const ZIntCuboid &box, double radius)
{
  Z3DGraph *graph = new Z3DGraph;

  int startIndex = graph->getNodeNumber();

  for (int i = 0; i < 8; ++i) {
    Z3DGraphNode node(box.getCorner(i).toPoint(), 0);
    node.setColor(QColor(128, 0, 128));
    graph->addNode(node);
  }

  if (radius > 0.0) {
    Z3DGraphEdge edge;
    edge.useNodeColor(true);
    edge.setShape(GRAPH_CYLINDER);
    //  edge.setStartColor(QColor(128, 128, 0));
    //  edge.setEndColor(QColor(128, 128, 0));
    edge.setWidth(radius);


    add_box_edge(graph, edge, startIndex);
  }

  return graph;
}

Z3DGraph* Z3DGraphFactory::makeBox(const ZIntCuboid &box,  Z3DGraph *graph)
{
  if (graph == nullptr) {
    graph = new Z3DGraph;
  }

  int startIndex = graph->getNodeNumber();

  for (int i = 0; i < 8; ++i) {
    Z3DGraphNode node(box.getCorner(i).toPoint(), m_nodeRadiusHint);
    node.setColor(m_nodeColorHint);
    graph->addNode(node);
  }

  Z3DGraphEdge edge;
  edge.useNodeColor(false);
  edge.setShape(m_shapeHint);
  edge.setStartColor(m_edgeColorHint);
  edge.setEndColor(m_edgeColorHint);
  edge.setWidth(m_edgeWidthHint);

  add_box_edge(graph, edge, startIndex);

  return graph;
}

Z3DGraph* Z3DGraphFactory::makeBox(const ZCuboid &box, Z3DGraph *graph)
{
  if (graph == nullptr) {
    graph = new Z3DGraph;
  }

  int startIndex = graph->getNodeNumber();

  for (int i = 0; i < 8; ++i) {
    Z3DGraphNode node(box.getCorner(i), m_nodeRadiusHint);
    node.setColor(m_nodeColorHint);
    graph->addNode(node);
  }

  Z3DGraphEdge edge;
  edge.useNodeColor(false);
  edge.setShape(m_shapeHint);
  edge.setStartColor(m_edgeColorHint);
  edge.setEndColor(m_edgeColorHint);
//  edge.setStartColor(QColor(128, 128, 0));
//  edge.setEndColor(QColor(128, 128, 0));
  edge.setWidth(m_edgeWidthHint);

  add_box_edge(graph, edge, startIndex);

  return graph;
}

#define ADD_EDGE(v1, v2) \
  if (edgeAdded[v1][v2] == false) {\
    edge.setConnection(v1, v2);\
    graph->addEdge(edge);\
    edgeAdded[v1][v2] = true;\
  }

Z3DGraph* Z3DGraphFactory::makeFaceGraph(
    const ZCuboid &box, const std::vector<int> &faceArray)
{
  bool edgeAdded[8][8];
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      edgeAdded[i][j] = false;
    }
  }

  Z3DGraph *graph = new Z3DGraph;

  for (int i = 0; i < 8; ++i) {
    Z3DGraphNode node(box.getCorner(i), m_nodeRadiusHint);
    node.setColor(m_nodeColorHint);
    graph->addNode(node);
  }

  Z3DGraphEdge edge;
  edge.useNodeColor(false);
  edge.setShape(m_shapeHint);
  edge.setStartColor(m_edgeColorHint);
  edge.setEndColor(m_edgeColorHint);

  for (size_t i = 0; i < faceArray.size(); ++i) {
    switch (faceArray[i]) {
    case 0:
      ADD_EDGE(0, 2);
      ADD_EDGE(0, 4);
      ADD_EDGE(2, 6);
      ADD_EDGE(4, 6);
      break;
    case 1:
      ADD_EDGE(1, 5);
      ADD_EDGE(1, 3);
      ADD_EDGE(3, 7);
      ADD_EDGE(5, 7);
      break;
    case 2:
      ADD_EDGE(0, 1);
      ADD_EDGE(0, 4);
      ADD_EDGE(1, 5);
      ADD_EDGE(4, 5);
      break;
    case 3:
      ADD_EDGE(2, 3);
      ADD_EDGE(2, 6);
      ADD_EDGE(3, 7);
      ADD_EDGE(6, 7);
      break;
    case 4:
      ADD_EDGE(0, 1);
      ADD_EDGE(0, 2);
      ADD_EDGE(2, 3);
      ADD_EDGE(1, 3);
      break;
    case 5:
      ADD_EDGE(4, 5);
      ADD_EDGE(4, 6);
      ADD_EDGE(6, 7);
      ADD_EDGE(5, 7);
      break;
    default:
      break;
    }
  }

  return graph;
}

Z3DGraph* Z3DGraphFactory::makeFaceGraph(
    const ZIntCuboid &box, const std::vector<int> &faceArray)
{
  bool edgeAdded[8][8];
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      edgeAdded[i][j] = false;
    }
  }

  Z3DGraph *graph = new Z3DGraph;

  for (int i = 0; i < 8; ++i) {
    Z3DGraphNode node(box.getCorner(i).toPoint(), m_nodeRadiusHint);
    node.setColor(m_nodeColorHint);
    graph->addNode(node);
  }

  Z3DGraphEdge edge;
  edge.useNodeColor(false);
  edge.setShape(m_shapeHint);
  edge.setStartColor(m_edgeColorHint);
  edge.setEndColor(m_edgeColorHint);

  for (size_t i = 0; i < faceArray.size(); ++i) {
    switch (faceArray[i]) {
    case 0:
      ADD_EDGE(0, 2);
      ADD_EDGE(0, 4);
      ADD_EDGE(2, 6);
      ADD_EDGE(4, 6);
      break;
    case 1:
      ADD_EDGE(1, 5);
      ADD_EDGE(1, 3);
      ADD_EDGE(3, 7);
      ADD_EDGE(5, 7);
      break;
    case 2:
      ADD_EDGE(0, 1);
      ADD_EDGE(0, 4);
      ADD_EDGE(1, 5);
      ADD_EDGE(4, 5);
      break;
    case 3:
      ADD_EDGE(2, 3);
      ADD_EDGE(2, 6);
      ADD_EDGE(3, 7);
      ADD_EDGE(6, 7);
      break;
    case 4:
      ADD_EDGE(0, 1);
      ADD_EDGE(0, 2);
      ADD_EDGE(2, 3);
      ADD_EDGE(1, 3);
      break;
    case 5:
      ADD_EDGE(4, 5);
      ADD_EDGE(4, 6);
      ADD_EDGE(6, 7);
      ADD_EDGE(5, 7);
      break;
    default:
      break;
    }
  }

  return graph;
}

Z3DGraph* Z3DGraphFactory::makeBoundingBox(const ZIntCuboid &box, const std::vector<int> &faceArray)
{
    bool edgeAdded[8][8];
    for (int i = 0; i < 8; ++i) {
      for (int j = 0; j < 8; ++j) {
        edgeAdded[i][j] = false;
      }
    }

    Z3DGraph *graph = new Z3DGraph;

    for (int i = 0; i < 8; ++i) {
      Z3DGraphNode node(box.getCorner(i).toPoint(), m_nodeRadiusHint);
      node.setColor(m_nodeColorHint);
      graph->addNode(node);
    }

    Z3DGraphEdge edge;
    edge.useNodeColor(false);
    edge.setShape(m_shapeHint);
    edge.setStartColor(m_edgeColorHint);
    edge.setEndColor(m_edgeColorHint);

    for (size_t i = 0; i < faceArray.size(); ++i) {
      switch (faceArray[i]) {
      case 0:
        ADD_EDGE(0, 2);
        ADD_EDGE(0, 4);
        ADD_EDGE(2, 6);
        ADD_EDGE(4, 6);
        break;
      case 1:
        ADD_EDGE(1, 5);
        ADD_EDGE(1, 3);
        ADD_EDGE(3, 7);
        ADD_EDGE(5, 7);
        break;
      case 2:
        ADD_EDGE(0, 1);
        ADD_EDGE(0, 4);
        ADD_EDGE(1, 5);
        ADD_EDGE(4, 5);
        break;
      case 3:
        ADD_EDGE(2, 3);
        ADD_EDGE(2, 6);
        ADD_EDGE(3, 7);
        ADD_EDGE(6, 7);
        break;
      case 4:
        ADD_EDGE(0, 1);
        ADD_EDGE(0, 2);
        ADD_EDGE(2, 3);
        ADD_EDGE(1, 3);
        break;
      case 5:
        ADD_EDGE(4, 5);
        ADD_EDGE(4, 6);
        ADD_EDGE(6, 7);
        ADD_EDGE(5, 7);
        break;
      default:
        break;
      }
    }

    return graph;
}

Z3DGraph Z3DGraphFactory::MakeSwcGraph(const ZSwcTree &tree, double edgeWidth)
{
  Z3DGraph graph;
  ZSwcTree::DepthFirstIterator treeIter(&tree);
  treeIter.excludeVirtual(true);
  std::vector<Swc_Tree_Node*> nodeArray;
  int index = 0;
  for (Swc_Tree_Node *tn = treeIter.begin(); tn != NULL;
       tn = treeIter.next(), ++index) {
    nodeArray.push_back(tn);
    tn->index = index;
  }

  for (Swc_Tree_Node *tn : nodeArray) {
    Z3DGraphNode node(SwcTreeNode::center(tn), SwcTreeNode::radius(tn));
    node.setColor(tree.getColor());
    graph.addNode(node);
  }

  for (Swc_Tree_Node *tn : nodeArray) {
    if (!SwcTreeNode::isRoot(tn)) {
      graph.addEdge(tn->parent->index, tn->index, edgeWidth, GRAPH_LINE);
    }
  }

  return graph;
}

Z3DGraph Z3DGraphFactory::MakeSwcFeatureGraph(const ZSwcTree &tree)
{
  Z3DGraph graph;
  ZSwcTree::DepthFirstIterator treeIter(&tree);
  treeIter.excludeVirtual(true);
  std::vector<Swc_Tree_Node*> nodeArray;
  int index = 0;
  for (Swc_Tree_Node *tn = treeIter.begin(); tn != NULL;
       tn = treeIter.next(), ++index) {
    nodeArray.push_back(tn);
    tn->index = index;
  }

  ZNormColorMap colorMap;
  for (Swc_Tree_Node *tn : nodeArray) {
    Z3DGraphNode node(SwcTreeNode::center(tn), SwcTreeNode::radius(tn));
    node.setColor(colorMap.mapColor(SwcTreeNode::feature(tn)));
    graph.addNode(node);
  }

  for (Swc_Tree_Node *tn : nodeArray) {
    if (!SwcTreeNode::isRoot(tn)) {
      graph.addEdge(tn->parent->index, tn->index, GRAPH_LINE);
    }
  }

  return graph;
}

Z3DGraph* Z3DGraphFactory::makeQuadrilateral(
    const ZPoint &pt1, const ZPoint &pt2,
    const ZPoint &pt3, const ZPoint &pt4, Z3DGraph *graph)
{
  if (graph == nullptr) {
    graph = new Z3DGraph;
  }

  int startIndex = graph->getNodeNumber();


  Z3DGraphNode node(pt1, m_nodeRadiusHint);
  node.setColor(m_nodeColorHint);
  graph->addNode(node);

  node.setCenter(pt2);
  graph->addNode(node);

  node.setCenter(pt3);
  graph->addNode(node);

  node.setCenter(pt4);
  graph->addNode(node);

  Z3DGraphEdge edge;
  edge.useNodeColor(false);
  edge.setShape(m_shapeHint);
  edge.setStartColor(m_edgeColorHint);
  edge.setEndColor(m_edgeColorHint);
  edge.setWidth(m_edgeWidthHint);

  edge.setRelativeConnection(startIndex, 0, 1);
  graph->addEdge(edge);

  edge.setRelativeConnection(startIndex, 1, 2);
  graph->addEdge(edge);

  edge.setRelativeConnection(startIndex, 2, 3);
  graph->addEdge(edge);

  edge.setRelativeConnection(startIndex, 0, 3);
  graph->addEdge(edge);

  return graph;
}

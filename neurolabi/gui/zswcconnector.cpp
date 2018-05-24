#include "zswcconnector.h"
#include "zgraph.h"
#include "tz_u8array.h"
#include "zswctree.h"
#include "zswcpath.h"

using namespace std;

ZSwcConnector::ZSwcConnector()
{
  init();
}

void ZSwcConnector::init()
{
  m_minDist = 20.0;
  m_surfaceDist = false;
  m_dist = 0.0;
}

void ZSwcConnector::useSurfaceDist(bool on)
{
  m_surfaceDist = on;
}

double ZSwcConnector::computeDistance(
    const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2) const
{
  double d = 0.0;

  if (m_surfaceDist) {
    d = SwcTreeNode::scaledSurfaceDistance(
          tn1, tn2, m_resolution.voxelSizeX(), m_resolution.voxelSizeY(),
          m_resolution.voxelSizeZ());
  } else {
    d = SwcTreeNode::scaledDistance(
          tn1, tn2, m_resolution.voxelSizeX(), m_resolution.voxelSizeY(),
          m_resolution.voxelSizeZ());
  }

  if (d < 0.0) {
    double r1 = SwcTreeNode::radius(tn1);
    double r2 = SwcTreeNode::radius(tn2);
    if (r1 == 0.0 || r2 == 0.0) {
      d = 0.0;
    } else {
      d = SwcTreeNode::scaledDistance(
            tn1, tn2, m_resolution.voxelSizeX(), m_resolution.voxelSizeY(),
            m_resolution.voxelSizeZ()) / r1 / r2;
    }
  }

  return d;
}


pair<Swc_Tree_Node*, Swc_Tree_Node*> ZSwcConnector::identifyConnection(
      const ZSwcPath &hook, const ZSwcTree &loop)
{
  pair<Swc_Tree_Node*, Swc_Tree_Node*> conn;

  if (!hook.empty()) {
    loop.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
    Swc_Tree_Node *head = hook.front();
    Swc_Tree_Node *tail = hook.back();
    m_dist = Infinity;
    for (const Swc_Tree_Node *tn = loop.begin(); tn != NULL; tn = loop.next()) {
      if (SwcTreeNode::isRegular(tn)) {
        double d = computeDistance(head, tn);
        if (d < m_dist) {
          m_dist = d;
          conn.first = head;
          conn.second = const_cast<Swc_Tree_Node*>(tn);
        }
        if (tail != NULL) {
          d = computeDistance(tail, tn);
          if (d < m_dist) {
            m_dist = d;
            conn.first = tail;
            conn.second = const_cast<Swc_Tree_Node*>(tn);
          }
        }
      }
    }
  }

  return conn;
}

pair<Swc_Tree_Node*, Swc_Tree_Node*> ZSwcConnector::identifyConnection(
      const ZSwcPath &hook, const vector<ZSwcTree*> &loop)
{
  double minDist = Infinity;
  pair<Swc_Tree_Node*, Swc_Tree_Node*> bestConn;
  for (vector<ZSwcTree*>::const_iterator iter = loop.begin();
       iter != loop.end(); ++iter) {
    pair<Swc_Tree_Node*, Swc_Tree_Node*> conn =
        identifyConnection(hook, *(*iter));
    if (getConnDist() < m_minDist &&  getConnDist() < minDist) {
      minDist = getConnDist();
      bestConn = conn;
    }
  }

  setConnDist(minDist);

  return bestConn;
}

pair<Swc_Tree_Node*, Swc_Tree_Node*> ZSwcConnector::identifyConnection(
      const ZSwcTree &hook, const ZSwcTree &loop)
{
  pair<Swc_Tree_Node*, Swc_Tree_Node*> conn;

  loop.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  m_dist = Infinity;

  ZSwcTree::TerminalIterator hookIter(&hook);

  ZSwcTree::DepthFirstIterator loopIter(&loop);
  while (loopIter.hasNext()) {
    Swc_Tree_Node *tn = loopIter.next();
    if (SwcTreeNode::isRegular(tn)) {
//      hookIter.begin();
      hookIter.restart();
      while (hookIter.hasNext()) {
        Swc_Tree_Node *head = hookIter.next();
        double d = computeDistance(head, tn);
        /*
        double d = SwcTreeNode::scaledDistance(
              head, tn, m_resolution.voxelSizeX(), m_resolution.voxelSizeY(),
              m_resolution.voxelSizeZ());
              */
        if (d < m_dist) {
          m_dist = d;
          conn.first = head;
          conn.second = const_cast<Swc_Tree_Node*>(tn);
        }
      }
    }
  }

  return conn;
}

std::pair<Swc_Tree_Node*, Swc_Tree_Node*> ZSwcConnector::identifyConnection(
      const ZSwcTree &hook, const vector<ZSwcTree*> &loop)
{
  double minDist = Infinity;
  pair<Swc_Tree_Node*, Swc_Tree_Node*> bestConn;

  for (vector<ZSwcTree*>::const_iterator iter = loop.begin();
       iter != loop.end(); ++iter) {
    pair<Swc_Tree_Node*, Swc_Tree_Node*> conn =
        identifyConnection(hook, *(*iter));
    if (getConnDist() < m_minDist &&  getConnDist() < minDist) {
      minDist = getConnDist();
      bestConn = conn;
    }
  }

  setConnDist(minDist);

  return bestConn;
}

#define NUMBER_OF_NEIGHBORS(v, neighbors) neighbors[v][0]
#define NEIGHBOR_OF(v, n, neighbors) neighbors[v][n]

ZGraph* ZSwcConnector::buildConnection(
    const vector<Swc_Tree_Node *> &nodeArray)
{
  std::vector<bool> isOrphan(nodeArray.size(), true);

  std::vector<Swc_Tree_Node*> regularRootArray(nodeArray.size());
  for (size_t i = 0; i < nodeArray.size(); ++i) {
    regularRootArray[i] = SwcTreeNode::regularRoot(nodeArray[i]);
  }

  ZGraph *graph = new ZGraph(ZGraph::UNDIRECTED_WITH_WEIGHT);
  for (size_t i = 0; i < nodeArray.size(); ++i) {
    for (size_t j = i + 1; j < nodeArray.size(); ++j) {
      Swc_Tree_Node *tn1 = nodeArray[i];
      Swc_Tree_Node *tn2 = nodeArray[j];
      if (SwcTreeNode::isRegular(tn1) && SwcTreeNode::isRegular(tn2)) {
        if (regularRootArray[i] != regularRootArray[j]) {
//          double w = SwcTreeNode::distance(nodeArray[i], nodeArray[j]);
          double w = computeDistance(tn1, tn2);
          /*
          double w = SwcTreeNode::scaledDistance(
                tn1, tn2, m_resolution.voxelSizeX(), m_resolution.voxelSizeY(),
                m_resolution.voxelSizeZ());
                */
          if (w <= m_minDist) {
            graph->addEdgeFast(i, j, w + 0.1);
          }
        } else {
          if (isOrphan[j]) {
            graph->addEdgeFast(i, j, 0);
            isOrphan[j] = false;
          }
        }
      }
    }
  }

  if (graph->size() > 0) {
    graph->toMst(); 

    //Remove zero weight edge
    std::vector<int> edgeList;
    for (size_t i = 0; i < graph->size(); ++i) {
      if (graph->getEdgeWeight(i) == 0.0) {
        edgeList.push_back(i);
      }
    }
    graph->removeEdge(edgeList);

#ifdef _DEBUG_
  graph->print();
#endif
#if 1
    if (graph->size() > 1) {
      //Breadth-first sort of edges
      Graph_Workspace *gw = New_Graph_Workspace();

      ZGraph *tmpGraph = graph->clone();
      graph->clear();
      graph->setDirected(true);

      Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_VLIST);
      Graph_Workspace_Load(gw, tmpGraph->getRawGraph());
      Graph_Workspace_Alloc(gw, GRAPH_WORKSPACE_VLIST);

      Arrayqueue aq;
      Arrayqueue_Attach(&aq, gw->vlist, gw->nvertex);
      Initialize_Arrayqueue(&aq);
      int root = tmpGraph->getEdgeBegin(0);

      int **neighbor_list = Graph_Neighbor_List(tmpGraph->getRawGraph(), gw);
      if (NUMBER_OF_NEIGHBORS(neighbor_list, root) > 0) {
        /* alloc <mask> */
        uint8_t *mask = u8array_malloc(gw->nvertex);
        int i;
        for (i = 0; i < gw->nvertex; i++) {
          mask[i] = 0;
        }

        for (int edgeIndex = 0; edgeIndex < tmpGraph->getEdgeNumber(); ++edgeIndex) {
          root = tmpGraph->getEdgeBegin(edgeIndex);
          if (mask[root] == 0) {
            Arrayqueue_Add_Last(&aq, root);
            mask[root] = 1;
            int next = root;
            do {
              for (i = 1; i <= NUMBER_OF_NEIGHBORS(next, neighbor_list); i++) {
                int node = NEIGHBOR_OF(next, i, neighbor_list);
                if (mask[node] == 0) {
                  Arrayqueue_Add_Last(&aq, node);
                  mask[node] = 1;
                  graph->addEdge(next, node);
                }
              }
              next = aq.array[next];
            } while (next >= 0);
          }
        }

        /* free <mask> */
        free(mask);
      }

      Kill_Graph_Workspace(gw);
      delete tmpGraph;
    }
#endif
  }

#ifdef _DEBUG_
  graph->print();
#endif

  return graph;
}

ZGraph* ZSwcConnector::buildConnection(const set<Swc_Tree_Node *> &nodeSet)
{
  vector<Swc_Tree_Node*> nodeArray;
  for (set<Swc_Tree_Node *>::const_iterator iter = nodeSet.begin();
       iter != nodeSet.end(); ++iter) {
    nodeArray.push_back(*iter);
  }

  return buildConnection(nodeArray);
}

void ZSwcConnector::setResolution(const ZResolution &resolution)
{
  m_resolution = resolution;
}

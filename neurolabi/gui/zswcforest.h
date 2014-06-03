#ifndef ZSWCFOREST_H
#define ZSWCFOREST_H

#include <vector>
#include <utility>
#include "tz_graph.h"
#include "tz_swc_tree.h"

class ZSwcTree;

class ZSwcForest : public std::vector<ZSwcTree*>
{
public:
  ZSwcForest();
  ~ZSwcForest();

  /*!
   * \brief Convert the forest into a tree object.
   *
   * The forest becomes empty after conversion. It returns NULL if there's no
   * regular node.
   *
   * \return The tree object containing all regular nodes in the tree elements.
   */
  ZSwcTree* toSwcTree();

public:
  //Graph* buildConnectionGraph(bool mst = false, double distThre = -1.0);
  //ZSwcTree* merge();
  void print();

private:
  Graph *m_graph;
  Graph_Workspace *m_workspace;
  std::vector<std::pair<Swc_Tree_Node*, Swc_Tree_Node*> > m_connection;
};

#endif // ZSWCFOREST_H

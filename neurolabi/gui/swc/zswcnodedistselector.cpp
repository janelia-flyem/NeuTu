#include "zswcnodedistselector.h"

#include "zswctree.h"

ZSwcNodeDistSelector::ZSwcNodeDistSelector()
{
}

ZSwcTreeNodeArray ZSwcNodeDistSelector::select(const ZSwcTree &tree) const
{
  tree.setLabel(0);

  tree.updateIterator(SWC_TREE_ITERATOR_LEAF);

  ZSwcTreeNodeArray nodeArray;

  for (Swc_Tree_Node *tn = tree.begin(); tn != NULL; tn = tree.next()) {
    nodeArray.push_back(tn);
    double dist = 0;
    Swc_Tree_Node *parent = SwcTreeNode::parent(tn);
    if (SwcTreeNode::label(parent) == 1) {
      continue;
    }

    dist += SwcTreeNode::length(tn);
    while (SwcTreeNode::isRegular(parent)) {
      if (dist >= m_minDistance) {
        nodeArray.push_back(parent);
        dist = 0;
        SwcTreeNode::setLabel(parent, 1);
      }
      dist += SwcTreeNode::length(parent);

      parent = SwcTreeNode::parent(parent);
      if (SwcTreeNode::label(parent) == 1) {
        break;
      }
    }
  }

  return nodeArray;
}

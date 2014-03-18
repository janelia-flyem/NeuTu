#ifndef ZSWCPRUNER_H
#define ZSWCPRUNER_H

#include "tz_swc_tree.h"
#include <vector>

class ZSwcTree;

class ZSwcPruner
{
public:
  ZSwcPruner();

  /*!
   * \brief Prune an SWC tree.
   *
   * \return Number of terminal branches that are removed
   */
  int prune(ZSwcTree *tree) const;

  inline void setMinLength(double length) {
    m_minLength = length;
  }

private:
  static Swc_Tree_Node* getParentBranchPoint(
      const std::vector<int> &neighborCount, Swc_Tree_Node *tn);

  static Swc_Tree_Node* getMinWeightedNode(
      const std::vector<Swc_Tree_Node*> &nodeArray,
      const std::vector<bool> &removed);

  static void removeTerminalBranch(Swc_Tree_Node *tn);
  static void reduceTerminalBranch(Swc_Tree_Node *tn);

private:
  double m_minLength;
};

#endif // ZSWCPRUNER_H

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
   * It removes short branches from a tree.
   *
   * \return Number of terminal branches that are removed.
   */
  int prune(ZSwcTree *tree) const;

  void removeOrphanBlob(ZSwcTree *tree) const;

  /*!
   * \brief Set the minimal length of preserved branches.
   *
   * All branches shorter than than \a length will be removed recursively. The
   * orphan branches might be treated differently.
   *
   * \param length The length threshold.
   */
  inline void setMinLength(double length) {
    m_minLength = length;
  }

  /*!
   * \brief Set the option of removing orphans
   *
   * An orphan branch is defined as a tree without branching.
   *
   * \param removing Remove the orphans shorter
   */
  inline void setRemovingOrphan(bool removing) {
    m_removingOrphan = removing;
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
  int m_minOrphanCount; //minimal count for orphan stat
  bool m_removingOrphan;
};

#endif // ZSWCPRUNER_H

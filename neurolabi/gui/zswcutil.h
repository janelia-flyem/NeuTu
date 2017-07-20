#ifndef ZSWCUTIL_H
#define ZSWCUTIL_H

#include <vector>
#include "zswctree.h"

namespace ZSwc
{
/*!
 * \brief Extract overlapping nodes.
 *
 * Find nodes in \a tree2 that have significant overlap with nodes in \a tree1.
 */
std::vector<Swc_Tree_Node*> FindOverlapNode(
    const ZSwcTree *tree1, const ZSwcTree *tree2);
std::vector<Swc_Tree_Node*> FindOverlapNode(
    const ZSwcTree &tree1, const ZSwcTree &tree2);

void Subtract(ZSwcTree *tree1, const ZSwcTree *tree2);

template <template<class...> class container>
void SetType(container<Swc_Tree_Node*> nodeGroup, int type);
}

template <template<class...> class container>
void ZSwc::SetType(container<Swc_Tree_Node *> nodeGroup, int type)
{
  for (typename container<Swc_Tree_Node*>::iterator iter = nodeGroup.begin();
       iter != nodeGroup.end(); ++iter) {
    SwcTreeNode::setType(*iter, type);
  }
}

#endif // ZSWCUTIL_H

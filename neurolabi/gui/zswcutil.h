#ifndef ZSWCUTIL_H
#define ZSWCUTIL_H

#include <vector>
#include <set>
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

#if __cplusplus == 201103L
template <template<class...> class container>
void SetType(container<Swc_Tree_Node*> nodeGroup, int type);
#define _SWC_SET_TYPE_DEFINED 1
#else
void SetType(const std::set<Swc_Tree_Node*> &nodeSet, int type);
#endif

template <typename InputIterator>
void SetType(const InputIterator &first, const InputIterator &last, int type);
}

#if __cplusplus == 201103L
template <template<class...> class container>
void ZSwc::SetType(container<Swc_Tree_Node *> nodeGroup, int type)
{
  for (typename container<Swc_Tree_Node*>::iterator iter = nodeGroup.begin();
       iter != nodeGroup.end(); ++iter) {
    SwcTreeNode::setType(*iter, type);
  }
}
#endif

template <typename InputIterator>
void ZSwc::SetType(const InputIterator &first, const InputIterator &last, int type)
{
  for (InputIterator iter = first; iter != last; ++iter) {
    SwcTreeNode::setType(*iter, type);
  }
}

#endif // ZSWCUTIL_H

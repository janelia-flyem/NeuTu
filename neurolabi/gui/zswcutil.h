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
}

#endif // ZSWCUTIL_H

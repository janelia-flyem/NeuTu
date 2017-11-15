#include "zswcutil.h"

std::vector<Swc_Tree_Node*> ZSwc::FindOverlapNode(
    const ZSwcTree *tree1, const ZSwcTree *tree2)
{
  std::vector<Swc_Tree_Node*> result;

  if (tree1 == NULL || tree2 == NULL) {
    return result;
  }

  const std::vector<Swc_Tree_Node*> &nodeArray1 = tree1->getSwcTreeNodeArray();
  const std::vector<Swc_Tree_Node*> &nodeArray2 = tree2->getSwcTreeNodeArray();

  for (std::vector<Swc_Tree_Node*>::const_iterator iter2 = nodeArray2.begin();
       iter2 != nodeArray2.end(); ++iter2) {
    for (std::vector<Swc_Tree_Node*>::const_iterator iter1 = nodeArray1.begin();
         iter1 != nodeArray1.end(); ++iter1) {
      if (SwcTreeNode::hasSignificantOverlap(*iter1, *iter2)) {
        result.push_back(*iter2);
        break;
      }
    }
  }

  return result;
}

std::vector<Swc_Tree_Node*> ZSwc::FindOverlapNode(
    const ZSwcTree &tree1, const ZSwcTree &tree2)
{
  return FindOverlapNode(&tree1, &tree2);
}

void ZSwc::Subtract(ZSwcTree *tree1, const ZSwcTree *tree2)
{
  Swc_Tree_Subtract(tree1->data(), tree2->data());
}


#ifndef _SWC_SET_TYPE_DEFINED
void ZSwc::SetType(const std::set<Swc_Tree_Node *> &nodeSet, int type)
{
  SetType(nodeSet.begin(), nodeSet.end(), type);
}
#endif





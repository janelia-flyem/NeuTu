#include "zswctreenodeselector.h"
#include "ztextlinecompositer.h"

ZSwcTreeNodeSelector::ZSwcTreeNodeSelector()
{
}

void ZSwcTreeNodeSelector::print() const
{
  ZTextLineCompositer comp;
  comp.appendLine("Selector:", 0);
  comp.appendLine("Selected:", 1);
  for (std::set<Swc_Tree_Node*>::const_iterator iter = m_selectedSet.begin();
       iter != m_selectedSet.end(); ++iter) {
    const Swc_Tree_Node *tn = *iter;
    comp.appendLine(SwcTreeNode::toString(tn), 2);
  }
  comp.appendLine("Deselected:", 1);
  for (std::set<Swc_Tree_Node*>::const_iterator iter = m_deselectedSet.begin();
       iter != m_deselectedSet.end(); ++iter) {
    const Swc_Tree_Node *tn = *iter;
    comp.appendLine(SwcTreeNode::toString(tn), 2);
  }

  comp.print(2);
}

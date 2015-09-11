#include "zswcsignalfitter.h"
#include "zstack.hxx"
#include "zswctree.h"

ZSwcSignalFitter::ZSwcSignalFitter()
{
  init();
}

void ZSwcSignalFitter::init()
{
  m_background = NeuTube::IMAGE_BACKGROUND_DARK;
  m_fixingTerminal = false;
}

bool ZSwcSignalFitter::fitSignal(
    Swc_Tree_Node *tn, const ZStack *stack, int channel)
{
  bool succ = false;
  if (tn != NULL && stack != NULL) {
    if (SwcTreeNode::isRegular(tn) && stack->channelNumber() > channel) {
      ZPoint oldCenter = SwcTreeNode::center(tn);

      const Stack *stackData = stack->c_stack(channel);
      SwcTreeNode::translate(tn, -stack->getOffset());
      if (!SwcTreeNode::fitSignal(tn, stackData, getBackground())) {
        succ = SwcTreeNode::fitSignal(
              tn, stackData, getBackground(), 2);
      } else {
        succ = true;
      }
      SwcTreeNode::translate(tn, stack->getOffset());

      if (m_fixingTerminal) {
        if (SwcTreeNode::isTerminal(tn)) {
          SwcTreeNode::setPos(tn, oldCenter);
        }
      }
    }
  }

  return succ;
}

void ZSwcSignalFitter::fitSignal(
    ZSwcTree *tree, const ZStack *stack, int channel)
{
  ZSwcTree::DepthFirstIterator depthIter(tree);
  while (depthIter.hasNext()) {
    Swc_Tree_Node *tn = depthIter.next();
    fitSignal(tn, stack, channel);
  }
}

void ZSwcSignalFitter::fitSignal(
    Swc_Tree *tree, const ZStack *stack, int channel)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, false);
  Swc_Tree_Node *tmptn = NULL;
  while ((tmptn = Swc_Tree_Next(tree)) != NULL) {
    if (!SwcTreeNode::isRoot(tmptn)) {
      fitSignal(tmptn, stack, channel);
    }
  }
}

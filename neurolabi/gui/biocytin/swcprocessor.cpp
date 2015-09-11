#include "swcprocessor.h"
#include "zswctree.h"
#include "swctreenode.h"
#include "tz_math.h"

Biocytin::SwcProcessor::SwcProcessor()
{
  init();
}

void Biocytin::SwcProcessor::init()
{
  m_minDeltaZ = 0.0;
  m_maxVLRatio = 3.0;
}

void Biocytin::SwcProcessor::setResolution(const ZResolution &resolution)
{
  m_resolution = resolution;
}

/*
void Biocytin::SwcProcessor::removeZJump(ZSwcTree *tree)
{
  tree->forceVirtualRoot();
  tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  Swc_Tree_Node *tn = tree->begin();
  while (tn != NULL) {
    Swc_Tree_Node *next = tree->next();
    if (!SwcTreeNode::isRoot(tn)) {
      Swc_Tree_Node *parent = SwcTreeNode::parent(tn);
      double deltaZ =
          fabs(SwcTreeNode::z(tn) - SwcTreeNode::z(parent)) *
          m_resolution.voxelSizeZ();
      double dx = SwcTreeNode::x(tn) - SwcTreeNode::x(parent);
      double dy = SwcTreeNode::y(tn) - SwcTreeNode::y(parent);
      double deltaXY = sqrt(dx * dx + dy * dy);

      if ((deltaZ > m_minDeltaZ) && (deltaZ / deltaXY > m_maxVLRatio)) {
        SwcTreeNode::detachParent(tn);
        SwcTreeNode::adoptChildren(tree->root(), tn);
        SwcTreeNode::kill(tn);
      }
    }
    tn = next;
  }
}
*/

void Biocytin::SwcProcessor::breakZJump(ZSwcTree *tree)
{
  tree->forceVirtualRoot();
  tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  Swc_Tree_Node *tn = tree->begin();
  while (tn != NULL) {
    Swc_Tree_Node *next = tree->next();
    if (!SwcTreeNode::isRoot(tn)) {
      Swc_Tree_Node *parent = SwcTreeNode::parent(tn);
      double deltaZ =
          fabs(SwcTreeNode::z(tn) - SwcTreeNode::z(parent)) *
          m_resolution.voxelSizeZ();
      double dx = (SwcTreeNode::x(tn) - SwcTreeNode::x(parent)) *
          m_resolution.voxelSizeX();
      double dy = SwcTreeNode::y(tn) - SwcTreeNode::y(parent) *
          m_resolution.voxelSizeY();
      double deltaXY = sqrt(dx * dx + dy * dy);

      if ((deltaZ > m_minDeltaZ) && (deltaZ / deltaXY > m_maxVLRatio)) {
        SwcTreeNode::setParent(tn, tree->root());
      }
    }
    tn = next;
  }
}

void Biocytin::SwcProcessor::AssignZ(ZSwcTree *tree, const Stack &depthImage)
{
  tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  for (Swc_Tree_Node *tn = tree->begin(); tn != NULL; tn = tree->next()) {
    if (SwcTreeNode::isRegular(tn)) {
      int x = iround(SwcTreeNode::x(tn));
      int y = iround(SwcTreeNode::y(tn));

      CLIP_VALUE(x, 0, C_Stack::width(&depthImage) - 1);
      CLIP_VALUE(y, 0, C_Stack::height(&depthImage) - 1);

      SwcTreeNode::setZ(tn, C_Stack::value(&depthImage, x, y, 0, 0));
    }
  }
}

/*
void Biocytin::SwcProcessor::RemoveZJump(ZSwcTree *tree, double minDeltaZ)
{
  tree->forceVirtualRoot();
  tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  Swc_Tree_Node *tn = tree->begin();
  while (tn != NULL) {
    Swc_Tree_Node *next = tree->next();
    if (!SwcTreeNode::isRoot(tn)) {
      double deltaZ = SwcTreeNode::z(tn) - SwcTreeNode::z(SwcTreeNode::parent(tn));
      if (deltaZ > minDeltaZ) {
        SwcTreeNode::detachParent(tn);
        SwcTreeNode::adoptChildren(tree->root(), tn);
        SwcTreeNode::kill(tn);
      }
    }
    tn = next;
  }
}
*/

void Biocytin::SwcProcessor::BreakZJump(ZSwcTree *tree, double minDeltaZ)
{
  tree->forceVirtualRoot();
  tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  Swc_Tree_Node *tn = tree->begin();
  while (tn != NULL) {
    Swc_Tree_Node *next = tree->next();
    if (!SwcTreeNode::isRoot(tn)) {
      double deltaZ =
          fabs(SwcTreeNode::z(tn) - SwcTreeNode::z(SwcTreeNode::parent(tn)));
      double dxy = SwcTreeNode::distance(
            tn, SwcTreeNode::parent(tn), SwcTreeNode::PLANE_EUCLIDEAN);
      if (deltaZ / (dxy + 1.0) > minDeltaZ) {
        SwcTreeNode::setParent(tn, tree->root());
      }
    }
    tn = next;
  }
}

void Biocytin::SwcProcessor::RemoveOrphan(ZSwcTree *tree)
{
  if (SwcTreeNode::isVirtual(tree->root())) {
    Swc_Tree_Node *root = SwcTreeNode::firstChild(tree->root());
    while (root != NULL) {
      Swc_Tree_Node *next = SwcTreeNode::nextSibling(root);
      if (!SwcTreeNode::hasChild(root)) {
        SwcTreeNode::detachParent(root);
        SwcTreeNode::kill(root);
      }
      root = next;
    }
  }
}

void Biocytin::SwcProcessor::SmoothZ(ZSwcTree *tree)
{
  const std::vector<Swc_Tree_Node*> &leafArray =
      tree->getSwcTreeNodeArray(ZSwcTree::LEAF_ITERATOR);
  tree->setLabel(0);
  for (std::vector<Swc_Tree_Node*>::const_iterator iter =
       leafArray.begin(); iter != leafArray.end(); ++iter) {
    Swc_Tree_Node *endTn = *iter;
    while (endTn != NULL) {
      if (SwcTreeNode::isRegularRoot(endTn) ||
          SwcTreeNode::label(endTn) == 1) {
        break;
      }
      endTn = endTn->parent;
    }
    ZSwcPath path(*iter, endTn);
    path.smoothZ();
    path.label(1);
  }
}

void Biocytin::SwcProcessor::SmoothRadius(ZSwcTree *tree)
{
  const std::vector<Swc_Tree_Node*> &leafArray =
      tree->getSwcTreeNodeArray(ZSwcTree::LEAF_ITERATOR);
  tree->setLabel(0);
  for (std::vector<Swc_Tree_Node*>::const_iterator iter =
       leafArray.begin(); iter != leafArray.end(); ++iter) {
    Swc_Tree_Node *endTn = *iter;
    while (endTn != NULL) {
      if (SwcTreeNode::isRegularRoot(endTn) ||
          SwcTreeNode::label(endTn) == 1) {
        break;
      }
      endTn = endTn->parent;
    }
    ZSwcPath path(*iter, endTn);
    //ad-hoc condition to avoid smoothing soma or large branches;
    //needs improvement
    if (path.size() > 5 && path.getAverageRadius() < 15.0) {
      path.smoothRadius(true);
    }
    path.label(1);
  }
}

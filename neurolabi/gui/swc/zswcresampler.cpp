#include "swc/zswcresampler.h"
#include "zswctree.h"
#include "swctreenode.h"
#include "tz_error.h"
#include "zswcbranch.h"


ZSwcResampler::ZSwcResampler()
{
  init();
}

void ZSwcResampler::init()
{
  m_radiusScale = 1.2;
  m_distanceScale = 2.0;
  m_ignoringInterRedundant = false;
}

int ZSwcResampler::suboptimalDownsample(ZSwcTree *tree)
{
  int count = 0;
  int treeCount = tree->updateIterator();
  int checked = 0;

  Swc_Tree_Node *tn = tree->begin();
  while (tn != NULL) {
    Swc_Tree_Node *next = tn->next;

    //std::cout << "before: " << next <<std::endl;

    ++checked;
    Swc_Tree_Node *parent = SwcTreeNode::parent(tn);
//    Swc_Tree_Node *child = SwcTreeNode::firstChild(tn);

    if (SwcTreeNode::isRegular(tn) && SwcTreeNode::isRegular(parent)) {
      bool redundant = false;
      SwcTreeNode::EMergeOption option = SwcTreeNode::MERGE_W_PARENT;

      if (SwcTreeNode::isWithin(tn, parent)) {
        redundant = true;
      } else if (SwcTreeNode::isWithin(parent, tn)) {
        redundant = true;
        option = SwcTreeNode::MERGE_W_CHILD;
      } else {
        if (SwcTreeNode::isContinuation(tn) || SwcTreeNode::isContinuation((parent))) {
          if (SwcTreeNode::hasSignificantOverlap(tn, parent)) {
            redundant = true;
            if (SwcTreeNode::isLeaf(tn)) {
              option = SwcTreeNode::MERGE_W_CHILD;
            } else {
              option = SwcTreeNode::MERGE_WEIGHTED_AVERAGE;
            }
          }
        }
      }

      if (!redundant) {
        if (!m_ignoringInterRedundant) {
          if (SwcTreeNode::isContinuation(tn)) {
            redundant = isInterRedundant(tn, parent);

          }
        } else {
          if (SwcTreeNode::isContinuation(tn) && SwcTreeNode::hasOverlap(tn, parent)) {
            redundant = isInterRedundant(tn, parent);
            if (SwcTreeNode::isBranchPoint(parent) || SwcTreeNode::isRoot(parent)) {
              option = SwcTreeNode::MERGE_W_PARENT;
            } else {
              option = SwcTreeNode::MERGE_WEIGHTED_AVERAGE;
            }
          }
        }
      }

      if (redundant) {
        if (next != NULL) {
          next = next->next;
        }
        SwcTreeNode::mergeToParent(tn, option);
        ++count;
      }
    }

    tn = next;
  }

#ifdef _DEBUG_
  std::cout << count << " removed" << std::endl;
  std::cout << checked << " " << treeCount << std::endl;
#endif

  return count;
}

int ZSwcResampler::optimalDownsample(ZSwcTree *tree)
{
  int n = 0;
  int subn = 0;

  tree->updateIterator();
  Swc_Tree_Node *tn = tree->begin();
  while (tn != NULL) {
    SwcTreeNode::setWeight(tn, 1.0);
    tn = tn->next;
  }


  while ((subn = suboptimalDownsample(tree)) > 0) {
    n += subn;
  }
//  n += optimizeCriticalParent(tree);

  tree->deprecate(ZSwcTree::ALL_COMPONENT);
  return n;
}

int ZSwcResampler::optimizeCriticalParent(ZSwcTree *tree)
{
  //Check nodes connected to leaves
  tree->updateIterator();

  Swc_Tree_Node *tn = tree->begin();

  int count = 0;

  while (tn != NULL) {
    Swc_Tree_Node *next = tn->next;

    if ((SwcTreeNode::isBranchPoint(tn) || SwcTreeNode::isLeaf(tn)) &&
        !SwcTreeNode::isRoot(tn)) {
      Swc_Tree_Node *parent = SwcTreeNode::parent(tn);

      bool redundant = false;

      if (SwcTreeNode::isContinuation(parent)) {
        //Set leaf to be the same as its parent if it's covered by the parent
        if (SwcTreeNode::isWithin(tn, parent)) {
          SwcTreeNode::copyProperty(parent, tn);
          redundant = true;
        }

        if (!redundant) {
          if (SwcTreeNode::isWithin(parent, tn)) {
            redundant = true;
          }
        }

        if (!m_ignoringInterRedundant) {
          if (!redundant) {
            redundant = isInterRedundant(parent, tn);
          }
        }
      }

      if (redundant) {
        TZ_ASSERT(!SwcTreeNode::isRoot(parent), "Invalid node");
        SwcTreeNode::mergeToParent(parent);
        ++count;
      }
    }

    tn = next;
  }

  std::cout << count << " critical removed" << std::endl;

  return count;
}

bool ZSwcResampler::isInterRedundant(
    const Swc_Tree_Node *tn, const Swc_Tree_Node *master) const
{
  bool redundant = false;

  Swc_Tree_Node *parent = SwcTreeNode::parent(tn);
  Swc_Tree_Node *child = SwcTreeNode::firstChild(tn);

  if (parent == master || child == master) {
    if (SwcTreeNode::isContinuation(tn) && SwcTreeNode::hasOverlap(tn, master)) {
      double d1 = SwcTreeNode::distance(tn, parent);
      double d2 = SwcTreeNode::distance(tn, child);
      double lambda = d2 / (d1 + d2);
      Swc_Tree_Node tmpNode;
      SwcTreeNode::setDefault(&tmpNode);
      SwcTreeNode::interpolate(parent, child, lambda, &tmpNode);

      TZ_ASSERT(SwcTreeNode::isRegular(&tmpNode), "Unexpected virtual node");

//      double sizeScale = 1.2;
      //More likely to be redundant with smaller distance scale
      //More likely to be redundant with bigger radius scale
      if (SwcTreeNode::distance(tn, &tmpNode) * m_distanceScale <
          SwcTreeNode::radius(&tmpNode)) { //not too far away
        if ((SwcTreeNode::radius(tn) * m_radiusScale > SwcTreeNode::radius(&tmpNode)) &&
            (SwcTreeNode::radius(tn) < SwcTreeNode::radius(&tmpNode) * m_radiusScale)) {
          redundant = true;
        }
      }
    }
  }

  return redundant;
}
#if 0
void ZSwcResampler::upsampleAsFixedNodeNumber(ZSwcTree *tree, int n)
{
  int nodeNumberAdded = n - tree->size();
  if (nodeNumberAdded <= 0) {
    return;
  }

  double length = tree->length();
  double resampleStep = length / (nodeNumberAdded + 1);

  tree->updateIterator();
  Swc_Tree_Node *tn = tree->begin();

  int count = 0;
  while (tn != NULL) {
    Swc_Tree_Node *next = tn->next;
  }

  int n = Swc_Tree_Label_Branch_All(data());

  for (int i = 1; i <= n; i++) {
    ZSwcBranch *branch = extractBranch(i);
    branch->resample(step);
  }
}
#endif

void ZSwcResampler::radiusResample(ZSwcTree *tree)
{
  int n = Swc_Tree_Label_Branch_All(tree->data());

  for (int i = 1; i <= n; i++) {
    ZSwcBranch *branch = tree->extractBranch(i);
    branch->radiusResample();
  }
}

void ZSwcResampler::denseInterpolate(ZSwcTree *tree)
{
  int n = Swc_Tree_Label_Branch_All(tree->data());

  for (int i = 1; i <= n; i++) {
    ZSwcBranch *branch = tree->extractBranch(i);
    branch->denseInterpolate();
  }
}

#include "zswcpruner.h"
#include "neutubeconfig.h"
#include "zswctree.h"
#include "swctreenode.h"
#include "zswcbranch.h"
#include "zswcforest.h"
#include "zswcdisttrunkanalyzer.h"
#include "zdoublevector.h"

ZSwcPruner::ZSwcPruner() : m_minLength(100.0), m_removingOrphan(false)
{
  m_minOrphanCount = 10;
}

Swc_Tree_Node* ZSwcPruner::getParentBranchPoint(
    const std::vector<int> &neighborCount, Swc_Tree_Node *tn)
{
  Swc_Tree_Node *parent = SwcTreeNode::parent(tn);
  int id = SwcTreeNode::id(parent);
  while (neighborCount[id] <= 2) {
    parent = SwcTreeNode::parent(tn);
    if (!SwcTreeNode::isRegular(parent)) {
      parent = NULL;
      break;
    }
    id = SwcTreeNode::id(parent);
    tn = parent;
  }

  return parent;
}

Swc_Tree_Node* ZSwcPruner::getMinWeightedNode(
    const std::vector<Swc_Tree_Node*> &nodeArray,
    const std::vector<bool> &removed)
{
  double minWeight = Infinity;
  Swc_Tree_Node *target = NULL;
  for (std::vector<Swc_Tree_Node*>::const_iterator iter = nodeArray.begin();
       iter != nodeArray.end(); ++iter) {
    Swc_Tree_Node *tn = *iter;
    if (removed[SwcTreeNode::id(tn)] == false) {
      if (minWeight > SwcTreeNode::weight(tn)) {
        minWeight = SwcTreeNode::weight(tn);
        target = tn;
      }
    }
  }

  return target;
}

int ZSwcPruner::prune(ZSwcTree *tree) const
{
  if (tree == NULL) {
    return 0;
  }

  if (tree->isEmpty()) {
    return 0;
  }

  if (tree->isForest()) {
    ZSwcForest * forest = tree->toSwcTreeArray();

#ifdef _DEBUG_
    forest->print();
#endif

    int count = 0;
    if (forest != NULL) {
      for (ZSwcForest::iterator iter = forest->begin(); iter != forest->end();
           ++iter) {
        count += prune(*iter);
      }

      ZSwcTree *result = forest->toSwcTree();
      tree->setData(result->data());
      result->setData(NULL, ZSwcTree::LEAVE_ALONE);
      delete result;
    }

    return count;
  }

#ifdef _DEBUG_
  tree->print();
#endif

  const std::vector<Swc_Tree_Node*>& branchPointArray =
      tree->getSwcTreeNodeArray(ZSwcTree::BRANCH_POINT_ITERATOR);
  if (branchPointArray.size() == 0) {
    return 0;
  }

  SwcTreeNode::setAsRoot(branchPointArray[0]);
  tree->setDataFromNode(branchPointArray[0], ZSwcTree::LEAVE_ALONE);

#ifdef _DEBUG_
  tree->print();
#endif

  int maxId = tree->resortId();

#ifdef _DEBUG_2
  tree->save(GET_DATA_DIR + "/test.swc");
#endif

  //Reduce the tree
  ZSwcTree *reduced = tree->clone();
  Swc_Tree_Reduce(reduced->data());

  std::vector<int> neighborCount(maxId + 1, 0);
  std::vector<bool> removed(maxId + 1, false);
  const std::vector<Swc_Tree_Node*>& reducedNodeArray =
      reduced->getSwcTreeNodeArray(ZSwcTree::DEPTH_FIRST_ITERATOR);

  for (std::vector<Swc_Tree_Node*>::const_iterator iter = reducedNodeArray.begin();
       iter != reducedNodeArray.end(); ++iter) {
    Swc_Tree_Node *tn = *iter;
    neighborCount[SwcTreeNode::id(tn)] = SwcTreeNode::regularNeighborNumber(tn);
  }

  std::vector<Swc_Tree_Node*> terminalArray =
      reduced->getSwcTreeNodeArray(ZSwcTree::TERMINAL_ITERATOR);

  //Get the longest path of the reduced tree
  ZSwcPath path = reduced->getLongestPath();

  //Set the root as a branch point of the path
  for (ZSwcPath::iterator iter = path.begin(); iter != path.end(); ++iter) {
    Swc_Tree_Node *tn = *iter;
    if (SwcTreeNode::isBranchPoint(tn)) {
      SwcTreeNode::setAsRoot(tn);
    }
  }

  int removeCount = 0;
  int terminalCount = (int) terminalArray.size();
  Swc_Tree_Node *terminal = getMinWeightedNode(terminalArray, removed);
  double minLength = SwcTreeNode::weight(terminal);  

  std::vector<Swc_Tree_Node*> originalNodeList(maxId + 1, NULL);
  tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  for (Swc_Tree_Node *tn = tree->begin(); tn != NULL; tn = tree->next()) {
    originalNodeList[SwcTreeNode::id(tn)] = tn;
  }
  std::vector<Swc_Tree_Node*> removeList;

  //While the minimal is smaller than the threshold
  while (minLength < m_minLength) {
#ifdef _DEBUG_2
    std::cout << SwcTreeNode::id(terminal) << " Weight: "
              << minLength << std::endl;
#endif

    //Mark the terminal as removed
    removed[SwcTreeNode::id(terminal)] = true;
    //std::cout << SwcTreeNode::id(terminal) << std::endl;
    removeList.push_back(originalNodeList[SwcTreeNode::id(terminal)]);
    ++removeCount;
    --terminalCount;
    reduceTerminalBranch(terminal);

    if (terminalCount < 3) {
      break;
    }
#if 0
    Swc_Tree_Node *ancestor = getParentBranchPoint(neighborCount, terminal);
    if (ancestor == NULL) {
      break;
    }
    neighborCount[SwcTreeNode::id(ancestor)]--;
    //If the parent of the terminal is continuous after removal
    if (neighborCount[SwcTreeNode::id(ancestor)] <= 2) {
      //Update the weight of termini located at the downstream of the parent of
      //the removed terminal
      /*
      Swc_Tree_Node *nextAncestor =
          getParentBranchPoint(neighborCount, ancestor);
      SwcTreeNode::addWeight(nextAncestor, SwcTreeNode::weight(ancestor));
      */
      for (std::vector<Swc_Tree_Node*>::iterator iter = terminalArray.begin();
           iter != terminalArray.end(); ++iter) {
        if (SwcTreeNode::isAncestor(ancestor, *iter)) {
          SwcTreeNode::addWeight(*iter, SwcTreeNode::weight(ancestor));
        }
      }
    }
#endif

    //Update the minimal
    terminal = getMinWeightedNode(terminalArray, removed);
    minLength = SwcTreeNode::weight(terminal);
  }

  for (std::vector<Swc_Tree_Node*>::iterator iter = terminalArray.begin();
       iter != terminalArray.end(); ++iter) {
    if (removed[SwcTreeNode::id(*iter)]) {
      SwcTreeNode::kill(*iter);
    }
  }
  delete reduced;

  for (std::vector<Swc_Tree_Node*>::const_iterator iter = removeList.begin();
       iter != removeList.end(); ++iter) {
    removeTerminalBranch(*iter);
  }

  return removeCount;
}

void ZSwcPruner::removeTerminalBranch(Swc_Tree_Node *tn)
{
  while (!SwcTreeNode::isBranchPoint(SwcTreeNode::parent(tn))) {
    tn = SwcTreeNode::parent(tn);
    if (tn == NULL) {
      break;
    }
  }

  if (tn != NULL) {
    SwcTreeNode::detachParent(tn);
    SwcTreeNode::killSubtree(tn);
  }
}

void ZSwcPruner::reduceTerminalBranch(Swc_Tree_Node *tn)
{
  Swc_Tree_Node *parent = SwcTreeNode::parent(tn);
  SwcTreeNode::detachParent(tn);
#ifdef _DEBUG_2
  std::cout << tn << " detached." << std::endl;
#endif
  if (!SwcTreeNode::isBranchPoint(parent)) {
    SwcTreeNode::addWeight(SwcTreeNode::firstChild(parent),
                           SwcTreeNode::weight(tn));
    SwcTreeNode::mergeToParent(parent);
  }
}

void ZSwcPruner::removeOrphanBlob(ZSwcTree *tree) const
{
  ZSwcForest *forest = tree->toSwcTreeArray();
  ZSwcDistTrunkAnalyzer trunkAnalyzer;
  trunkAnalyzer.setDistanceWeight(0, 1);

  std::cout << forest->size() << " subtrees" << std::endl;

  double minLength = m_minLength;

  if (minLength == 0.0 && (int) forest->size() >= m_minOrphanCount) {
    ZDoubleVector treeLength(forest->size());


    for (size_t i = 0; i < forest->size(); ++i) {
      ZSwcTree *subtree = forest->getSwcTree(i);
      treeLength[i] = subtree->mainTrunk(&trunkAnalyzer).getLength();
    }

    minLength = treeLength.mean();
    std::cout << "Mean length:  " << minLength << std::endl;
  }

  int count = 0;
  for (size_t i = 0; i < forest->size(); ++i) {
    ZSwcTree *subtree = forest->getSwcTree(i);
    if (subtree->mainTrunk(&trunkAnalyzer).getLength() >= minLength) {
      tree->merge(subtree);
    } else {
      delete subtree;
      ++count;
    }
  }

  std::cout << count << " trees removed." << std::endl;

  forest->setDataOwner(false);
  delete forest;
}

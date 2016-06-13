#ifndef ZSWCTREETEST_H
#define ZSWCTREETEST_H

#include "ztestheader.h"

#include <set>


#include "zswctree.h"
#include "swc/zswcmetric.h"
#include "swc/zswcterminalsurfacemetric.h"
#include "swc/zswcterminalanglemetric.h"
#include "swctreenode.h"
#include "swc/zswcsubtreeanalyzer.h"
#include "zswcglobalfeatureanalyzer.h"
#include "zdoublevector.h"
#include "swc/zswcpruner.h"
#include "zswcforest.h"
#include "neutubeconfig.h"

#ifdef _USE_GTEST_

static void testTreeIterator(ZSwcTree &tree,
                             const ZTestSwcTreeIteratorConfig &config,
                             int *truthArray,
                             int truthCount, bool testReverse = false)
{
  int count = -1;

  if (config.start == NULL && config.blocker == NULL) {
    count = tree.updateIterator(config.option, TRUE);
  } else if (config.blocker == NULL) {
    count = tree.updateIterator(config.option, config.start, TRUE);
  } else if (config.start == NULL) {
    count = tree.updateIterator(config.option, *(config.blocker), TRUE);
  } else {
    count = tree.updateIterator(config.option, config.start, *(config.blocker),
                                TRUE);
  }

  ASSERT_EQ(count, truthCount);

  if (truthArray == NULL) {
    ASSERT_EQ(NULL, tree.begin());
  }

  for (Swc_Tree_Node *tn = tree.begin(); tn != tree.end(); tn = tree.next()) {
    if (testReverse) {
      ASSERT_EQ(SwcTreeNode::id(tn), truthArray[count - 1 - SwcTreeNode::index(tn)]);
    } else {
      ASSERT_EQ(SwcTreeNode::id(tn), truthArray[SwcTreeNode::index(tn)]);
    }
  }

//  return true;
}

TEST(TestSwcTree, TestTreeIterator) {
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/breadth_first.swc");
  ZTestSwcTreeIteratorConfig config;
  config.option = SWC_TREE_ITERATOR_DEPTH_FIRST;
  {
    std::cout << "testing SWC_TREE_ITERATOR_DEPTH_FIRST" << std::endl;
    int array[7] = { -1, 1, 2, 4, 3, 6, 5 };
    testTreeIterator(tree, config, array, sizeof(array) / sizeof(array[0]));
  }

  config.option = SWC_TREE_ITERATOR_BREADTH_FIRST;
  {
    std::cout << "testing SWC_TREE_ITERATOR_BREADTH_FIRST" << std::endl;
    int array[7] = { -1, 1, 2, 4, 3, 6, 5 };
    testTreeIterator(tree, config, array, sizeof(array) / sizeof(array[0]));
  }

  {
    config.option = SWC_TREE_ITERATOR_BREADTH_FIRST;
    int array[7] = { -1, 1, 2, 4, 3, 6, 5 };
    testTreeIterator(tree, config, array, sizeof(array) / sizeof(array[0]));
  }

  {
    config.option = SWC_TREE_ITERATOR_REVERSE;
    int array[7] = { 5, 6, 3, 4, 2, 1, -1 };
    testTreeIterator(tree, config, array, sizeof(array) / sizeof(array[0]));
  }

  {
    config.option = SWC_TREE_ITERATOR_LEAF;
    int array[3] = { 4, 6, 5 };
    testTreeIterator(tree, config, array, sizeof(array) / sizeof(array[0]));
  }

  {
    config.option = SWC_TREE_ITERATOR_REVERSE;
    int array[3] = { 4, 6, 5 };
    testTreeIterator(tree, config, array, sizeof(array) / sizeof(array[0]), true);
  }

  //With blockers
  std::set<Swc_Tree_Node*> blocker;
  blocker.insert(tree.data()->root->first_child);

  {
    config.option = SWC_TREE_ITERATOR_BREADTH_FIRST;
    config.blocker = &blocker;
    int array[1] = { -1 };
    testTreeIterator(tree, config, array, sizeof(array) / sizeof(array[0]));
  }

  {
    config.option = SWC_TREE_ITERATOR_DEPTH_FIRST;
    int array[1] = { -1 };
    testTreeIterator(tree, config, array, sizeof(array) / sizeof(array[0]));
  }

  {
    config.option = SWC_TREE_ITERATOR_REVERSE;
    int array[1] = { -1 };
    testTreeIterator(tree, config, array, sizeof(array) / sizeof(array[0]));
  }

  {
    config.option = SWC_TREE_ITERATOR_LEAF;
    testTreeIterator(tree, config, NULL, 0);
  }

  blocker.clear();
  blocker.insert(tree.data()->root->first_child->first_child);
  {
    config.option = SWC_TREE_ITERATOR_BREADTH_FIRST;
    int array[2] = { -1, 1 };
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]));
  }

  {
    config.option = SWC_TREE_ITERATOR_DEPTH_FIRST;
    int array[2] = { -1, 1 };
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]));
  }

  {
    config.option = SWC_TREE_ITERATOR_REVERSE;
    int array[2] = { 1, -1 };
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]));
  }

  blocker.clear();
  blocker.insert(tree.data()->root->first_child->first_child->first_child->next_sibling);
  {
    config.option = SWC_TREE_ITERATOR_BREADTH_FIRST;
    int array[4] = { -1, 1, 2, 4};
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]));
  }

  {
    int array[4] = { -1, 1, 2, 4 };
    config.option = SWC_TREE_ITERATOR_REVERSE;
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]), true);
  }

  {
    int array[4] = { -1, 1, 2, 4 };
    config.option = SWC_TREE_ITERATOR_DEPTH_FIRST;
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]));
  }

  {

    int array[4] = { -1, 1, 2, 4 };
    config.option = SWC_TREE_ITERATOR_REVERSE;
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]), true);
  }

  blocker.clear();
  blocker.insert(tree.data()->root->first_child->first_child->first_child);
  {
    int array[6] = { -1, 1, 2, 3, 6, 5 };
    config.option = SWC_TREE_ITERATOR_BREADTH_FIRST;
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]));
  }

  {
    int array[6] = { -1, 1, 2, 3, 6, 5 };
    config.option = SWC_TREE_ITERATOR_REVERSE;
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]), true);
  }

  {
    int array[6] = { -1, 1, 2, 3, 6, 5 };
    config.option = SWC_TREE_ITERATOR_DEPTH_FIRST;
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]));
  }

  {
    int array[6] = { -1, 1, 2, 3, 6, 5 };
    config.option = SWC_TREE_ITERATOR_REVERSE;
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]), true);
  }

  {
    int array[2] = { 6, 5 };
    config.option = SWC_TREE_ITERATOR_LEAF;
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]));

    config.option = SWC_TREE_ITERATOR_REVERSE;
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]), true);
  }

  blocker.insert(tree.data()->root->first_child->first_child->first_child->next_sibling);
  {
    int array[3] = { -1, 1, 2 };
    config.option = SWC_TREE_ITERATOR_BREADTH_FIRST;
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]));
  }

  {
    int array[3] = { -1, 1, 2 };
    config.option = SWC_TREE_ITERATOR_REVERSE;
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]), true);
  }

  {
    int array[3] = { -1, 1, 2 };
    config.option = SWC_TREE_ITERATOR_DEPTH_FIRST;
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]));
  }

  {
    int array[3] = { -1, 1, 2 };
    config.option = SWC_TREE_ITERATOR_REVERSE;
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]), true);
  }

#if 0
  blocker.clear();
  blocker.insert(tree.data()->root->first_child->first_child->first_child->first_child);
  {
    int array[6] = { -1, 1, 2, 3, 4, 6 };
    config.option = SWC_TREE_ITERATOR_BREADTH_FIRST;
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]));
  }

  {
    int array[6] = { -1, 1, 2, 3, 6, 4 };
    config.option = SWC_TREE_ITERATOR_DEPTH_FIRST;
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]));
  }

  Swc_Tree_Node *start = tree.data()->root->first_child->first_child;
  {
    int array[4] = { 2, 3, 4, 6 };
    config.start = start;
    config.option = SWC_TREE_ITERATOR_BREADTH_FIRST;
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]));
  }

  {
    int array[4] = { 2, 3, 6, 4 };
    config.option = SWC_TREE_ITERATOR_DEPTH_FIRST;
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]));
  }

  ////////////
  config.start = tree.data()->root->first_child->first_child->first_child;
  {
    int array[2] = { 3, 6 };
    config.option = SWC_TREE_ITERATOR_BREADTH_FIRST;
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]));
  }

  {
    int array[2] = { 3, 6 };
    config.option = SWC_TREE_ITERATOR_DEPTH_FIRST;
    testTreeIterator(tree, config, array,
                               sizeof(array) / sizeof(array[0]));
  }
#endif
}

TEST(SwcTree, Metric)
{
  ZSwcTree tree1;
  ZSwcTree tree2;

  std::string filePath1 = GET_TEST_DATA_DIR + "/benchmark/swc/dist/angle/tree1.swc";
  std::string filePath2 = GET_TEST_DATA_DIR + "/benchmark/swc/dist/angle/tree2.swc";

  tree1.load(filePath1);
  tree2.load(filePath2);

  ZSwcTerminalAngleMetric angleMetric;
  double dist = angleMetric.measureDistance(&tree1, &tree2);

  ASSERT_DOUBLE_EQ(0, dist);
}

TEST(SwcTree, Backtrace)
{
  //test computeBackTraceLenth
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/length_test.swc");
  tree.forceVirtualRoot();
  tree.computeBackTraceLength();
  //tree.print();

  const std::vector<Swc_Tree_Node*> &nodeArray =
      tree.getSwcTreeNodeArray(ZSwcTree::DEPTH_FIRST_ITERATOR);
  ASSERT_EQ(7, (int) nodeArray.size());
  ASSERT_EQ(50, SwcTreeNode::weight(nodeArray[0]));
  ASSERT_EQ(50, SwcTreeNode::weight(nodeArray[1]));
  ASSERT_EQ(40, SwcTreeNode::weight(nodeArray[2]));
  ASSERT_EQ(20, SwcTreeNode::weight(nodeArray[4]));
  ASSERT_EQ(0, SwcTreeNode::weight(nodeArray[3]));
  ASSERT_EQ(0, SwcTreeNode::weight(nodeArray[5]));
  ASSERT_EQ(0, SwcTreeNode::weight(nodeArray[6]));
}

TEST(SwcTree, Subtree)
{
  //test labelSubtree
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/length_test.swc");

  Swc_Tree_Node *tn = tree.queryNode(3);
  tree.labelSubtree(tn, 1);


  const std::vector<Swc_Tree_Node*> &nodeArray =
      tree.getSwcTreeNodeArray(ZSwcTree::DEPTH_FIRST_ITERATOR);
  ASSERT_EQ(7, (int) nodeArray.size());
  ASSERT_EQ(0, SwcTreeNode::label(nodeArray[0]));
  ASSERT_EQ(0, SwcTreeNode::label(nodeArray[1]));
  ASSERT_EQ(0, SwcTreeNode::label(nodeArray[2]));
  ASSERT_EQ(0, SwcTreeNode::label(nodeArray[3]));
  ASSERT_EQ(1, SwcTreeNode::label(nodeArray[4]));
  ASSERT_EQ(1, SwcTreeNode::label(nodeArray[5]));
  ASSERT_EQ(1, SwcTreeNode::label(nodeArray[6]));
}

TEST(SwcTree, getNodeArray)
{
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/long_fork.swc");
  ASSERT_EQ(1, (int) tree.getSwcTreeNodeArray(ZSwcTree::BRANCH_POINT_ITERATOR).size());
  ASSERT_EQ(2, (int) tree.getSwcTreeNodeArray(ZSwcTree::LEAF_ITERATOR).size());
  ASSERT_EQ(3, (int) tree.getSwcTreeNodeArray(ZSwcTree::TERMINAL_ITERATOR).size());

  /*
  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/forest2.swc");
  ASSERT_EQ(3, (int) tree.getSwcTreeNodeArray(ZSwcTree::BRANCH_POINT_ITERATOR).size());
  ASSERT_EQ(11, (int) tree.getSwcTreeNodeArray(ZSwcTree::LEAF_ITERATOR).size());
  ASSERT_EQ(15, (int) tree.getSwcTreeNodeArray(ZSwcTree::TERMINAL_ITERATOR).size());
  */
}

TEST(SwcTree, getLongestPath)
{
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/long_fork.swc");

  ZSwcPath path = tree.getLongestPath();
  ASSERT_EQ(24, (int) path.size());
  ASSERT_EQ(1, SwcTreeNode::id(*path.begin()));
  ASSERT_EQ(31, SwcTreeNode::id(path.at(23)));

  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/compare/compare5.swc");
  path = tree.getLongestPath();
  ASSERT_EQ(9, (int) path.size());
  ASSERT_EQ(8, SwcTreeNode::id(*path.begin()));
  ASSERT_EQ(14, SwcTreeNode::id(path.at(8)));
//  path.print();
}

TEST(SwcTree, forest)
{
  ZSwcTree tree;
  ASSERT_EQ(0x0, tree.toSwcTreeArray());

  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/long_fork.swc");
  ZSwcForest *forest = tree.toSwcTreeArray();

  ASSERT_EQ(1, (int) forest->size());
  ASSERT_EQ(true, tree.isEmpty());

  delete forest;
}

TEST(SwcTree, Prune)
{
  ZSwcPruner pruner;
  pruner.setMinLength(18.0);

  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/fork2.swc");

  ASSERT_EQ(1, pruner.prune(&tree));

  pruner.setMinLength(100.0);
  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/compare/compare1.swc");

  ASSERT_EQ(4 , pruner.prune(&tree));

  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/compare/compare1.swc");

  ZSwcTree tree2;
  tree2.load(GET_TEST_DATA_DIR + "/benchmark/swc/fork2.swc");
  tree.merge(&tree2, false);

  tree.print();

  ASSERT_EQ(5, pruner.prune(&tree));

  tree.save(GET_TEST_DATA_DIR + "/test.swc");
}

TEST(SwcTree, boundBox)
{
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/fork2.swc");

  ASSERT_TRUE(tree.isDeprecated(ZSwcTree::BOUND_BOX));
  ASSERT_TRUE(tree.getBoundBox().isValid());
}

TEST(SwcTree, ExtIterator)
{
  {
    ZSwcTree tree;
    tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/depth_first.swc");

    ZSwcTree::DepthFirstIterator iter(&tree);
    iter.excludeVirtual(true);

    ASSERT_TRUE(iter.hasNext());

    ASSERT_EQ(1, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(2, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(6, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(3, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(5, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(4, SwcTreeNode::id(iter.next()));

    ASSERT_FALSE(iter.hasNext());

    iter.restart();
    ASSERT_TRUE(iter.hasNext());

    ASSERT_EQ(1, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(2, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(6, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(3, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(5, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(4, SwcTreeNode::id(iter.next()));

    ASSERT_FALSE(iter.hasNext());

    ASSERT_EQ(1, SwcTreeNode::id(iter.begin()));
    ASSERT_EQ(2, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(6, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(3, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(5, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(4, SwcTreeNode::id(iter.next()));

    ASSERT_FALSE(iter.hasNext());
  }

  {
    ZSwcTree tree;
    tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/fork2.swc");

    ZSwcTree::TerminalIterator iter(&tree);
    ASSERT_TRUE(iter.hasNext());
    ASSERT_EQ(1, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(5, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(3, SwcTreeNode::id(iter.next()));
    ASSERT_FALSE(iter.hasNext());

    iter.restart();
    ASSERT_TRUE(iter.hasNext());
    ASSERT_EQ(1, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(5, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(3, SwcTreeNode::id(iter.next()));
    ASSERT_FALSE(iter.hasNext());

    ASSERT_EQ(1, SwcTreeNode::id(iter.begin()));
    ASSERT_EQ(5, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(3, SwcTreeNode::id(iter.next()));
    ASSERT_FALSE(iter.hasNext());
  }

  {
    ZSwcTree tree;
    tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/fork2.swc");

    ZSwcTree::LeafIterator iter(&tree);
    ASSERT_TRUE(iter.hasNext());
    ASSERT_EQ(5, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(3, SwcTreeNode::id(iter.next()));
    ASSERT_FALSE(iter.hasNext());

    iter.restart();
    ASSERT_TRUE(iter.hasNext());
    ASSERT_EQ(5, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(3, SwcTreeNode::id(iter.next()));
    ASSERT_FALSE(iter.hasNext());

    ASSERT_EQ(5, SwcTreeNode::id(iter.begin()));
    ASSERT_EQ(3, SwcTreeNode::id(iter.next()));
    ASSERT_FALSE(iter.hasNext());
  }

  {
    ZSwcTree tree;
    tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/multi_tree2.swc");

    ZSwcTree::RegularRootIterator iter(&tree);
    ASSERT_TRUE(iter.hasNext());
    ASSERT_EQ(8, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(6, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(4, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(1, SwcTreeNode::id(iter.next()));
    ASSERT_FALSE(iter.hasNext());

    iter.restart();
    ASSERT_TRUE(iter.hasNext());
    ASSERT_EQ(8, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(6, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(4, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(1, SwcTreeNode::id(iter.next()));
    ASSERT_FALSE(iter.hasNext());

    ASSERT_EQ(8, SwcTreeNode::id(iter.begin()));
    ASSERT_EQ(6, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(4, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(1, SwcTreeNode::id(iter.next()));
    ASSERT_FALSE(iter.hasNext());
  }

  {
    ZSwcTree tree;
    tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/forest1.swc");

    ZSwcTree::DownstreamIterator iter(tree.firstRegularRoot());
    ASSERT_TRUE(iter.hasNext());
    ASSERT_EQ(17, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(18, SwcTreeNode::id(iter.next()));
    ASSERT_FALSE(iter.hasNext());

    iter.restart();
    ASSERT_TRUE(iter.hasNext());
    ASSERT_EQ(17, SwcTreeNode::id(iter.next()));
    ASSERT_EQ(18, SwcTreeNode::id(iter.next()));
    ASSERT_FALSE(iter.hasNext());

    ASSERT_EQ(17, SwcTreeNode::id(iter.begin()));
    ASSERT_EQ(18, SwcTreeNode::id(iter.next()));
    ASSERT_FALSE(iter.hasNext());


    Swc_Tree_Node *tn = tree.queryNode(10);
    ZSwcTree::DownstreamIterator iter2(tn);
    ASSERT_TRUE(iter2.hasNext());
    ASSERT_EQ(10, SwcTreeNode::id(iter2.next()));
    ASSERT_EQ(11, SwcTreeNode::id(iter2.next()));

    iter2.restart();
    ASSERT_TRUE(iter2.hasNext());
    ASSERT_EQ(10, SwcTreeNode::id(iter2.next()));
    ASSERT_EQ(11, SwcTreeNode::id(iter2.next()));

    ASSERT_EQ(10, SwcTreeNode::id(iter2.begin()));
    ASSERT_EQ(11, SwcTreeNode::id(iter2.next()));
    ASSERT_EQ(12, SwcTreeNode::id(iter2.next()));
    ASSERT_FALSE(iter2.hasNext());
  }
}

#endif

#endif // ZSWCTREETEST_H

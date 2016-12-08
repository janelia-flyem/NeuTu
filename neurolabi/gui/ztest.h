#ifndef ZTEST_H
#define ZTEST_H

#include <set>
#include <iostream>
#include "zswctree.h"

class MainWindow;

struct ZTestSwcTreeIteratorConfig {
  int option;
  Swc_Tree_Node *start;
  std::set<Swc_Tree_Node*> *blocker;

  ZTestSwcTreeIteratorConfig() : option(SWC_TREE_ITERATOR_NO_UPDATE),
    start(NULL), blocker(NULL) {}
};

class ZTest
{
public:
  ZTest();

  static ZTest& getInstance() {
    static ZTest test;

    return test;
  }

  void setCommandLineArg(int argc, char *argv[]);
  void runUnitTest();

public:
  template <typename T>
  static bool testEqual(const T &golden, const T &v);
  static void test(MainWindow *host);
  static void stressTest(MainWindow *host);
  static bool testTreeIterator();
  static int RunUnitTest(int argc, char *argv[]);

public:
  static bool testTreeIterator(ZSwcTree &tree,
                               const ZTestSwcTreeIteratorConfig &config,
                               int *truthArray,
                               int truthCount, bool testReverse = false);

  static std::ostream &m_failureStream;

private:
  int m_argc;
  char **m_argv;
};

template<typename T>
bool ZTest::testEqual(const T &golden, const T &v)
{
  if (golden != v) {
    m_failureStream << "Unexpected value: " << v << "; Expected: "
                    << golden << std::endl;

    return false;
  }

  return true;
}

#endif // ZTEST_H

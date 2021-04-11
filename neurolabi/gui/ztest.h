#ifndef ZTEST_H
#define ZTEST_H

#include <set>
#include <iostream>

#include "zjsonobject.h"

class MainWindow;
class ZSwcTree;
struct ZTestSwcTreeIteratorConfig;
class ZDvidTarget;

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
  static ZTest& GetIntance() {
    static ZTest test;

    return test;
  }

  template <typename T>
  static bool testEqual(const T &golden, const T &v);
  void test(MainWindow *host);
  static void Test(MainWindow *host);
  static void stressTest(MainWindow *host);
  static bool testTreeIterator();
  static int RunUnitTest(int argc, char *argv[]);
  static void CrashTest();
  static void CommandLineTest();

public:
  static bool testTreeIterator(ZSwcTree &tree,
                               const ZTestSwcTreeIteratorConfig &config,
                               int *truthArray,
                               int truthCount, bool testReverse = false);

  static std::ostream &m_failureStream;

private:
  std::string getDataDir() const;
  ZDvidTarget getDvidTarget() const;
  std::string getVar(const std::string &key) const;

private:
  int m_argc;
  char **m_argv = nullptr;
  ZJsonObject m_testEnv;
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

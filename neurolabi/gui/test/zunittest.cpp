#include "zunittest.h"

#include "ztestall.h"

ZUnitTest::ZUnitTest(int argc, char *argv[])
{
  m_argc = argc;
  m_argv = argv;
}

int ZUnitTest::run()
{
#ifdef _USE_GTEST_
  ::testing::InitGoogleTest(&m_argc, m_argv);

  return RUN_ALL_TESTS();
#else
  return 0;
#endif
}

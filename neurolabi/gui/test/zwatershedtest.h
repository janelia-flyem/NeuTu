#ifndef ZWATERSHEDTEST_H
#define ZWATERSHEDTEST_H

#include "ztestheader.h"
#include "flyem/zstackwatershedcontainer.h"

#ifdef _USE_GTEST_

TEST(ZStackWatershedContainer, test)
{
  ASSERT_TRUE(ZStackWatershedContainer::Test());
}

#endif

#endif // ZWATERSHEDTEST_H

#ifndef ZNETWORKUTILSTEST_H
#define ZNETWORKUTILSTEST_H

#include "ztestheader.h"
#include "qt/network/znetworkutils.h"

#ifdef _USE_GTEST_

TEST(ZNetworkUtils, Basic)
{
  ASSERT_FALSE(ZNetworkUtils::HasHead(""));
}

#endif


#endif // ZNETWORKUTILSTEST_H

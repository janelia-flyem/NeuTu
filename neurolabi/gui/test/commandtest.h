#ifndef COMMANDTEST_H
#define COMMANDTEST_H

#include "ztestheader.h"

#include "command/utilities.h"
#include "common/memorystream.h"

#ifdef _USE_GTEST_

TEST(command, utilities)
{
  std::string str("123 456");
  ZMemoryInputStream stream(str.data(), str.length());

  auto bodyArray = neutu::ImportBodies(stream);
  ASSERT_EQ(2, int(bodyArray.size()));
  ASSERT_EQ(123, int(bodyArray[0]));
  ASSERT_EQ(456, int(bodyArray[1]));

}

#endif

#endif // COMMANDTEST_H

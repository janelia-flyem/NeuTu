#ifndef COMMANDTEST_H
#define COMMANDTEST_H

#include "ztestheader.h"

#include "command/utilities.h"
#include "common/memorystream.h"

#ifdef _USE_GTEST_

TEST(command, utilities)
{
  {
    std::string str("123 456");
    ZMemoryInputStream stream(str.data(), str.length());

    auto bodyArray = neutu::ImportBodies(stream);
    ASSERT_EQ(2, int(bodyArray.size()));
    ASSERT_EQ(123, int(bodyArray[0]));
    ASSERT_EQ(456, int(bodyArray[1]));
  }

  {
    std::string str("123,456");
    ZMemoryInputStream stream(str.data(), str.length());

    auto bodyArray = neutu::ImportBodiesFromCsv(stream, 0, false);
    ASSERT_EQ(1, int(bodyArray.size()));
    ASSERT_EQ(123, int(bodyArray[0]));
  }

  {
    std::string str("body,status\n123,456\n12,3\n");
    ZMemoryInputStream stream(str.data(), str.length());

    auto bodyArray = neutu::ImportBodiesFromCsv(stream, 0, true);
    ASSERT_EQ(2, int(bodyArray.size()));
    ASSERT_EQ(123, int(bodyArray[0]));
    ASSERT_EQ(12, int(bodyArray[1]));
  }

  {
    std::string str("1,2\n");
    ZMemoryInputStream stream(str.data(), str.length());

    auto bodyArray = neutu::ImportBodiesFromCsv(stream, 0, true);
    ASSERT_TRUE(bodyArray.empty());
  }
}

#endif

#endif // COMMANDTEST_H

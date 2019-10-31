#ifndef ZSTRINGBUILDTEST_H
#define ZSTRINGBUILDTEST_H

#include "ztestheader.h"
#include "common/zstringbuilder.h"

#ifdef _USE_GTEST_

TEST(ZStringBuilder, Basic)
{
  {
    ZStringBuilder builder("");

    ASSERT_TRUE(std::string(builder).empty());
  }

  {
    std::string str = ZStringBuilder("test").append(":a").append(1);
    ASSERT_EQ("test:a1", str);
  }

  {
    std::string str = ZStringBuilder("test").append(":a").append(1, 3);
    ASSERT_EQ("test:a001", str);
  }

  {
    std::string str =
        ZStringBuilder("test").append(":a").append(int64_t(12345678910ll), 3);
    ASSERT_EQ("test:a12345678910", str);
  }

  {
    std::string str =
        ZStringBuilder("test").append(":a").append(uint64_t(12345678910ull), 13);
    ASSERT_EQ("test:a0012345678910", str);
  }

  {
    std::string str =
        ZStringBuilder("test").append(":a").append(uint64_t(12345678910ull), 13).
        append(":more:").append(uint64_t(12345678910ull));
    ASSERT_EQ("test:a0012345678910:more:12345678910", str);
  }
}

#endif



#endif // ZSTRINGBUILDTEST_H

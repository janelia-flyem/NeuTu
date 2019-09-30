#ifndef COMMONTEST_H
#define COMMONTEST_H

#include "ztestheader.h"

#include "common/utilities.h"

#ifdef _USE_GTEST_

TEST(common, utilities)
{
  ASSERT_TRUE(neutu::UsingLocalHost("127.0.0.1"));
  ASSERT_TRUE(neutu::UsingLocalHost("localhost"));

  ASSERT_TRUE(neutu::UsingLocalHost("127.0.0.1:8000"));
  ASSERT_TRUE(neutu::UsingLocalHost("localhost:8080"));

  ASSERT_TRUE(neutu::UsingLocalHost("127.0.0.1:8000/test"));
  ASSERT_TRUE(neutu::UsingLocalHost("localhost:8080/test"));

  ASSERT_TRUE(neutu::UsingLocalHost("http://127.0.0.1"));
  ASSERT_TRUE(neutu::UsingLocalHost("http://localhost"));

  ASSERT_TRUE(neutu::UsingLocalHost("neutuse:http://127.0.0.1"));
  ASSERT_TRUE(neutu::UsingLocalHost("neutuse:http://localhost"));

  ASSERT_FALSE(neutu::UsingLocalHost("http://127.0.0.11"));
  ASSERT_FALSE(neutu::UsingLocalHost("http://localhost2"));

  ASSERT_FALSE(neutu::UsingLocalHost("http://localhost2/127.0.0.11"));
  ASSERT_FALSE(neutu::UsingLocalHost("http://127.0.0.2/localhost"));

  ASSERT_EQ(0, neutu::UnsignedCrop(-1));
  ASSERT_EQ(1, neutu::UnsignedCrop(1));

  ASSERT_EQ(0, neutu::UnsignedCrop(int64_t(-1)));
  ASSERT_EQ(214748364789llu, neutu::UnsignedCrop(214748364789ll));
}

#endif

#endif // COMMONTEST_H

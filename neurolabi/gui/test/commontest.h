#ifndef COMMONTEST_H
#define COMMONTEST_H

#include "ztestheader.h"

#include "common/utilities.h"
#include "common/math.h"
#include "common/debug.h"

#include "filesystem/utilities.h"

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

  ASSERT_EQ("http://127.0.0.2/localhost",
            neutu::WithoutQueryString("http://127.0.0.2/localhost"));
  ASSERT_EQ("", neutu::WithoutQueryString(""));
  ASSERT_EQ("", neutu::WithoutQueryString("?"));
  ASSERT_EQ("http://127.0.0.2/localhost",
            neutu::WithoutQueryString("http://127.0.0.2/localhost?test"));
  ASSERT_EQ("http://127.0.0.2/localhost",
            neutu::WithoutQueryString("http://127.0.0.2/localhost??test"));
  ASSERT_EQ("",
            neutu::WithoutQueryString("?http://127.0.0.2/localhost?"));


  ASSERT_TRUE(neutu::IsIntegerValue(0.0));
  ASSERT_TRUE(neutu::IsIntegerValue(10.0));
  ASSERT_TRUE(neutu::IsIntegerValue(100.0f));
  ASSERT_FALSE(neutu::IsIntegerValue(0.9));
  ASSERT_FALSE(neutu::IsIntegerValue(10.000001));
  ASSERT_FALSE(neutu::IsIntegerValue(100.1f));

  ASSERT_TRUE(neutu::WithinOpenRange(2, 1, 3));
  ASSERT_FALSE(neutu::WithinOpenRange(1, 1, 3));
  ASSERT_TRUE(neutu::WithinOpenRange(2.0, 1.9, 3.0));

  std::vector<std::pair<int, int>> partition;
  neutu::RangePartitionProcess(1, 10, 10, [&](int x0, int x1) {
    partition.push_back(std::pair<int, int>(x0, x1));
  });


  ASSERT_EQ(size_t(10), partition.size());
  for (size_t i = 1; i <= 10; ++i) {
    ASSERT_EQ(int(i), partition[i-1].first);
    ASSERT_EQ(int(i), partition[i-1].second);
  }

  partition.clear();
  neutu::RangePartitionProcess(1, 10, 11, [&](int x0, int x1) {
    partition.push_back(std::pair<int, int>(x0, x1));
  });

  ASSERT_EQ(size_t(10), partition.size());
  for (size_t i = 1; i <= 10; ++i) {
    ASSERT_EQ(int(i), partition[i-1].first);
    ASSERT_EQ(int(i), partition[i-1].second);
  }

  partition.clear();
  neutu::RangePartitionProcess(1, 10, 1, [&](int x0, int x1) {
    partition.push_back(std::pair<int, int>(x0, x1));
  });

  ASSERT_EQ(size_t(1), partition.size());
  ASSERT_EQ(1, partition[0].first);
  ASSERT_EQ(10, partition[0].second);


  partition.clear();
  neutu::RangePartitionProcess(1, 10, 20, [&](int x0, int x1) {
    partition.push_back(std::pair<int, int>(x0, x1));
  });

  ASSERT_EQ(size_t(10), partition.size());
  for (size_t i = 1; i <= 10; ++i) {
    ASSERT_EQ(int(i), partition[i-1].first);
    ASSERT_EQ(int(i), partition[i-1].second);
  }

  partition.clear();
  neutu::RangePartitionProcess(1, 10, 2, [&](int x0, int x1) {
    partition.push_back(std::pair<int, int>(x0, x1));
  });

  ASSERT_EQ(size_t(2), partition.size());
  ASSERT_EQ(1, partition[0].first);
  ASSERT_EQ(5, partition[0].second);
  ASSERT_EQ(6, partition[1].first);
  ASSERT_EQ(10, partition[1].second);

  partition.clear();
  neutu::RangePartitionProcess(1, 10, 3, [&](int x0, int x1) {
    partition.push_back(std::pair<int, int>(x0, x1));
  });

  ASSERT_EQ(size_t(3), partition.size());
  ASSERT_EQ(1, partition[0].first);
  ASSERT_EQ(4, partition[0].second);
  ASSERT_EQ(5, partition[1].first);
  ASSERT_EQ(7, partition[1].second);
  ASSERT_EQ(8, partition[2].first);
  ASSERT_EQ(10, partition[2].second);

  partition.clear();
  neutu::RangePartitionProcess(1, 11, 3, [&](int x0, int x1) {
    partition.push_back(std::pair<int, int>(x0, x1));
  });

  ASSERT_EQ(size_t(3), partition.size());
  ASSERT_EQ(1, partition[0].first);
  ASSERT_EQ(4, partition[0].second);
  ASSERT_EQ(5, partition[1].first);
  ASSERT_EQ(8, partition[1].second);
  ASSERT_EQ(9, partition[2].first);
  ASSERT_EQ(11, partition[2].second);

  partition.clear();
  neutu::RangePartitionProcess(2345, 2455667, 300, [&](int x0, int x1) {
    partition.push_back(std::pair<int, int>(x0, x1));
  });

  ASSERT_EQ(size_t(300), partition.size());
  ASSERT_EQ(2345, partition[0].first);
  ASSERT_EQ(2455667, partition[partition.size() - 1].second);
  for (size_t i = 1; i < partition.size(); ++i) {
    ASSERT_EQ(partition[i-1].second + 1, partition[i].first);
  }

  ASSERT_EQ(123456ll, neutu::ToInt64("123456"));
  ASSERT_EQ(-123456ll, neutu::ToInt64("-123456"));
  ASSERT_EQ(123456ull, neutu::ToUint64("123456"));
  ASSERT_LE(0ull, neutu::ToUint64("-123456"));
  ASSERT_EQ(18446744073709550592ull, neutu::ToUint64("18446744073709550592"));
  ASSERT_EQ(-1844674407370955059, neutu::ToInt64("-1844674407370955059"));

  {
    int state = 5;
    {
      neutu::ApplyOnce ao([&]() { state = 3; }, [&]() { state = 5;});
      ASSERT_EQ(3, state);
    }
    ASSERT_EQ(5, state);
  }

  {
    int state = 5;
    {
      APPLY_ONCE(state = 3, state = 5);
      ASSERT_EQ(3, state);
    }
    ASSERT_EQ(5, state);
  }

  {
    std::vector<int> array({1, 2, 3, 4, 5});
    ASSERT_EQ("1, 2, 3, 4, 5", neutu::ToString(array.begin(), array.end(), ", "));
  }
}

TEST(common, math)
{
  ASSERT_EQ(0, neutu::iround(0.1));
  ASSERT_EQ(1, neutu::iround(0.6));

  ASSERT_EQ(1, neutu::iround(1.1));
  ASSERT_EQ(2, neutu::iround(1.6));
}

TEST(filesystem, utilities)
{
  ASSERT_EQ("tif", neutu::FileExtension("test.tif"));
#if defined(_UNIX_)
  ASSERT_EQ("/test1/test2.tif", neutu::JoinPath("/test1", "test2.tif"));
  ASSERT_EQ("/test1/test2/test3.tif", neutu::JoinPath("/test1", "test2", "test3.tif"));
  ASSERT_EQ("/test1/test2/test3.tif", neutu::JoinPath("/test1/", "test2/", "test3.tif"));
  ASSERT_EQ("/test1", neutu::Absolute("/test1", "test2.tif"));
  ASSERT_EQ("/test2/test1", neutu::Absolute("test1", "/test2"));
  ASSERT_EQ("/test1", neutu::Absolute("/test1", "/test2"));
  ASSERT_EQ("/test2/../test1", neutu::Absolute("../test1", "/test2"));
//  std::cout << neutu::Absolute("test1", "test2") << std::endl;
//  std::cout << neutu::Absolute("test1", "") << std::endl;
#endif
}

TEST(common, debug)
{
  HighlightDebug debug;
  debug.setTopicFilter("test1;test2;test 3");
  ASSERT_EQ(OUTPUT_HIGHLIGHT_1, debug.getIcon("test1"));
  ASSERT_EQ(OUTPUT_HIGHLIGHT_2, debug.getIcon("test2"));
  ASSERT_EQ(OUTPUT_HIGHLIGHT_3, debug.getIcon("test 3"));
  ASSERT_TRUE(debug.getIcon("test4").empty());

  debug.setTopicFilter(" test1; test2; test 3 ");
  ASSERT_EQ(OUTPUT_HIGHLIGHT_1, debug.getIcon("test1"));
  ASSERT_EQ(OUTPUT_HIGHLIGHT_2, debug.getIcon("test2"));
  ASSERT_EQ(OUTPUT_HIGHLIGHT_3, debug.getIcon("test 3"));
  ASSERT_TRUE(debug.getIcon("test4").empty());

  debug.setTopicFilter(" ~ test1; test2; test3 ");
  ASSERT_TRUE(debug.getIcon("test1").empty());
  ASSERT_TRUE(debug.getIcon("test2").empty());
  ASSERT_TRUE(debug.getIcon("test3").empty());
  ASSERT_EQ(OUTPUT_HIGHLIGHT_1, debug.getIcon("test 3"));
}

#endif

#endif // COMMONTEST_H

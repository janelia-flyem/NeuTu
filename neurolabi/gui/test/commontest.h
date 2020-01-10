#ifndef COMMONTEST_H
#define COMMONTEST_H

#include "ztestheader.h"

#include "common/utilities.h"
#include "common/math.h"

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
  ASSERT_EQ(214748364789llu, neutu::UnsignedCrop(int64_t(214748364789ll)));

  ASSERT_TRUE(neutu::IsIntegerValue(0.0));
  ASSERT_TRUE(neutu::IsIntegerValue(10.0));
  ASSERT_TRUE(neutu::IsIntegerValue(100.0f));
  ASSERT_FALSE(neutu::IsIntegerValue(0.9));
  ASSERT_FALSE(neutu::IsIntegerValue(10.000001));
  ASSERT_FALSE(neutu::IsIntegerValue(100.1f));

  ASSERT_TRUE(neutu::WithinOpenRange(2, 1, 3));
  ASSERT_FALSE(neutu::WithinOpenRange(1, 1, 3));
  ASSERT_TRUE(neutu::WithinOpenRange(2.0, 1.9, 3.0));

  ASSERT_EQ(2, neutu::ClipValue(2, 1, 3));
  ASSERT_EQ(1.0, neutu::ClipValue(0.0, 1.0, 3.0));
  ASSERT_EQ(3.0, neutu::ClipValue(4.0, 1.0, 3.0));

  int x0 = -1;
  int x1 = 5;
  ASSERT_TRUE(neutu::ClipRange(1, 3, x0, x1));
  ASSERT_EQ(1, x0);
  ASSERT_EQ(3, x1);


  x0 = -1;
  x1 = 2;
  ASSERT_TRUE(neutu::ClipRange(1, 3, x0, x1));
  ASSERT_EQ(1, x0);
  ASSERT_EQ(2, x1);

  x0 = 1;
  x1 = 5;
  ASSERT_TRUE(neutu::ClipRange(1, 3, x0, x1));
  ASSERT_EQ(1, x0);
  ASSERT_EQ(3, x1);

  x0 = 1;
  x1 = 3;
  ASSERT_TRUE(neutu::ClipRange(1, 3, x0, x1));
  ASSERT_EQ(1, x0);
  ASSERT_EQ(3, x1);

  x0 = 1;
  x1 = 2;
  ASSERT_TRUE(neutu::ClipRange(1, 3, x0, x1));
  ASSERT_EQ(1, x0);
  ASSERT_EQ(2, x1);

  x0 = 5;
  x1 = -1;
  ASSERT_FALSE(neutu::ClipRange(1, 3, x0, x1));

  x0 = -5;
  x1 = -1;
  ASSERT_FALSE(neutu::ClipRange(1, 3, x0, x1));

  x0 = 5;
  x1 = 10;
  ASSERT_FALSE(neutu::ClipRange(1, 3, x0, x1));

  std::vector<std::pair<int, int>> partition;
  neutu::RangePartitionProcess(1, 10, 10, [&](int x0, int x1) {
    partition.push_back(std::pair<int, int>(x0, x1));
  });

  ASSERT_EQ(10, partition.size());
  for (size_t i = 1; i <= 10; ++i) {
    ASSERT_EQ(i, partition[i-1].first);
    ASSERT_EQ(i, partition[i-1].second);
  }

  partition.clear();
  neutu::RangePartitionProcess(1, 10, 11, [&](int x0, int x1) {
    partition.push_back(std::pair<int, int>(x0, x1));
  });

  ASSERT_EQ(10, partition.size());
  for (size_t i = 1; i <= 10; ++i) {
    ASSERT_EQ(i, partition[i-1].first);
    ASSERT_EQ(i, partition[i-1].second);
  }

  partition.clear();
  neutu::RangePartitionProcess(1, 10, 1, [&](int x0, int x1) {
    partition.push_back(std::pair<int, int>(x0, x1));
  });

  ASSERT_EQ(1, partition.size());
  ASSERT_EQ(1, partition[0].first);
  ASSERT_EQ(10, partition[0].second);


  partition.clear();
  neutu::RangePartitionProcess(1, 10, 20, [&](int x0, int x1) {
    partition.push_back(std::pair<int, int>(x0, x1));
  });

  ASSERT_EQ(10, partition.size());
  for (size_t i = 1; i <= 10; ++i) {
    ASSERT_EQ(i, partition[i-1].first);
    ASSERT_EQ(i, partition[i-1].second);
  }

  partition.clear();
  neutu::RangePartitionProcess(1, 10, 2, [&](int x0, int x1) {
    partition.push_back(std::pair<int, int>(x0, x1));
  });

  ASSERT_EQ(2, partition.size());
  ASSERT_EQ(1, partition[0].first);
  ASSERT_EQ(5, partition[0].second);
  ASSERT_EQ(6, partition[1].first);
  ASSERT_EQ(10, partition[1].second);

  partition.clear();
  neutu::RangePartitionProcess(1, 10, 3, [&](int x0, int x1) {
    partition.push_back(std::pair<int, int>(x0, x1));
  });

  ASSERT_EQ(3, partition.size());
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

  ASSERT_EQ(3, partition.size());
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

  ASSERT_EQ(300, partition.size());
  ASSERT_EQ(2345, partition[0].first);
  ASSERT_EQ(2455667, partition[partition.size() - 1].second);
  for (size_t i = 1; i < partition.size(); ++i) {
    ASSERT_EQ(partition[i-1].second + 1, partition[i].first);
  }

}

TEST(common, math)
{
  ASSERT_EQ(0, neutu::iround(0.1));
  ASSERT_EQ(1, neutu::iround(0.6));

  ASSERT_EQ(1, neutu::iround(1.1));
  ASSERT_EQ(2, neutu::iround(1.6));
}

#endif

#endif // COMMONTEST_H

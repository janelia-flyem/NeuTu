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
}

#endif

#endif // COMMONTEST_H

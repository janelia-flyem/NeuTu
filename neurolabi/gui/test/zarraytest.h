#ifndef ZARRAYTEST_H
#define ZARRAYTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zarray.h"

#ifdef _USE_GTEST_
TEST(ZArray, basic)
{
  ZArray array;
  array.print();

  ASSERT_TRUE(array.isEmpty());
  ASSERT_TRUE(array.getElementNumber() == 0);

  int dims[3] = {2, 3, 4};
  ZArray array2(mylib::UINT64_TYPE, 3, dims);
//  array2.print();

  ASSERT_EQ(24, int(array2.getElementNumber()));

  array2.setValue<uint64_t>(0, 1);
  ASSERT_EQ(1, int(array2.getUint64Value(0)));

  array2.setValue(1, uint64_t(3));
  ASSERT_EQ(3, int(array2.getUint64Value(1)));

  array2.setValue<uint64_t>(2, 3);
  ASSERT_EQ(3, int(array2.getUint64Value(2)));

  array2.replaceValue<uint64_t>(3, 5);
  ASSERT_EQ(1, int(array2.getUint64Value(0)));
  ASSERT_EQ(5, int(array2.getUint64Value(1)));
  ASSERT_EQ(5, int(array2.getUint64Value(2)));

//  array2.print();

  ASSERT_EQ(5, int(array2.getMax<uint64_t>()));
}

TEST(ZArray, copyData)
{
  int dims[3] = {2, 3, 4};
  ZArray array(mylib::UINT64_TYPE, 3, dims);
  array.setZero();

  ZArray subarray(mylib::UINT64_TYPE, 3, dims);
  subarray.setZero();
  subarray.setValue<uint64_t>(0, 1);

  array.copyDataFrom(subarray.getDataPointer<void>());
  ASSERT_EQ(uint64_t(1), array.getValue<uint64_t>(0));

  subarray.setValue<uint64_t>(2, 2);
  subarray.setValue<uint64_t>(3, 3);
  array.copyDataFrom(subarray.getDataPointer<void>(), 1, 3);
  ASSERT_EQ(uint64_t(2), array.getValue<uint64_t>(3));
  ASSERT_EQ(uint64_t(0), array.getValue<uint64_t>(4));

  array.copyDataFrom(subarray.getDataPointer<void>(), 1, 4);
  ASSERT_EQ(uint64_t(2), array.getValue<uint64_t>(3));
  ASSERT_EQ(uint64_t(3), array.getValue<uint64_t>(4));

}

#endif

#endif // ZARRAYTEST_H

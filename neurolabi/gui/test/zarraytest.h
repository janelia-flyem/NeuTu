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

  array2.setStartCoordinate({10, 20, 30});
  ASSERT_FALSE(array2.withinDataRange({12, 23, 34, 0}));
  ASSERT_FALSE(array2.withinDataRange({12, 23}));
  ASSERT_FALSE(array2.withinDataRange({12, 23, 34}));
  ASSERT_TRUE(array2.withinDataRange({10, 20, 30}));

  int count = 0;
  array2.forEachCoordinates([&](const std::vector<int> &coords) {
//    std::cout << coords[0] << ", " << coords[1] << ", " << coords[2] << std::endl;
    ++count;
    ASSERT_TRUE(array2.withinDataRange(coords));
  });
  ASSERT_EQ(2*3*4, count);

  array2.setValue(0, uint64_t(1));

  for (size_t i = 0; i < array2.getElementNumber(); ++i) {
    array2.setValue(i, uint64_t(i + 1));
  }
  array2.print();

  ASSERT_EQ(size_t(0), array2.getIndex({10, 20, 30}));
  ASSERT_EQ(size_t(1), array2.getIndex({11, 20, 30}));
  ASSERT_EQ(size_t(3), array2.getIndex({11, 21, 30}));
  ASSERT_EQ(size_t(9), array2.getIndex({11, 21, 31}));

  ZArray *array3 = array2.crop({10, 20, 30}, {1, 1, 1});
  ASSERT_EQ(uint64_t(1), array3->getValue<uint64_t>(0));
//  std::cout << array3->getValue<uint64_t>(0) << std::endl;
  delete array3;

  array3 = array2.crop({10, 20, 30}, {2, 2, 1});
//  array3->print();
  ASSERT_EQ(uint64_t(4), array3->getValue<uint64_t>(3));
  delete array3;

  array3 = array2.crop({10, 20, 30}, {2, 5, 1});
  ASSERT_EQ(uint64_t(6), array3->getValue<uint64_t>(5));
  delete array3;

  array3 = array2.crop({11, 20, 30}, {2, 2, 1});
  ASSERT_EQ(uint64_t(2), array3->getValue<uint64_t>(0));
//  array3->print();
  delete array3;

  array3 = array2.crop({11, 21, 30}, {2, 2, 1});
//  ASSERT_EQ(2, array3->getValue<uint64_t>(0));
  array3->print();
  delete array3;

  array3 = array2.crop({11, 21, 31}, {2, 2, 1});
//  ASSERT_EQ(2, array3->getValue<uint64_t>(0));
  array3->print();
  delete array3;
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

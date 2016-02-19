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
  array2.print();
}

#endif

#endif // ZARRAYTEST_H

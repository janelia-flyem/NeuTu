#ifndef ZSTACKOBJECTHANDLETEST_H
#define ZSTACKOBJECTHANDLETEST_H

#ifdef _USE_GTEST_

#include "gtest/gtest.h"

#include "data3d/zstackobjecthandle.h"

TEST(ZStackObjectHandle, Basic)
{
  ZStackObjectHandle::_reset_();

  ZStackObjectHandle handle1;
  ASSERT_EQ(1, int(handle1._getValue_()));

  ZStackObjectHandle handle2;
  ASSERT_EQ(2, int(handle2._getValue_()));

  ZStackObjectHandle handle3 = handle1;

  ASSERT_EQ(handle1, handle3);
  ASSERT_NE(handle1, handle2);

  ZStackObjectHandle handle4;
  ASSERT_EQ(3, int(handle4._getValue_()));

  ZStackObjectHandle handle5{handle4};
  ASSERT_EQ(3, int(handle5._getValue_()));

}

#endif

#endif // ZSTACKOBJECTHANDLETEST_H

#ifndef ZSTACKOBJECTHANDLETEST_H
#define ZSTACKOBJECTHANDLETEST_H

#ifdef _USE_GTEST_

#include "gtest/gtest.h"

#include "data3d/zstackobjecthandle.h"

TEST(ZStackObjectHandle, Basic)
{
  ZStackObjectHandle handle1;
  ZStackObjectHandle handle2;

  ZStackObjectHandle handle3 = handle1;

  ASSERT_EQ(handle1, handle3);
  ASSERT_NE(handle1, handle2);

}

#endif

#endif // ZSTACKOBJECTHANDLETEST_H

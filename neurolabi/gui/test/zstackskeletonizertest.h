#ifndef ZSTACKSKELETONIZERTEST_H
#define ZSTACKSKELETONIZERTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zstackskeletonizer.h"

#ifdef _USE_GTEST_
TEST(ZStackSkeletonizer, Configure)
{
  ZStackSkeletonizer skeletonizer;
  skeletonizer.setDownsampleInterval(1, 2, 3);
  int xintv = 0;
  int yintv = 0;
  int zintv = 0;

  skeletonizer.getDownsampleInterval(&xintv, &yintv, &zintv);
  ASSERT_EQ(1, xintv);
  ASSERT_EQ(2, yintv);
  ASSERT_EQ(3, zintv);

  skeletonizer.setDownsampleInterval(-1, 2, -3);
  skeletonizer.getDownsampleInterval(&xintv, &yintv, &zintv);
  ASSERT_EQ(0, xintv);
  ASSERT_EQ(2, yintv);
  ASSERT_EQ(0, zintv);

}

#endif


#endif // ZSTACKSKELETONIZERTEST_H

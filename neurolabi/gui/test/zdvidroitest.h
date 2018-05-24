#ifndef ZDVIDROITEST_H
#define ZDVIDROITEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "dvid/zdvidroi.h"

#ifdef _USE_GTEST_

TEST(ZDvidRoi, Basic)
{
  ZDvidRoi roi;

  ASSERT_FALSE(roi.contains(0, 0, 0));

  roi.getRoiRef()->addSegment(0, 0, 0, 0);
  ASSERT_FALSE(roi.contains(0, 0, 0));

  roi.setBlockSize(1, 1, 1);
  ASSERT_TRUE(roi.contains(0, 0, 0));

  roi.clear();
  ASSERT_FALSE(roi.contains(0, 0, 0));

  roi.getRoiRef()->addSegment(1, 2, 3, 3);
  roi.setBlockSize(1, 1, 1);
  ASSERT_FALSE(roi.contains(0, 0, 0));
  ASSERT_TRUE(roi.contains(3, 2, 1));
  ASSERT_FALSE(roi.contains(4, 2, 1));

  roi.setBlockSize(2, 2, 2);
  ASSERT_TRUE(roi.contains(6, 4, 2));
  ASSERT_TRUE(roi.contains(7, 5, 3));
  ASSERT_FALSE(roi.contains(8, 5, 3));


#if 0
  ZDvidRoi roi;
  roi.setDvidTarget(target);
  roi.update(name);

  ASSERT_TRUE(roi.contains(name, pt));
#endif
}

#endif

#endif // ZDVIDROITEST_H

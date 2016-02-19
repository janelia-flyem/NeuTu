#ifndef ZLINESEGMENTTEST_H
#define ZLINESEGMENTTEST_H

#include "ztestheader.h"
#include "zlinesegment.h"

#ifdef _USE_GTEST_

TEST(ZLineSegment, basic)
{
  ZLineSegment seg(0, 0, 0, 0, 0, 0);
  ASSERT_DOUBLE_EQ(0.0, seg.getLength());

  seg.setEndPoint(1, 0, 0);
  ASSERT_DOUBLE_EQ(1.0, seg.getLength());

  ZPoint pt = seg.getInterpolation(0.5);
  ASSERT_DOUBLE_EQ(0.5, pt.x());

  seg.setEndPoint(5, 0, 0);
  pt = seg.getInterpolation(0.5);
  ASSERT_DOUBLE_EQ(0.5, pt.x());

  pt = seg.getInterpolation(1.5);
  ASSERT_DOUBLE_EQ(1.5, pt.x());
}

#endif


#endif // ZLINESEGMENTTEST_H

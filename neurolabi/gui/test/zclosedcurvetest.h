#ifndef ZCLOSEDCURVETEST_H
#define ZCLOSEDCURVETEST_H

#include "ztestheader.h"
#include "zlinesegment.h"
#include "zclosedcurve.h"

#ifdef _USE_GTEST_
TEST(ZClosedCurve, basic)
{
  ZClosedCurve curve;
  ASSERT_TRUE(curve.isEmpty());

  curve.append(ZPoint(0, 0, 0));
  ASSERT_EQ(1, (int) curve.getLandmarkNumber());
  ASSERT_EQ(0, (int) curve.getSegmentNumber());
  ASSERT_DOUBLE_EQ(0.0, curve.getLength());

  curve.append(ZPoint(1, 0, 0));
  ASSERT_EQ(2, (int) curve.getLandmarkNumber());
  ASSERT_EQ(2, (int) curve.getSegmentNumber());
  ASSERT_DOUBLE_EQ(2.0, curve.getLength());

  ZPoint pt = curve.getLandmark(0);
  ASSERT_DOUBLE_EQ(0.0, pt.distanceTo(ZPoint(0, 0, 0)));

  pt = curve.getLandmark(1);
  ASSERT_DOUBLE_EQ(0.0, pt.distanceTo(ZPoint(1, 0, 0)));

  pt = curve.getLandmark(2);
  ASSERT_DOUBLE_EQ(0.0, pt.distanceTo(ZPoint(0, 0, 0)));

  pt = curve.getLandmark(3);
  ASSERT_DOUBLE_EQ(0.0, pt.distanceTo(ZPoint(1, 0, 0)));

  pt = curve.getLandmark(-1);
  ASSERT_DOUBLE_EQ(0.0, pt.distanceTo(ZPoint(1, 0, 0)));

  pt = curve.getLandmark(-2);
  ASSERT_DOUBLE_EQ(0.0, pt.distanceTo(ZPoint(0, 0, 0)));

  curve.append(ZPoint(1, 1, 1));
  pt = curve.getLandmark(-1);
  ASSERT_DOUBLE_EQ(0.0, pt.distanceTo(ZPoint(1, 1, 1)));

  pt = curve.getLandmark(-2);
  ASSERT_DOUBLE_EQ(0.0, pt.distanceTo(ZPoint(1, 0, 0)));

  pt = curve.getLandmark(-3);
  ASSERT_DOUBLE_EQ(0.0, pt.distanceTo(ZPoint(0, 0, 0)));

  ASSERT_DOUBLE_EQ(0.0, curve.getLandmark(2).distanceTo(
                     curve.getLandmark(2 + 5 * curve.getLandmarkNumber())));
  ASSERT_DOUBLE_EQ(0.0, curve.getLandmark(2).distanceTo(
                     curve.getLandmark(2 + -5 * (int) curve.getLandmarkNumber())));
}

TEST(ZClosedCurve, match)
{
  ZClosedCurve curve;
  curve.append(ZPoint(0, 0, 0));
  curve.append(1, 0, 0);
  curve.append(1, 1, 1);
  ASSERT_EQ(0, (int) curve.getMinXIndex());

  curve.append(-1, 0, 0);
  ASSERT_EQ(3, (int) curve.getMinXIndex());

  curve.append(3, 0, 0);
  ASSERT_EQ(3,  (int) curve.getMinXIndex());

  ZClosedCurve curve2 = curve;
  ASSERT_EQ(0, curve.findMatchShift(curve2));

  curve2.append(-4, 0, 0);
  ASSERT_EQ(0, curve.findMatchShift(curve2));

}

TEST(ZClosedCurve, resample)
{
  ZClosedCurve curve;
  curve.append(ZPoint(0, 0, 0));
  curve.append(1, 0, 0);

  ZClosedCurve curve2 = curve.resampleF(5);
  //curve2.print();
  ASSERT_EQ(5, (int) curve2.getLandmarkNumber());

  curve.append(1, 1, 1);
  curve2 = curve.resampleF(9);
  curve2.print();
  ASSERT_EQ(9, (int) curve2.getLandmarkNumber());
}

#endif

#endif // ZCLOSEDCURVETEST_H

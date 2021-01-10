#ifndef ZLOCALNEUROSEGTEST_H
#define ZLOCALNEUROSEGTEST_H

#include "ztestheader.h"
#include "zlocalneuroseg.h"
#include "geometry/zpointarray.h"

#ifdef _USE_GTEST_

TEST(ZLocalNeuroseg, Field)
{
  Local_Neuroseg *locseg = New_Local_Neuroseg();

  double x = 10.0;
  double y = 20.0;
  double z = 30.0;

  double r = 5.0;

  Set_Local_Neuroseg(
        locseg,  r, 0.0, NEUROSEG_DEFAULT_H, 0.0, 0.0, 0.0, 0.0, 1.0,
        x, y, z);
  ZLocalNeuroseg s(locseg);

  ZPointArray points = s.sample(1.0, 1.0);

  locseg->seg.r1 = 5.1;
  locseg->seg.h = 12.1;
  for (const ZPoint &pt : points) {
    ASSERT_TRUE(Local_Neuroseg_Hit_Test(locseg, pt.x(), pt.y(), pt.z()))
        << pt.toString();
  }
}

#endif

#endif // ZLOCALNEUROSEGTEST_H

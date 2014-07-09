#ifndef Z3DINTERACTTEST_H
#define Z3DINTERACTTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "geometry/zgeo3dtransform.h"
#include "zstack.hxx"

#ifdef _USE_GTEST_

TEST(3DPaint, Basic)
{
  ZStack stack;
  stack.readStack(GET_TEST_DATA_DIR + "/ball.tif");
  ZGeo3dScalarField *field;

  ZGeo3dTransform transform;
  transform.setRotationMatrix();
}

#endif

#endif // Z3DINTERACTTEST_H

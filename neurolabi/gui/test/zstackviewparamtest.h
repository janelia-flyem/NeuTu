#ifndef ZSTACKVIEWPARAMTEST_H
#define ZSTACKVIEWPARAMTEST_H

#include "ztestheader.h"
#include "zstackviewparam.h"

#ifdef _USE_GTEST_
TEST(ZStackViewParam, Basic)
{
  ZStackViewParam param;
  ASSERT_EQ(neutu::EAxis::Z, param.getSliceAxis());
  ASSERT_FALSE(param.isValid());

  param.setWidgetRect(QRect(0, 0, 100, 200));
  param.setCanvasRect(QRect(0, 0, 300, 400));
  param.setViewPort(QRect(10, 20, 50, 60));
  ASSERT_TRUE(param.isValid());

  param.closeViewPort();
  ASSERT_FALSE(param.isValid());

  param.openViewPort();
  ASSERT_TRUE(param.isValid());

  ZStackViewParam param2 = param;
  param2.setViewPort(QRect(15, 25, 20, 30));
  ASSERT_TRUE(param.contains(param2));

  param.closeViewPort();
  ASSERT_FALSE(param.contains(param2));

  param.openViewPort();
  param2.setZ(param.getZ() + 1);
  ASSERT_FALSE(param.contains(param2));
  ASSERT_TRUE(param.containsViewport(param2));

  param.setSliceAxis(neutu::EAxis::X);
  param2.setZ(param.getZ());
  ASSERT_FALSE(param.contains(param2));
  param2.setSliceAxis(param.getSliceAxis());
  ASSERT_TRUE(param.contains(param2));

  param2.setViewPort(QRect(15, 25, 200, 300));
  ASSERT_FALSE(param.contains(param2));
  param2.closeViewPort();
  ASSERT_TRUE(param.contains(param2));

  param.closeViewPort();
  ASSERT_TRUE(param.contains(param2));

}

#endif

#endif // ZSTACKVIEWPARAMTEST_H

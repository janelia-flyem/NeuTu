#ifndef ZSTACKVIEWTEST_H
#define ZSTACKVIEWTEST_H

#ifdef _USE_GTEST_

#include "gtest/gtest.h"

#include "mvc/zstackview.h"

TEST(ZStackView, Construct)
{
  ZStackView *view = new ZStackView();
  ASSERT_EQ(1, view->getViewId());

  view = new ZStackView();
  ASSERT_EQ(2, view->getViewId());
}

#endif


#endif // ZSTACKVIEWTEST_H

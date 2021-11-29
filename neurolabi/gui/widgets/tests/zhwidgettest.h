#ifndef ZHWIDGETTEST_H
#define ZHWIDGETTEST_H

#ifdef _USE_GTEST_

#include "gtest/gtest.h"

#include "widgets/zhwidget.h"

TEST(ZHWidget, Basic)
{
  ZHWidget widget(1);

  ASSERT_FALSE(widget.assumingVisible());

  widget.addWidget(new QWidget, 1);
  ASSERT_FALSE(widget.assumingVisible());

  widget.addWidget(new QWidget, 0);
  ASSERT_TRUE(widget.assumingVisible());
}

#endif

#endif // ZHWIDGETTEST_H

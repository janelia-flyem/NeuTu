#ifndef ZGROUPVISWIDGETTEST_H
#define ZGROUPVISWIDGETTEST_H

#ifdef _USE_GTEST_

#include "gtest/gtest.h"

#include <QHBoxLayout>
#include <QPushButton>
#include "widgets/zgroupviswidget.h"

TEST(ZGroupVisWidget, Basic)
{
  ZGroupVisWidget widget(nullptr);

  ASSERT_TRUE(widget.isDefaultVisible());

  widget.setDefaultVisible(false);
  ASSERT_FALSE(widget.isDefaultVisible());

  widget.toggleDefaultVisible();
  ASSERT_TRUE(widget.isDefaultVisible());

  widget.toggleDefaultVisible();
  ASSERT_FALSE(widget.isDefaultVisible());

  widget.setDefaultVisible(true);
  ASSERT_FALSE(widget.assumingVisible());

  QWidget *child = new QWidget;
  widget.setLayout(new QHBoxLayout(&widget));
  widget.layout()->addWidget(child);
  ASSERT_FALSE(widget.assumingVisible(child));

  widget.assumeVisible(child, true);
  ASSERT_TRUE(widget.assumingVisible(child));
  ASSERT_TRUE(widget.assumingVisible());

  widget.assumeVisible(child, false);
  ASSERT_FALSE(widget.assumingVisible(child));
  ASSERT_FALSE(widget.assumingVisible());

  widget.assumeVisible(child, true);
  ASSERT_TRUE(widget.assumingVisible(child));
  ASSERT_TRUE(widget.assumingVisible());

  widget.setDefaultVisible(false);
  ASSERT_TRUE(widget.assumingVisible(child));
  ASSERT_FALSE(widget.assumingVisible());

  widget.setDefaultVisible(true);
  ASSERT_TRUE(widget.assumingVisible(child));
  ASSERT_TRUE(widget.assumingVisible());

  widget.assumeVisible(child, false);

  ZGroupVisWidget *groupChild = new ZGroupVisWidget;
  groupChild->setLayout(new QHBoxLayout);
  widget.assumeVisible(groupChild, true);
  ASSERT_FALSE(groupChild->assumingVisible());
  ASSERT_FALSE(widget.assumingVisible(groupChild));
  ASSERT_FALSE(widget.assumingVisible());

  QWidget *child2 = new QWidget;
  groupChild->assumeVisible(child2, true);
  ASSERT_TRUE(groupChild->assumingVisible(child2));
  ASSERT_FALSE(groupChild->assumingVisible());
  groupChild->layout()->addWidget(child2);
  ASSERT_TRUE(groupChild->assumingVisible());
  ASSERT_TRUE(widget.assumingVisible(groupChild));
  ASSERT_FALSE(widget.assumingVisible());

  widget.layout()->addWidget(groupChild);
  ASSERT_TRUE(widget.assumingVisible());

  groupChild->setDefaultVisible(false);
  ASSERT_FALSE(widget.assumingVisible(groupChild));
  ASSERT_FALSE(widget.assumingVisible());

  groupChild->setDefaultVisible(true);
  ASSERT_TRUE(widget.assumingVisible(groupChild));
  ASSERT_TRUE(widget.assumingVisible());

  groupChild->setDefaultVisible(false);
  ASSERT_FALSE(widget.assumingVisible(groupChild));
  ASSERT_FALSE(widget.assumingVisible());
  widget.assumeVisible(child, true);
  ASSERT_FALSE(widget.assumingVisible(groupChild));
  ASSERT_TRUE(widget.assumingVisible());
}

#endif

#endif // ZGROUPVISWIDGETTEST_H

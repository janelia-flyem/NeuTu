#ifndef ZSTROKE2DTEST_H
#define ZSTROKE2DTEST_H

#include "ztestheader.h"

#include "zstroke2d.h"

#ifdef _USE_GTEST_

TEST(ZStroke2d, Basic)
{
  ZStroke2d stroke;

  ASSERT_TRUE(stroke.isEmpty());

  ASSERT_EQ(-1, stroke.getZ());

  stroke.append(1, 2);
  ASSERT_EQ(1, stroke.getPointNumber());
  stroke.append(3, 4);
  ASSERT_EQ(2, stroke.getPointNumber());

  stroke.setLast(5, 6);
  ASSERT_EQ(2, stroke.getPointNumber());
  stroke.append(3, 4, 1);
  ASSERT_EQ(1, stroke.getZ());
  stroke.append(3, 4, 0.1);
  ASSERT_EQ(0, stroke.getZ());
  stroke.updateWithLast(3, 4, 7.1);
  ASSERT_EQ(7, stroke.getZ());
  stroke.set(3, 4);
  ASSERT_EQ(1, stroke.getPointNumber());

  stroke.set(3, 4, 5);
  ASSERT_EQ(5, stroke.getZ());

  stroke.set(3, 4, 6.1);
  ASSERT_EQ(6, stroke.getZ());

  stroke.setZ(0.1);
  ASSERT_EQ(0, stroke.getZ());
  stroke.setZ(0.5);
  ASSERT_EQ(1, stroke.getZ());
  stroke.setZ(0.6);
  ASSERT_EQ(1, stroke.getZ());
  stroke.setZ(-0.1);
  ASSERT_EQ(0, stroke.getZ());
  stroke.setZ(-0.5);
  ASSERT_EQ(0, stroke.getZ());
  stroke.setZ(-0.6);
  ASSERT_EQ(-1, stroke.getZ());

  stroke.setZRounding(false);
  stroke.clear();
  ASSERT_TRUE(stroke.isEmpty());
  stroke.append(3, 4, 0.1);
  ASSERT_DOUBLE_EQ(0.1, stroke.getZ());
  stroke.updateWithLast(3, 4, 0.5);
  ASSERT_EQ(0.5, stroke.getZ());

  stroke.setZ(0.5);
  ASSERT_EQ(0.5, stroke.getZ());
  stroke.setZ(-0.1);
  ASSERT_DOUBLE_EQ(-0.1, stroke.getZ());
}

TEST(ZStroke2d, Label)
{
  ZStroke2d stroke;

  stroke.setLabel(2);
  ASSERT_EQ(2, stroke.getLabel());
  ASSERT_EQ(qRgb(0, 255, 0), stroke.getColor().rgb());

  stroke.toggleLabel(true);
  ASSERT_EQ(3, stroke.getLabel());
  stroke.toggleLabel(true);
  ASSERT_EQ(3, stroke.getLabel());
  stroke.toggleLabel(false);
  ASSERT_EQ(2, stroke.getLabel());
}

#endif

#endif // ZSTROKE2DTEST_H

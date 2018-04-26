#ifndef ZSTACKDOCHELPERTEST_H
#define ZSTACKDOCHELPERTEST_H

#include "ztestheader.h"
#include "zstackdoc.h"
#include "zstackdochelper.h"

#ifdef _USE_GTEST_

TEST(ZStackDocHelper, Range)
{
  ZStackDoc doc;
  ZIntCuboid range;
  range.setFirstCorner(10, 20, 30);
  range.setSize(100, 200, 300);
  doc.loadStack(ZStackFactory::MakeVirtualStack(range));

  ZIntCuboid box = ZStackDocHelper::GetDataSpaceRange(doc);
  ASSERT_EQ(ZIntPoint(10, 20, 30), box.getFirstCorner());
  ASSERT_EQ(ZIntPoint(109, 219, 329), box.getLastCorner());

  box = ZStackDocHelper::GetStackSpaceRange(doc, neutube::Z_AXIS);
  ASSERT_EQ(ZIntPoint(10, 20, 30), box.getFirstCorner());
  ASSERT_EQ(ZIntPoint(109, 219, 329), box.getLastCorner());

  box = ZStackDocHelper::GetStackSpaceRange(doc, neutube::X_AXIS);
  ASSERT_EQ(ZIntPoint(30, 20, 10), box.getFirstCorner());
  ASSERT_EQ(ZIntPoint(329, 219, 109), box.getLastCorner());

  box = ZStackDocHelper::GetStackSpaceRange(doc, neutube::Y_AXIS);
  ASSERT_EQ(ZIntPoint(10, 30, 20), box.getFirstCorner());
  ASSERT_EQ(ZIntPoint(109, 329, 219), box.getLastCorner());

  box = ZStackDocHelper::GetStackSpaceRange(doc, neutube::A_AXIS);
  ASSERT_EQ(iround(range.getDiagonalLength()), box.getWidth());
  ASSERT_EQ(iround(range.getDiagonalLength()), box.getHeight());
  ASSERT_EQ(iround(range.getDiagonalLength()), box.getDepth());
  ASSERT_EQ(range.getCenter(), box.getCenter());
}

#endif

#endif // ZSTACKDOCHELPERTEST_H

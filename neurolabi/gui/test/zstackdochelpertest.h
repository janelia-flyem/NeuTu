#ifndef ZSTACKDOCHELPERTEST_H
#define ZSTACKDOCHELPERTEST_H

#include "ztestheader.h"
#include "mvc/zstackdoc.h"
#include "mvc/zstackdochelper.h"
#include "mvc/zstackdocutil.h"

#ifdef _USE_GTEST_

TEST(ZStackDocUtil, Range)
{
  ZStackDoc doc;
  ZIntCuboid range;
  range.setFirstCorner(10, 20, 30);
  range.setSize(100, 200, 300);
  doc.loadStack(ZStackFactory::MakeVirtualStack(range));

  ZIntCuboid box = ZStackDocUtil::GetDataSpaceRange(doc);
  ASSERT_EQ(ZIntPoint(10, 20, 30), box.getFirstCorner());
  ASSERT_EQ(ZIntPoint(109, 219, 329), box.getLastCorner());

  box = ZStackDocUtil::GetStackSpaceRange(doc, neutu::EAxis::Z);
  ASSERT_EQ(ZIntPoint(10, 20, 30), box.getFirstCorner());
  ASSERT_EQ(ZIntPoint(109, 219, 329), box.getLastCorner());

  box = ZStackDocUtil::GetStackSpaceRange(doc, neutu::EAxis::X);
  ASSERT_EQ(ZIntPoint(30, 20, 10), box.getFirstCorner());
  ASSERT_EQ(ZIntPoint(329, 219, 109), box.getLastCorner());

  box = ZStackDocUtil::GetStackSpaceRange(doc, neutu::EAxis::Y);
  ASSERT_EQ(ZIntPoint(10, 30, 20), box.getFirstCorner());
  ASSERT_EQ(ZIntPoint(109, 329, 219), box.getLastCorner());

  box = ZStackDocUtil::GetStackSpaceRange(doc, neutu::EAxis::ARB);
  ASSERT_EQ(iround(range.getDiagonalLength()), box.getWidth());
  ASSERT_EQ(iround(range.getDiagonalLength()), box.getHeight());
  ASSERT_EQ(iround(range.getDiagonalLength()), box.getDepth());
  ASSERT_EQ(range.getCenter(), box.getCenter());
}

#endif

#endif // ZSTACKDOCHELPERTEST_H

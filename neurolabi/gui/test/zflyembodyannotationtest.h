#ifndef ZFLYEMBODYANNOTATIONTEST_H
#define ZFLYEMBODYANNOTATIONTEST_H

#include "ztestheader.h"
#include "zflyembodyannotation.h"

#ifdef _USE_GTEST_

TEST(ZFlyEmBodyAnnotation, merge)
{
  ZFlyEmBodyAnnotation annotation1;
  ZFlyEmBodyAnnotation annotation2;

  annotation1.setStatus("Orphan");
  annotation2.setStatus("");

  annotation1.mergeAnnotation(annotation2);
  ASSERT_TRUE(annotation1.getStatus().empty());

  annotation1.setStatus("Orphan");

  annotation2.setStatus("Hard to trace");
  annotation1.mergeAnnotation(annotation2);
  ASSERT_EQ("Hard to trace", annotation1.getStatus());

  annotation1.setStatus("Not examined");
  annotation1.mergeAnnotation(annotation2);
  ASSERT_EQ("Not examined", annotation1.getStatus());


  annotation2.setStatus("Partially traced");
  annotation1.mergeAnnotation(annotation2);
  ASSERT_EQ("Partially traced", annotation1.getStatus());

  annotation2.setStatus("Traced in ROI");
  annotation1.mergeAnnotation(annotation2);
  ASSERT_EQ("Traced in ROI", annotation1.getStatus());

  annotation2.setStatus("traced");
  annotation1.mergeAnnotation(annotation2);
  ASSERT_EQ("traced", annotation1.getStatus());

  annotation2.setStatus("Finalized");
  annotation1.mergeAnnotation(annotation2);
  ASSERT_EQ("Finalized", annotation1.getStatus());


}

#endif

#endif // ZFLYEMBODYANNOTATIONTEST_H

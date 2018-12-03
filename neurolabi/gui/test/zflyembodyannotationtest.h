#ifndef ZFLYEMBODYANNOTATIONTEST_H
#define ZFLYEMBODYANNOTATIONTEST_H

#include "ztestheader.h"
#include "flyem/zflyembodyannotation.h"
#include "zjsonobject.h"

#ifdef _USE_GTEST_

TEST(ZFlyEmBodyAnnotation, Basic)
{
  ZFlyEmBodyAnnotation annot;
  ASSERT_EQ(uint64_t(0), annot.getBodyId());

  ZJsonObject json;
  json.setEntry("status", "Hard to trace");
  json.setEntry("comment", "test");
  json.setEntry("body ID", 123);
  json.setEntry("name", "KC");
  json.setEntry("class", "neuron");
  json.setEntry("user", "zhaot");
  json.setEntry("naming user", "mock");

  annot.loadJsonObject(json);

  ASSERT_EQ(uint64_t(123), annot.getBodyId());
  ASSERT_EQ("Hard to trace", annot.getStatus());
  ASSERT_EQ("test", annot.getComment());
  ASSERT_EQ("KC", annot.getName());
  ASSERT_EQ("neuron", annot.getType());
  ASSERT_EQ("zhaot", annot.getUser());
  ASSERT_EQ("mock", annot.getNamingUser());

  annot.clear();
  ASSERT_EQ(uint64_t(0), annot.getBodyId());
  ASSERT_EQ("", annot.getStatus());
  ASSERT_EQ("", annot.getComment());
  ASSERT_EQ("", annot.getName());
  ASSERT_EQ("", annot.getType());
  ASSERT_EQ("", annot.getUser());
  ASSERT_EQ("", annot.getNamingUser());
}

TEST(ZFlyEmBodyAnnotation, merge)
{
  ZFlyEmBodyAnnotation annotation1;
  ZFlyEmBodyAnnotation annotation2;

  annotation1.setStatus("Orphan");
  annotation2.setStatus("");

  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Orphan", annotation1.getStatus());

  annotation1.setStatus("Orphan");

  annotation2.setStatus("Orphan hotknife");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Orphan hotknife", annotation1.getStatus());

  annotation2.setStatus("Leaves");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Leaves", annotation1.getStatus());

  annotation2.setStatus("Hard to trace");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Hard to trace", annotation1.getStatus());

  annotation1.setStatus("Not examined");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Hard to trace", annotation1.getStatus());

  annotation2.setStatus("Partially traced");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Partially traced", annotation1.getStatus());

  annotation2.setStatus("Prelim Roughly traced");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Prelim Roughly traced", annotation1.getStatus());

  annotation2.setStatus("Roughly traced");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Roughly traced", annotation1.getStatus());

  annotation2.setStatus("Traced in ROI");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Traced in ROI", annotation1.getStatus());

  annotation2.setStatus("traced");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("traced", annotation1.getStatus());

  annotation2.setStatus("Finalized");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Finalized", annotation1.getStatus());
}

#endif

#endif // ZFLYEMBODYANNOTATIONTEST_H

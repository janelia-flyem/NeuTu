#ifndef ZDVIDANNOTATIONTEST_H
#define ZDVIDANNOTATIONTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "dvid/zdvidannotation.h"
#include "zjsonobject.h"
#include "zjsonarray.h"

#ifdef _USE_GTEST_

TEST(ZDvidAnnotation, Json)
{
  ASSERT_EQ("PreSynTo", ZDvidAnnotation::GetMatchingRelation("PostSynTo"));
  ASSERT_EQ("PostSynTo", ZDvidAnnotation::GetMatchingRelation("PreSynTo"));
  ASSERT_EQ("PostSynTo", ZDvidAnnotation::GetMatchingRelation("ConvergentTo"));

  ASSERT_EQ("UnknownRelationship",
            ZDvidAnnotation::GetMatchingRelation("UnknowRelationship"));
  ASSERT_EQ("UnknownRelationship", ZDvidAnnotation::GetMatchingRelation("xxx"));

  ASSERT_EQ("GroupedWith", ZDvidAnnotation::GetMatchingRelation("GroupedWith"));

  ZJsonArray relArray;
  ZJsonObject relJson =
      ZDvidAnnotation::MakeRelJson(ZIntPoint(1, 2, 3), "PreSynTo");
  relArray.append(relJson);

  relJson = ZDvidAnnotation::MakeRelJson(ZIntPoint(4, 5, 6), "PreSynTo");
  relArray.append(relJson);

  int index = ZDvidAnnotation::MatchRelation(
        relArray, ZIntPoint(1, 2, 3), "PostSynTo");
  ASSERT_EQ(0, index);
  index = ZDvidAnnotation::MatchRelation(
          relArray, ZIntPoint(4, 5, 6), "PostSynTo");
  ASSERT_EQ(1, index);
  index = ZDvidAnnotation::MatchRelation(
          relArray, ZIntPoint(4, 5, 7), "PostSynTo");
  ASSERT_EQ(-1, index);
}

#endif

#endif // ZDVIDANNOTATIONTEST_H

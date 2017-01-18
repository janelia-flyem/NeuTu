#ifndef ZDVIDANNOTATIONTEST_H
#define ZDVIDANNOTATIONTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "dvid/zdvidannotation.h"
#include "zjsonobject.h"
#include "zjsonarray.h"
#include "flyem/zflyemtodoitem.h"

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

  ZDvidAnnotation annot;
  ZJsonObject jsonObj = annot.toJsonObject();
  ASSERT_FALSE(jsonObj.hasKey("Prop"));
  ZDvidAnnotation::AddProperty(jsonObj, "test", true);
  ASSERT_TRUE(jsonObj.hasKey("Prop"));
  ZJsonObject propJson(jsonObj.value("Prop"));

//  std::cout << ZJsonParser::stringValue(propJson.value("test").getData()) << std::endl;

  ASSERT_STREQ("1", ZJsonParser::stringValue(propJson.value("test").getData()));

  ZDvidAnnotation::AddProperty(jsonObj, "test2", "true");
  ASSERT_STREQ("true", ZJsonParser::stringValue(propJson.value("test2").getData()));

}

TEST(ZDvidAnnotation, ZFlyEmToDoItem)
{
  ZFlyEmToDoItem item;
//  item.toJsonObject().print();

  item.setChecked(true);
  item.setAction(ZFlyEmToDoItem::TO_MERGE);
//  item.toJsonObject().print();
  ASSERT_EQ(ZFlyEmToDoItem::TO_MERGE, item.getAction());

  item.setAction(ZFlyEmToDoItem::TO_SPLIT);
  ASSERT_EQ(ZFlyEmToDoItem::TO_SPLIT, item.getAction());

  item.setAction(ZFlyEmToDoItem::TO_DO);
//  item.toJsonObject().print();
  ASSERT_EQ(ZFlyEmToDoItem::TO_DO, item.getAction());


}

#endif

#endif // ZDVIDANNOTATIONTEST_H

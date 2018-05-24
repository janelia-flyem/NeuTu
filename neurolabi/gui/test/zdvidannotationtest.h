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
  item.setAction(neutube::TO_MERGE);
//  item.toJsonObject().print();
  ASSERT_EQ(neutube::TO_MERGE, item.getAction());

  item.setAction(neutube::TO_SPLIT);
  ASSERT_EQ(neutube::TO_SPLIT, item.getAction());

  item.setAction(neutube::TO_DO);
//  item.toJsonObject().print();
  ASSERT_EQ(neutube::TO_DO, item.getAction());

  std::string splitTag = std::string(ZFlyEmToDoItem::ACTION_KEY) + ":"
      + ZFlyEmToDoItem::ACTION_SPLIT_TAG;
  ASSERT_FALSE(item.hasTag(splitTag));

  item.addTag(splitTag);
  ASSERT_TRUE(item.hasTag(splitTag));

  item.removeActionTag();
  ASSERT_FALSE(item.hasTag(splitTag));

  item.setAction(neutube::TO_SPLIT);
  ASSERT_TRUE(item.hasTag(splitTag));

  item.setChecked(true);
  ASSERT_FALSE(item.hasTag(splitTag));

  item.setChecked(false);
  ASSERT_TRUE(item.hasTag(splitTag));

}

TEST(ZDvidAnnotation, Radius)
{
  ZResolution resolution;
  resolution.setUnit(ZResolution::UNIT_PIXEL);
  resolution.setVoxelSize(1.0, 1.0, 1.0);
  ASSERT_DOUBLE_EQ(
        ZDvidAnnotation::GetDefaultRadius(ZDvidAnnotation::KIND_POST_SYN),
        ZDvidAnnotation::GetDefaultRadius(
          ZDvidAnnotation::KIND_POST_SYN, resolution));
}

#endif

#endif // ZDVIDANNOTATIONTEST_H

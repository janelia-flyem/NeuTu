#ifndef ZDVIDANNOTATIONTEST_H
#define ZDVIDANNOTATIONTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonobjectparser.h"
#include "dvid/zdvidannotation.h"
#include "dvid/zdvidsynapse.h"
#include "flyem/zflyemtodoitem.h"

#ifdef _USE_GTEST_

TEST(ZDvidAnnotation, Basic)
{
  ZDvidAnnotation annot;
  annot.setPosition(1, 2, 3);
  annot.addProperty("test", "t1");
  ASSERT_EQ("t1", annot.getProperty<std::string>("test"));

  ZDvidAnnotation annot2 = annot;
  ASSERT_EQ("t1", annot2.getProperty<std::string>("test"));

  annot.addProperty("test2", "t2");
  ASSERT_EQ("t2", annot.getProperty<std::string>("test2"));
  ASSERT_EQ("", annot2.getProperty<std::string>("test2"));
}

TEST(ZDvidAnnotation, Property)
{
  ZDvidAnnotation annot;
  annot.setPosition(1, 2, 3);
  ASSERT_EQ(ZIntPoint(1, 2, 3), annot.getPosition());
  annot.setRadius(1.0);
  ASSERT_EQ(1.0, annot.getRadius());

  annot.setKind(ZDvidAnnotation::EKind::KIND_PRE_SYN);
  ASSERT_EQ(ZDvidAnnotation::EKind::KIND_PRE_SYN, annot.getKind());

  annot.setComment("test");
  ASSERT_EQ("test", annot.getComment());

  annot.setComment("");
  ASSERT_TRUE(annot.getComment().empty());
  ASSERT_FALSE(annot.hasProperty(ZDvidAnnotation::KEY_COMMENT));
}

TEST(ZDvidAnnotation, Partner)
{
  ZDvidAnnotation annot;
  annot.setPosition(1, 2, 3);

  annot.setKind(ZDvidAnnotation::EKind::KIND_POST_SYN);
  ASSERT_FALSE(annot.isPrimaryPartner());

  annot.setKind(ZDvidAnnotation::EKind::KIND_PRE_SYN);
  ASSERT_TRUE(annot.isPrimaryPartner());
  ASSERT_FALSE(annot.hasPartnerIn({ZIntPoint(1, 2, 3), ZIntPoint(4, 5, 6)}));
  annot.addPartner(4, 5, 6);
  ASSERT_TRUE(annot.hasPartnerIn({ZIntPoint(1, 2, 3), ZIntPoint(4, 5, 6)}));

  annot.addPartner(7, 8, 9);
  ASSERT_TRUE(annot.hasPartnerIn({ZIntPoint(1, 2, 3), ZIntPoint(7, 8, 9)}));
  ASSERT_TRUE(annot.hasPartnerIn({ZIntPoint(1, 2, 3), ZIntPoint(4, 5, 6)}));
  ASSERT_FALSE(annot.hasPartnerIn({ZIntPoint(1, 2, 3), ZIntPoint(7, 5, 6)}));
}

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

  ASSERT_EQ("1", ZJsonParser::stringValue(propJson.value("test").getData()))
      << propJson.dumpString(0);

  ZDvidAnnotation::AddProperty(jsonObj, "test2", "true");
  ASSERT_EQ("true", ZJsonParser::stringValue(propJson.value("test2").getData()));

  ASSERT_FALSE(ZDvidAnnotation::IsChecked(jsonObj));
  ZDvidAnnotation::AddProperty(jsonObj, "checked", true);
  ASSERT_TRUE(ZDvidAnnotation::IsChecked(jsonObj));

  ZJsonObject jsonObj2;
  ASSERT_FALSE(ZDvidAnnotation::HasSameSubProp(jsonObj, jsonObj2));
  ASSERT_FALSE(ZDvidAnnotation::HasSameSubProp(jsonObj2, jsonObj));
  jsonObj2 = jsonObj.clone();
  ASSERT_TRUE(ZDvidAnnotation::HasSameSubProp(jsonObj, jsonObj2));
  ASSERT_TRUE(ZDvidAnnotation::HasSameSubProp(jsonObj2, jsonObj));

  ZDvidAnnotation::AddProperty(jsonObj, "checked", true);
  ASSERT_TRUE(ZDvidAnnotation::HasSameSubProp(jsonObj, jsonObj2));
  ASSERT_TRUE(ZDvidAnnotation::HasSameSubProp(jsonObj2, jsonObj));

  ZDvidAnnotation::AddProperty(jsonObj2, "checked", false);
  ASSERT_TRUE(ZDvidAnnotation::HasSameSubProp(jsonObj, jsonObj2));
  ASSERT_TRUE(ZDvidAnnotation::HasSameSubProp(jsonObj2, jsonObj));

  ZDvidAnnotation::AddProperty(jsonObj2, "user", "test");
  ASSERT_TRUE(ZDvidAnnotation::HasSameSubProp(jsonObj, jsonObj2));
  ASSERT_TRUE(ZDvidAnnotation::HasSameSubProp(jsonObj2, jsonObj));

  ZDvidAnnotation::AddProperty(jsonObj, "p1", 1);
  ASSERT_FALSE(ZDvidAnnotation::HasSameSubProp(jsonObj, jsonObj2));
  ASSERT_FALSE(ZDvidAnnotation::HasSameSubProp(jsonObj2, jsonObj));
  ZDvidAnnotation::AddProperty(jsonObj2, "p1", 1);
  ASSERT_TRUE(ZDvidAnnotation::HasSameSubProp(jsonObj, jsonObj2));
  ASSERT_TRUE(ZDvidAnnotation::HasSameSubProp(jsonObj2, jsonObj));

  ZDvidAnnotation::AddProperty(jsonObj2, "p2", 1);
  ASSERT_FALSE(ZDvidAnnotation::HasSameSubProp(jsonObj, jsonObj2));
  ASSERT_FALSE(ZDvidAnnotation::HasSameSubProp(jsonObj2, jsonObj));

  jsonObj.removeKey("Prop");
  ASSERT_FALSE(ZDvidAnnotation::HasSameSubProp(jsonObj, jsonObj2));
  ASSERT_FALSE(ZDvidAnnotation::HasSameSubProp(jsonObj2, jsonObj));

  jsonObj2.removeKey("Prop");
  ASSERT_TRUE(ZDvidAnnotation::HasSameSubProp(jsonObj, jsonObj2));
  ASSERT_TRUE(ZDvidAnnotation::HasSameSubProp(jsonObj2, jsonObj));

  ZDvidAnnotation::AddProperty(jsonObj, "checked", true);
  ASSERT_TRUE(ZDvidAnnotation::HasSameSubProp(jsonObj, jsonObj2));
  ASSERT_TRUE(ZDvidAnnotation::HasSameSubProp(jsonObj2, jsonObj));

  ZDvidAnnotation::UpdateTime(jsonObj, jsonObj2);
  ASSERT_TRUE(ZDvidAnnotation::HasSameSubProp(jsonObj, jsonObj2));
  ASSERT_TRUE(ZDvidAnnotation::HasSameSubProp(jsonObj2, jsonObj));

  propJson = jsonObj.value("Prop");
  ASSERT_TRUE(ZJsonObjectParser::GetValue(
                propJson, ZDvidAnnotation::KEY_CREATED_TIME, "").empty());
  ASSERT_TRUE(ZJsonObjectParser::GetValue(
                propJson, ZDvidAnnotation::KEY_MODIFIED_TIME, "").empty());
  ASSERT_FALSE(ZJsonObjectParser::GetValue(
                 propJson, ZDvidAnnotation::KEY_CHECKED_TIME, "").empty());

  jsonObj2.clear();
  ZDvidAnnotation::UpdateTime(jsonObj, jsonObj2);
  ASSERT_FALSE(ZJsonObjectParser::GetValue(
                propJson, ZDvidAnnotation::KEY_CREATED_TIME, "").empty());
  ASSERT_TRUE(ZJsonObjectParser::GetValue(
                propJson, ZDvidAnnotation::KEY_MODIFIED_TIME, "").empty());
  ASSERT_FALSE(ZJsonObjectParser::GetValue(
                 propJson, ZDvidAnnotation::KEY_CHECKED_TIME, "").empty());


  ZDvidAnnotation::AddProperty(jsonObj, "checked", false);
  ZDvidAnnotation::UpdateTime(jsonObj, jsonObj2);
  ASSERT_FALSE(ZJsonObjectParser::GetValue(
                propJson, ZDvidAnnotation::KEY_CREATED_TIME, "").empty());
  ASSERT_TRUE(ZJsonObjectParser::GetValue(
                propJson, ZDvidAnnotation::KEY_MODIFIED_TIME, "").empty());
  ASSERT_TRUE(ZJsonObjectParser::GetValue(
                 propJson, ZDvidAnnotation::KEY_CHECKED_TIME, "").empty());


  jsonObj.decode("{\"Pos\":[650,875,1023],\"Kind\":\"Note\","
      "\"Tags\":[\"user:zhaot\"],\"Prop\":{\"body ID\":\"1536878688\","
      "\"checked\":\"1\",\"comment\":\"\",\"custom\":\"1\",\"status\":\"\","
      "\"time\":\"\",\"type\":\"Other\",\"user\":\"zhaot\"},\"Rels\":null}", false);
  propJson = jsonObj.value("Prop");
  ASSERT_TRUE(ZJsonObjectParser::GetValue(
                propJson, ZDvidAnnotation::KEY_CREATED_TIME, "").empty());
  jsonObj2 = jsonObj.clone();
  ZDvidAnnotation::UpdateTime(jsonObj, jsonObj2);
  ASSERT_TRUE(ZJsonObjectParser::GetValue(
                propJson, ZDvidAnnotation::KEY_CREATED_TIME, "").empty());
  ASSERT_TRUE(ZJsonObjectParser::GetValue(
                propJson, ZDvidAnnotation::KEY_MODIFIED_TIME, "").empty());
  ASSERT_TRUE(ZJsonObjectParser::GetValue(
                 propJson, ZDvidAnnotation::KEY_CHECKED_TIME, "").empty());

  ZDvidAnnotation::RemoveProperty(jsonObj2, "checked");
  ZDvidAnnotation::UpdateTime(jsonObj, jsonObj2);
  ASSERT_FALSE(ZJsonObjectParser::GetValue(
                 propJson, ZDvidAnnotation::KEY_CHECKED_TIME, "").empty());
  ASSERT_TRUE(ZJsonObjectParser::GetValue(
                propJson, ZDvidAnnotation::KEY_MODIFIED_TIME, "").empty());

  ZDvidAnnotation::RemoveProperty(jsonObj, "checked");
  ZDvidAnnotation::UpdateTime(jsonObj, jsonObj2);
  ASSERT_TRUE(ZJsonObjectParser::GetValue(
                propJson, ZDvidAnnotation::KEY_MODIFIED_TIME, "").empty());
  ASSERT_TRUE(ZJsonObjectParser::GetValue(
                 propJson, ZDvidAnnotation::KEY_CHECKED_TIME, "").empty());

  ZDvidAnnotation::RemoveProperty(jsonObj, "comment");
  ZDvidAnnotation::UpdateTime(jsonObj, jsonObj2);
  ASSERT_TRUE(ZJsonObjectParser::GetValue(
                propJson, ZDvidAnnotation::KEY_MODIFIED_TIME, "").empty());
  ASSERT_TRUE(ZJsonObjectParser::GetValue(
                 propJson, ZDvidAnnotation::KEY_CHECKED_TIME, "").empty());

  ZDvidAnnotation::AddProperty(jsonObj, "comment", "test");
  ZDvidAnnotation::UpdateTime(jsonObj, jsonObj2);
  ASSERT_FALSE(ZJsonObjectParser::GetValue(
                propJson, ZDvidAnnotation::KEY_MODIFIED_TIME, "").empty());
  ASSERT_TRUE(ZJsonObjectParser::GetValue(
                 propJson, ZDvidAnnotation::KEY_CHECKED_TIME, "").empty());

  jsonObj2 = jsonObj.clone();

  ZDvidAnnotation::AddProperty(
        jsonObj, ZDvidAnnotation::KEY_CREATED_TIME, "created time");
  ZDvidAnnotation::AddProperty(
        jsonObj2, ZDvidAnnotation::KEY_CREATED_TIME, "old created time");
  ZDvidAnnotation::UpdateTime(jsonObj, jsonObj2);
  ASSERT_EQ("created time", ZJsonObjectParser::GetValue(
              propJson, ZDvidAnnotation::KEY_CREATED_TIME, ""));

  ZDvidAnnotation::RemoveProperty(jsonObj, ZDvidAnnotation::KEY_CREATED_TIME);
  ZDvidAnnotation::UpdateTime(jsonObj, jsonObj2);
  ASSERT_EQ("old created time", ZJsonObjectParser::GetValue(
              propJson, ZDvidAnnotation::KEY_CREATED_TIME, ""));


  ZDvidAnnotation::AddProperty(
        jsonObj, ZDvidAnnotation::KEY_MODIFIED_TIME, "modified time");
  ZDvidAnnotation::AddProperty(
        jsonObj2, ZDvidAnnotation::KEY_MODIFIED_TIME, "old modified time");
  ZDvidAnnotation::UpdateTime(jsonObj, jsonObj2);
  ASSERT_EQ("modified time", ZJsonObjectParser::GetValue(
              propJson, ZDvidAnnotation::KEY_MODIFIED_TIME, ""));
  ZDvidAnnotation::RemoveProperty(jsonObj, ZDvidAnnotation::KEY_MODIFIED_TIME);
  ZDvidAnnotation::UpdateTime(jsonObj, jsonObj2);
  ASSERT_EQ("old modified time", ZJsonObjectParser::GetValue(
              propJson, ZDvidAnnotation::KEY_MODIFIED_TIME, ""));

  ZDvidAnnotation::AddProperty(jsonObj, "checked", true);
  ZDvidAnnotation::AddProperty(jsonObj2, "checked", true);
  ZDvidAnnotation::AddProperty(
        jsonObj, ZDvidAnnotation::KEY_CHECKED_TIME, "checked time");
  ZDvidAnnotation::AddProperty(
        jsonObj2, ZDvidAnnotation::KEY_CHECKED_TIME, "old checked time");
  ZDvidAnnotation::UpdateTime(jsonObj, jsonObj2);
  ASSERT_EQ("checked time", ZJsonObjectParser::GetValue(
              propJson, ZDvidAnnotation::KEY_CHECKED_TIME, ""));
  ZDvidAnnotation::RemoveProperty(jsonObj, ZDvidAnnotation::KEY_CHECKED_TIME);
  ZDvidAnnotation::UpdateTime(jsonObj, jsonObj2);
  ASSERT_EQ("old checked time", ZJsonObjectParser::GetValue(
              propJson, ZDvidAnnotation::KEY_CHECKED_TIME, ""));
}

TEST(ZDvidAnnotation, ZFlyEmToDoItem)
{
  {
    ZFlyEmToDoItem item;
    //  item.toJsonObject().print();

    item.setChecked(true);
    item.setAction(neutu::EToDoAction::TO_MERGE);
    //  item.toJsonObject().print();
    ASSERT_EQ(neutu::EToDoAction::TO_MERGE, item.getAction());

    ZFlyEmToDoItem item2(item);
    ASSERT_EQ(neutu::EToDoAction::TO_MERGE, item2.getAction());

    std::string mergeTag = std::string(ZFlyEmToDoItem::KEY_ACTION) + ":"
        + ZFlyEmToDoItem::ACTION_MERGE_TAG;
    ASSERT_FALSE(item.hasTag(mergeTag));

    item.setChecked(false);
    ASSERT_TRUE(item.hasTag(mergeTag));
    ASSERT_FALSE(item.isChecked());

    item.setChecked(true);
    ASSERT_FALSE(item.hasTag(mergeTag));
    ASSERT_TRUE(item.isChecked());

    item.setAction(neutu::EToDoAction::TO_SPLIT);
    ASSERT_EQ(neutu::EToDoAction::TO_SPLIT, item.getAction());

    item.setAction(neutu::EToDoAction::TO_DO);
    //  item.toJsonObject().print();
    ASSERT_EQ(neutu::EToDoAction::TO_DO, item.getAction());

    std::string splitTag = std::string(ZFlyEmToDoItem::KEY_ACTION) + ":"
        + ZFlyEmToDoItem::ACTION_SPLIT_TAG;
    ASSERT_FALSE(item.hasTag(splitTag));

    item.addTag(splitTag);
    ASSERT_TRUE(item.hasTag(splitTag));

    item.removeActionTag();
    ASSERT_FALSE(item.hasTag(splitTag));

    item.setAction(neutu::EToDoAction::TO_SPLIT);
    ASSERT_FALSE(item.hasTag(splitTag));

    item.setChecked(false);
    ASSERT_TRUE(item.hasTag(splitTag));

    item.setChecked(true);
    ASSERT_FALSE(item.hasTag(splitTag));

    item.setChecked(false);
    ASSERT_TRUE(item.hasTag(splitTag));

    item.setAction(neutu::EToDoAction::TO_TRACE_TO_SOMA);
    ASSERT_EQ(ZFlyEmToDoItem::ACTION_TRACE_TO_SOMA, item.getActionName());
    ASSERT_TRUE(item.hasTag(
                std::string(ZFlyEmToDoItem::KEY_ACTION) + ":"
                        + ZFlyEmToDoItem::ACTION_TRACE_TO_SOMA_TAG));
  }

  {
    ZFlyEmToDoItem item;
    item.setAction(neutu::EToDoAction::TO_MERGE);

    ASSERT_TRUE(item.hasTag(ZFlyEmToDoItem::GetActionTag(neutu::EToDoAction::TO_MERGE)));

    item.setAction(neutu::EToDoAction::TO_SPLIT);
    std::cout << item.getTagSet().size() << std::endl;
    [](const std::set<std::string> tagSet) {
      std::for_each(tagSet.begin(), tagSet.end(), [](const std::string &tag) {
        std::cout << tag << std::endl;
      });
    } (item.getTagSet());

    ASSERT_FALSE(item.hasTag(ZFlyEmToDoItem::GetActionTag(neutu::EToDoAction::TO_MERGE)));
    ASSERT_TRUE(item.hasTag(ZFlyEmToDoItem::GetActionTag(neutu::EToDoAction::TO_SPLIT)));

    item.setChecked(true);
    ASSERT_TRUE(item.getTagSet().empty());

    item.setChecked(false);
    ASSERT_TRUE(item.hasTag(ZFlyEmToDoItem::GetActionTag(neutu::EToDoAction::TO_SPLIT)));
  }

  {
    ZFlyEmToDoItem item(1, 2, 3);
    ASSERT_EQ(ZIntPoint(1, 2, 3), item.getPosition());

    item.setPriority(0);
    ASSERT_EQ("Unknown", item.getPriorityName());

    item.setPriority(2);
    ASSERT_EQ("High", item.getPriorityName());

    item.setPriority(4);
    ASSERT_EQ("Medium", item.getPriorityName());

    item.setPriority(6);
    ASSERT_EQ("Low", item.getPriorityName());
  }

}

TEST(ZDvidAnnotation, Radius)
{
  ZResolution resolution;
  resolution.setUnit(ZResolution::EUnit::UNIT_PIXEL);
  resolution.setVoxelSize(1.0, 1.0, 1.0);
  ASSERT_DOUBLE_EQ(
        ZDvidAnnotation::GetDefaultRadius(ZDvidAnnotation::EKind::KIND_POST_SYN),
        ZDvidAnnotation::GetDefaultRadius(
          ZDvidAnnotation::EKind::KIND_POST_SYN, resolution));
}

TEST(ZDvidSynapse, Basic)
{
  ZDvidSynapse synapse;
  synapse.setBodyId(1);
  synapse.addPartner(1, 2, 3);
  synapse.addPartner(4, 5, 6);

  std::unordered_map<ZIntPoint, uint64_t> labelMap;
  labelMap[ZIntPoint(1, 2, 3)] = 10;
  labelMap[ZIntPoint(4, 5, 6)] = 20;

  ASSERT_EQ("1-[10][20]", synapse.getConnString(labelMap));

  synapse.setKind(ZDvidAnnotation::EKind::KIND_PRE_SYN);
  ASSERT_EQ("1->[10][20]", synapse.getConnString(labelMap));

  synapse.setKind(ZDvidAnnotation::EKind::KIND_POST_SYN);
  ASSERT_EQ("1<-[10][20]", synapse.getConnString(labelMap));

  ASSERT_FALSE(synapse.hasConfidenceProperty());

  synapse.setConfidence(1.0);
  ASSERT_TRUE(synapse.hasConfidenceProperty());

  synapse.setConfidence(-1.0);
  ASSERT_DOUBLE_EQ(-1.0, synapse.getConfidence());

  synapse.setConfidence(0.5);
  ASSERT_DOUBLE_EQ(0.5, synapse.getConfidence());

  synapse.removeConfidenceProperty();
  ASSERT_FALSE(synapse.hasConfidenceProperty());
  ASSERT_DOUBLE_EQ(1.0, synapse.getConfidence());

  synapse.setConfidence("0.6");
  ASSERT_DOUBLE_EQ(0.6, synapse.getConfidence());
  ASSERT_EQ("0.6", synapse.getConfidenceStr());

}

#endif

#endif // ZDVIDANNOTATIONTEST_H

#ifndef ZDVIDANNOTATIONTEST_H
#define ZDVIDANNOTATIONTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "dvid/zdvidannotation.h"
#include "zjsonobject.h"
#include "zjsonarray.h"
#include "flyem/zflyemtodoitem.h"
#include "dvid/zdvidsynapse.h"

#ifdef _USE_GTEST_

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

  ASSERT_EQ("1", ZJsonParser::stringValue(propJson.value("test").getData()));

  ZDvidAnnotation::AddProperty(jsonObj, "test2", "true");
  ASSERT_EQ("true", ZJsonParser::stringValue(propJson.value("test2").getData()));

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

  ASSERT_EQ("1-10,20,", synapse.getConnString(labelMap));

  synapse.setKind(ZDvidAnnotation::EKind::KIND_PRE_SYN);
  ASSERT_EQ("1->10,20,", synapse.getConnString(labelMap));

  synapse.setKind(ZDvidAnnotation::EKind::KIND_POST_SYN);
  ASSERT_EQ("1<-10,20,", synapse.getConnString(labelMap));

}

#endif

#endif // ZDVIDANNOTATIONTEST_H

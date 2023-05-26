#ifndef ZFLYEMBODYANNOTATIONTEST_H
#define ZFLYEMBODYANNOTATIONTEST_H

#include "ztestheader.h"
#include "flyem/zflyembodyannotation.h"
#include "zjsonobject.h"
#include "flyem/zflyembodyannotationprotocol.h"
#include "neutubeconfig.h"
#include "zjsonobjectparser.h"
#include "flyem/zflyemproofutil.h"

#ifdef _USE_GTEST_

TEST(ZFlyEmBodyAnnotation, Basic)
{
  ZFlyEmBodyAnnotation annot;

  ASSERT_EQ(int64_t(0), annot.getTimestamp());

  annot.updateTimestamp();
  ASSERT_LT(int64_t(0), annot.getTimestamp());

  ZJsonObject json;
  json.setEntry("status", "Hard to trace");
  json.setEntry("comment", "test");
  json.setEntry("body ID", 123);
  json.setEntry("name", "KC");
  json.setEntry("class", "neuron");
  json.setEntry("user", "test_user");
  json.setEntry("naming user", "mock");
  json.setEntry("clonal unit", "clonal unit test");
  json.setEntry("auto-type", "auto type test");
  json.setEntry("property", "Distinct");
  json.setEntry(ZFlyEmBodyAnnotation::KEY_PRIMARY_NEURITE, "neurite test");
  json.setEntry(ZFlyEmBodyAnnotation::KEY_SYNONYM, "note test");

  annot.loadJsonObject(json);
  ASSERT_EQ(int64_t(0), annot.getTimestamp());

  ASSERT_EQ("Hard to trace", annot.getStatus());
  ASSERT_EQ("test", annot.getComment());
  ASSERT_EQ("KC", annot.getName());
  ASSERT_EQ("neuron", annot.getClass());
  ASSERT_EQ("test_user", annot.getUser());
  ASSERT_EQ("clonal unit test", annot.getClonalUnit());
  ASSERT_EQ("auto type test", annot.getAutoType());
  ASSERT_EQ("Distinct", annot.getProperty());
  ASSERT_EQ("neurite test", annot.getPrimaryNeurite());
  ASSERT_EQ("note test", annot.getSynonym());

  annot.updateTimestamp();
  const int64_t t = annot.getTimestamp();
  ASSERT_LT(int64_t(0), t);

  ZJsonObject json2 = annot.toJsonObject();
  ZJsonObjectParser parser;
  ASSERT_EQ("clonal unit test", parser.GetValue(json2, "clonal unit", ""));
  ASSERT_EQ("auto type test", parser.GetValue(json2, "auto-type", ""));
  ASSERT_EQ("neurite test", parser.GetValue(
              json2, ZFlyEmBodyAnnotation::KEY_CELL_BODY_FIBER, ""));
  ASSERT_EQ("note test", parser.GetValue(
              json2, ZFlyEmBodyAnnotation::KEY_NOTES, ""));

  annot.clear();
  ASSERT_EQ(int64_t(0), annot.getTimestamp());
  ASSERT_EQ("", annot.getStatus());
  ASSERT_EQ("", annot.getComment());
  ASSERT_EQ("", annot.getName());
  ASSERT_EQ("", annot.getClass());
  ASSERT_EQ("", annot.getUser());
  ASSERT_EQ("", annot.getProperty());
  ASSERT_EQ("", annot.getPrimaryNeurite());
  ASSERT_EQ("", annot.getSynonym());

  annot.loadJsonObject(json2);
  ASSERT_EQ(t, annot.getTimestamp());

  ZJsonObject json3;
  json3.setEntry(ZFlyEmBodyAnnotation::KEY_CELL_BODY_FIBER, "neurite test");
  json3.setEntry(ZFlyEmBodyAnnotation::KEY_NOTES, "note test");
  ZFlyEmBodyAnnotation::UpdateTimeStamp(json3);
  annot.loadJsonObject(json3);
  ASSERT_EQ("neurite test", parser.GetValue(
              json2, ZFlyEmBodyAnnotation::KEY_CELL_BODY_FIBER, ""));
  ASSERT_EQ("note test", parser.GetValue(
              json2, ZFlyEmBodyAnnotation::KEY_NOTES, ""));
  ASSERT_LT(int64_t(0), annot.getTimestamp());
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

  annotation2.setStatus("Anchor");
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

TEST(ZFlyEmBodyAnnotation, mergeJson)
{
  ZJsonObject annotation1;
  ZJsonObject annotation2;
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_STATUS, "Orphan");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_STATUS, "");

  annotation1 = ZFlyEmBodyAnnotation::MergeAnnotation(
        annotation1, annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Orphan", ZFlyEmBodyAnnotation::GetStatus(annotation1));

  ZFlyEmBodyAnnotation::SetStatus(annotation1, "Orphan");
  ZFlyEmBodyAnnotation::SetStatus(annotation2, "Orphan hotknife");
  annotation1 = ZFlyEmBodyAnnotation::MergeAnnotation(
        annotation1, annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Orphan hotknife", ZFlyEmBodyAnnotation::GetStatus(annotation1));

  ZFlyEmBodyAnnotation::SetStatus(annotation2, "Leaves");
  annotation1 = ZFlyEmBodyAnnotation::MergeAnnotation(
        annotation1, annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Leaves", ZFlyEmBodyAnnotation::GetStatus(annotation1));

  ZFlyEmBodyAnnotation::SetStatus(annotation2, "Hard to trace");
  annotation1 = ZFlyEmBodyAnnotation::MergeAnnotation(
        annotation1, annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Hard to trace", ZFlyEmBodyAnnotation::GetStatus(annotation1));

  ZFlyEmBodyAnnotation::SetStatus(annotation1, "Not examined");
  annotation1 = ZFlyEmBodyAnnotation::MergeAnnotation(
        annotation1, annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Hard to trace", ZFlyEmBodyAnnotation::GetStatus(annotation1));

  ZFlyEmBodyAnnotation::SetStatus(annotation2, "Partially traced");
  annotation1 = ZFlyEmBodyAnnotation::MergeAnnotation(
        annotation1, annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Partially traced", ZFlyEmBodyAnnotation::GetStatus(annotation1));

  ZFlyEmBodyAnnotation::SetStatus(annotation1, "Prelim Roughly traced");
  ZFlyEmBodyAnnotation::SetStatus(annotation2, "Prelim Roughly traced");
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_LOCATION, "test location");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_LOCATION, "test location 2");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_HEMILINEAGE, "hemilineage 2");
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_HEMILINEAGE, "");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_OUT_OF_BOUNDS, true);
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_INSTANCE, "name 2");
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_TIMESTAMP, int64_t(10));
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_TIMESTAMP, int64_t(20));
  ZJsonObject merged = ZFlyEmBodyAnnotation::MergeAnnotation(
        annotation1, annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("test location", ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_LOCATION, ""));
  ASSERT_EQ("hemilineage 2", ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_HEMILINEAGE, ""));
  ASSERT_TRUE(ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_OUT_OF_BOUNDS, false));
  ASSERT_EQ("name 2", ZFlyEmBodyAnnotation::GetName(merged));
  ASSERT_EQ(20, ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_TIMESTAMP, 0));

  ZFlyEmBodyAnnotation::SetStatus(annotation2, "Roughly traced");
  merged = ZFlyEmBodyAnnotation::MergeAnnotation(
          annotation1, annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("test location 2", ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_LOCATION, ""));
  ASSERT_EQ("hemilineage 2", ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_HEMILINEAGE, ""));
  ASSERT_TRUE(ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_OUT_OF_BOUNDS, false));
  ASSERT_EQ("name 2", ZFlyEmBodyAnnotation::GetName(merged));
  ASSERT_EQ(20, ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_TIMESTAMP, 0));


  annotation1.clear();
  annotation2.clear();
  ZFlyEmBodyAnnotation::SetStatus(annotation1, "Traced");
  ZFlyEmBodyAnnotation::SetStatus(annotation2, "Roughly traced");
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_LOCATION, "test location");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_LOCATION, "test location 2");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_HEMILINEAGE, "hemilineage 2");
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_HEMILINEAGE, "");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_OUT_OF_BOUNDS, true);
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_INSTANCE, "name 2");
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_TIMESTAMP, int64_t(10));
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_TIMESTAMP, int64_t(20));
  merged = ZFlyEmBodyAnnotation::MergeAnnotation(
          annotation1, annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("test location", ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_LOCATION, ""));
  ASSERT_EQ("", ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_HEMILINEAGE, ""));
  ASSERT_FALSE(ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_OUT_OF_BOUNDS, false));
  ASSERT_EQ("", ZFlyEmBodyAnnotation::GetName(merged));
  ASSERT_EQ(10, ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_TIMESTAMP, 0));

  annotation1.clear();
  annotation2.clear();
  ZFlyEmBodyAnnotation::SetStatus(annotation1, "Prelim Roughly traced");
  ZFlyEmBodyAnnotation::SetStatus(annotation2, "Prelim Roughly traced");
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_INSTANCE, "name 1");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_INSTANCE, "name 2");
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_TIMESTAMP, int64_t(20));
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_TIMESTAMP, int64_t(10));
  merged = ZFlyEmBodyAnnotation::MergeAnnotation(
          annotation1, annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("name 1", ZFlyEmBodyAnnotation::GetName(merged));
  ASSERT_EQ(20, ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_TIMESTAMP, 0));

  annotation1.clear();
  annotation2.clear();
  ZFlyEmBodyAnnotation::SetStatus(annotation1, "Prelim Roughly traced");
  ZFlyEmBodyAnnotation::SetStatus(annotation2, "Prelim Roughly traced");
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_INSTANCE, "name 1");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_INSTANCE, "name 2");
  merged = ZFlyEmBodyAnnotation::MergeAnnotation(
          annotation1, annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("name 1", ZFlyEmBodyAnnotation::GetName(merged));

  annotation1.clear();
  annotation2.clear();
  ZFlyEmBodyAnnotation::SetStatus(annotation1, "Prelim Roughly traced");
  ZFlyEmBodyAnnotation::SetStatus(annotation2, "Prelim Roughly traced");
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_INSTANCE, "name 1");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_INSTANCE, "name 2");
  merged = ZFlyEmBodyAnnotation::MergeAnnotation(
          annotation1, annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("name 1", ZFlyEmBodyAnnotation::GetName(merged));
}

TEST(ZFlyEmBodyAnnotation, Merger)
{
  ZFlyEmBodyAnnotationProtocol annotMerger;
  ZJsonObject jsonObj;
  jsonObj.load(GET_BENCHMARK_DIR + "/body_status.json");
  annotMerger.loadJsonObject(jsonObj);

  ASSERT_EQ(9999, annotMerger.getStatusRank(""));
  ASSERT_EQ(999, annotMerger.getStatusRank("test"));
  ASSERT_EQ(0, annotMerger.getStatusRank("Finalized"));
  ASSERT_EQ(10, annotMerger.getStatusRank("Traced"));
  ASSERT_EQ(20, annotMerger.getStatusRank("Traced in roi"));
  ASSERT_EQ(30, annotMerger.getStatusRank("roughly Traced"));
  ASSERT_EQ(40, annotMerger.getStatusRank("Prelim Roughly traced"));
  ASSERT_EQ(120, annotMerger.getStatusRank("ORPHAN"));

  ASSERT_TRUE(annotMerger.isFinal("Finalized"));
  ASSERT_FALSE(annotMerger.isFinal("Hard to trace"));

}

TEST(ZFlyEmBodyAnnotation, Json)
{
  ZJsonObject obj;
  obj.setEntry("instance", "test");
  obj.setEntry("status", "Traced");
  obj.setEntry("class", "test type");
  ASSERT_EQ("test", ZFlyEmBodyAnnotation::GetName(obj));
  ASSERT_EQ("Traced", ZFlyEmBodyAnnotation::GetStatus(obj));
  ASSERT_EQ("test type", ZFlyEmBodyAnnotation::GetClass(obj));

  ZFlyEmBodyAnnotation::SetUser(obj, "test_user");
  ASSERT_EQ("test_user", ZJsonObjectParser::GetValue(obj, "user", ""));
  ASSERT_EQ("test_user", ZFlyEmBodyAnnotation::GetUser(obj));

  ZJsonObject oldObj;
  oldObj.setEntry(ZFlyEmBodyAnnotation::KEY_INSTANCE, "old instance");
  ASSERT_EQ("old instance", ZFlyEmBodyAnnotation::GetName(oldObj));

  ASSERT_EQ("test_user", ZFlyEmBodyAnnotation::GetUser(obj));
  obj.setEntry(ZFlyEmBodyAnnotation::KEY_INSTANCE, "old instance");
  ZFlyEmBodyAnnotation::UpdateUserFields(obj, "test_user4", oldObj);
  ASSERT_EQ("test_user", ZFlyEmBodyAnnotation::GetUser(obj));

  ZFlyEmBodyAnnotation::SetUser(oldObj, "");
  ZFlyEmBodyAnnotation::SetUser(obj, "");
  ZFlyEmBodyAnnotation::UpdateUserFields(obj, "test_user5", oldObj);
  ASSERT_EQ("test_user5", ZFlyEmBodyAnnotation::GetUser(obj));

  ZFlyEmBodyAnnotation::UpdateUserFields(obj, "test_user6", oldObj);
  ASSERT_EQ("test_user5", ZFlyEmBodyAnnotation::GetUser(obj));

  {
    ZJsonObject obj1;
    ZJsonObject obj2;
    ASSERT_TRUE(ZFlyEmBodyAnnotation::IsSameAnnotation(obj1, obj2));
    obj1.denull();
    ASSERT_TRUE(ZFlyEmBodyAnnotation::IsSameAnnotation(obj1, obj2));
    ASSERT_TRUE(ZFlyEmBodyAnnotation::IsSameAnnotation(obj2, obj1));
    obj1.setEntry("bodyid", 1);
    ASSERT_FALSE(ZFlyEmBodyAnnotation::IsSameAnnotation(obj1, obj2));
    obj2.denull();
    ASSERT_FALSE(ZFlyEmBodyAnnotation::IsSameAnnotation(obj1, obj2));
    obj2.setEntry("bodyid", 1);
    ASSERT_TRUE(ZFlyEmBodyAnnotation::IsSameAnnotation(obj1, obj2));
    obj2.setEntry("status", true);
    ASSERT_FALSE(ZFlyEmBodyAnnotation::IsSameAnnotation(obj1, obj2));
    obj1.setEntry("status", true);
    ASSERT_TRUE(ZFlyEmBodyAnnotation::IsSameAnnotation(obj1, obj2));
    obj1.clear();
    obj1.setEntry("status", true);
    obj1.setEntry("bodyid", 1);
    ASSERT_TRUE(ZFlyEmBodyAnnotation::IsSameAnnotation(obj1, obj2))
        << obj1.dumpJanssonString(JSON_INDENT(0)) << obj2.dumpJanssonString(JSON_INDENT(0));

    obj1.clear();
    obj2.clear();
    obj1.setEntry("status", "Hard to trace");
    obj1.setEntry("comment", "test");
    obj1.setEntry("body ID", 123);
    obj1.setEntry("name", "KC");
    obj1.setEntry("class", "neuron");
    obj1.setEntry("user", "test_user");
    obj1.setEntry("naming user", "mock");
    obj1.setEntry("clonal unit", "clonal unit test");
    obj1.setEntry("auto-type", "auto type test");
    obj1.setEntry("property", "Distinct");
    obj1.setEntry(ZFlyEmBodyAnnotation::KEY_PRIMARY_NEURITE, "neurite test");
    obj1.setEntry(ZFlyEmBodyAnnotation::KEY_SYNONYM, "note test");

    obj2.setEntry("status", "Hard to trace");
    obj2.setEntry("comment", "test");
    obj2.setEntry("auto-type", "auto type test");
    obj2.setEntry("property", "Distinct");
    obj2.setEntry(ZFlyEmBodyAnnotation::KEY_PRIMARY_NEURITE, "neurite test");
    obj2.setEntry("naming user", "mock");
    obj2.setEntry("clonal unit", "clonal unit test");
    obj2.setEntry(ZFlyEmBodyAnnotation::KEY_SYNONYM, "note test");
    obj2.setEntry("body ID", 123);
    obj2.setEntry("name", "KC");
    obj2.setEntry("class", "neuron");
    ASSERT_FALSE(ZFlyEmBodyAnnotation::IsSameAnnotation(obj1, obj2));
    obj2.setEntry("user", "test_user2");
    ASSERT_FALSE(ZFlyEmBodyAnnotation::IsSameAnnotation(obj1, obj2));

    obj2.setEntry("user", "test_user");
    ASSERT_TRUE(ZFlyEmBodyAnnotation::IsSameAnnotation(obj1, obj2));

    obj1.clear();
    ASSERT_TRUE(ZFlyEmBodyAnnotation::IsEmptyAnnotation(obj1));

    obj1.setEntry("test", ZJsonValue::MakeNull());
    ASSERT_TRUE(ZFlyEmBodyAnnotation::IsEmptyAnnotation(obj1));

    obj1.setEntry(ZFlyEmBodyAnnotation::KEY_BODY_ID, 1);
    ASSERT_TRUE(ZFlyEmBodyAnnotation::IsEmptyAnnotation(obj1));

    obj1.setEntry("test", 1);
    ASSERT_FALSE(ZFlyEmBodyAnnotation::IsEmptyAnnotation(obj1));

  }
}

#endif

#endif // ZFLYEMBODYANNOTATIONTEST_H

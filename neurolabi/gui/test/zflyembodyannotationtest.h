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
//  ASSERT_EQ(uint64_t(0), annot.getBodyId());

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
  json.setEntry(ZFlyEmBodyAnnotation::KEY_STATUS_USER, "test_user2");

  annot.loadJsonObject(json);
  ASSERT_EQ(int64_t(0), annot.getTimestamp());

//  ASSERT_EQ(uint64_t(123), annot.getBodyId());
  ASSERT_EQ("Hard to trace", annot.getStatus());
  ASSERT_EQ("test", annot.getComment());
  ASSERT_EQ("KC", annot.getName());
  ASSERT_EQ("neuron", annot.getClass());
  ASSERT_EQ("test_user", annot.getUser());
  ASSERT_EQ("test_user2", annot.getStatusUser());
  ASSERT_EQ("mock", annot.getNamingUser());
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
//  ASSERT_EQ(uint64_t(0), annot.getBodyId());
  ASSERT_EQ("", annot.getStatus());
  ASSERT_EQ("", annot.getComment());
  ASSERT_EQ("", annot.getName());
  ASSERT_EQ("", annot.getClass());
  ASSERT_EQ("", annot.getUser());
  ASSERT_EQ("", annot.getNamingUser());
  ASSERT_EQ("", annot.getProperty());
  ASSERT_EQ("", annot.getPrimaryNeurite());
  ASSERT_EQ("", annot.getSynonym());
  ASSERT_EQ("", annot.getStatusUser());

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
//  annotation1.setBodyId(1);
  ZFlyEmBodyAnnotation annotation2;
//  annotation2.setBodyId(2);
  annotation1.setStatus("Orphan");
  annotation2.setStatus("");

  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Orphan", annotation1.getStatus());
//  ASSERT_EQ(uint64_t(1), annotation1.getBodyId());

  annotation1.setStatus("Orphan");

  annotation2.setStatus("Orphan hotknife");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Orphan hotknife", annotation1.getStatus());
//  ASSERT_EQ(uint64_t(1), annotation1.getBodyId());

  annotation2.setStatus("Leaves");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Leaves", annotation1.getStatus());
//  ASSERT_EQ(uint64_t(1), annotation1.getBodyId());

  annotation2.setStatus("Hard to trace");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Hard to trace", annotation1.getStatus());

  annotation1.setStatus("Not examined");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Hard to trace", annotation1.getStatus());
//  ASSERT_EQ(uint64_t(1), annotation1.getBodyId());

  annotation2.setStatus("Partially traced");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Partially traced", annotation1.getStatus());
//  ASSERT_EQ(uint64_t(1), annotation1.getBodyId());

  annotation2.setStatus("Prelim Roughly traced");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Prelim Roughly traced", annotation1.getStatus());

  annotation2.setStatus("Roughly traced");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Roughly traced", annotation1.getStatus());
//  ASSERT_EQ(uint64_t(1), annotation1.getBodyId());

  annotation2.setStatus("Anchor");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Roughly traced", annotation1.getStatus());

  annotation2.setStatus("Traced in ROI");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Traced in ROI", annotation1.getStatus());
//  ASSERT_EQ(uint64_t(1), annotation1.getBodyId());

  annotation2.setStatus("traced");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("traced", annotation1.getStatus());

  annotation2.setStatus("Finalized");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Finalized", annotation1.getStatus());
//  ASSERT_EQ(uint64_t(1), annotation1.getBodyId());
}

TEST(ZFlyEmBodyAnnotation, mergeJson)
{
  ZJsonObject annotation1;
//  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_BODY_ID, 1);
  ZJsonObject annotation2;
//  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_BODY_ID, 2);
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_STATUS, "Orphan");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_STATUS, "");

  annotation1 = ZFlyEmBodyAnnotation::MergeAnnotation(
        annotation1, annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Orphan", ZFlyEmBodyAnnotation::GetStatus(annotation1));
//  ASSERT_EQ(1, int(ZJsonObjectParser::GetValue(
//                     annotation1, ZFlyEmBodyAnnotation::KEY_BODY_ID, 0)));

  ZFlyEmBodyAnnotation::SetStatus(annotation1, "Orphan");
  ZFlyEmBodyAnnotation::SetStatus(annotation2, "Orphan hotknife");
  annotation1 = ZFlyEmBodyAnnotation::MergeAnnotation(
        annotation1, annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Orphan hotknife", ZFlyEmBodyAnnotation::GetStatus(annotation1));
//  ASSERT_EQ(1, int(ZFlyEmBodyAnnotation::GetBodyId(annotation1)));

  ZFlyEmBodyAnnotation::SetStatus(annotation2, "Leaves");
  annotation1 = ZFlyEmBodyAnnotation::MergeAnnotation(
        annotation1, annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Leaves", ZFlyEmBodyAnnotation::GetStatus(annotation1));
//  ASSERT_EQ(uint64_t(1), ZFlyEmBodyAnnotation::GetBodyId(annotation1));

  ZFlyEmBodyAnnotation::SetStatus(annotation2, "Hard to trace");
  annotation1 = ZFlyEmBodyAnnotation::MergeAnnotation(
        annotation1, annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Hard to trace", ZFlyEmBodyAnnotation::GetStatus(annotation1));

  ZFlyEmBodyAnnotation::SetStatus(annotation1, "Not examined");
  annotation1 = ZFlyEmBodyAnnotation::MergeAnnotation(
        annotation1, annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Hard to trace", ZFlyEmBodyAnnotation::GetStatus(annotation1));
//  ASSERT_EQ(uint64_t(1), ZFlyEmBodyAnnotation::GetBodyId(annotation1));

  ZFlyEmBodyAnnotation::SetStatus(annotation2, "Partially traced");
  annotation1 = ZFlyEmBodyAnnotation::MergeAnnotation(
        annotation1, annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Partially traced", ZFlyEmBodyAnnotation::GetStatus(annotation1));
//  ASSERT_EQ(uint64_t(1), ZFlyEmBodyAnnotation::GetBodyId(annotation1));

  ZFlyEmBodyAnnotation::SetStatus(annotation1, "Prelim Roughly traced");
  ZFlyEmBodyAnnotation::SetStatus(annotation2, "Prelim Roughly traced");
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_LOCATION, "test location");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_LOCATION, "test location 2");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_HEMILINEAGE, "hemilineage 2");
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_HEMILINEAGE, "");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_OUT_OF_BOUNDS, true);
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_NAMING_USER_OLD, "user 1");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_INSTANCE, "name 2");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_NAMING_USER, "user 2");
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
  ASSERT_EQ("user 2", ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_NAMING_USER, ""));
  ASSERT_EQ(20, ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_TIMESTAMP, 0));
//  ASSERT_EQ(uint64_t(1), ZFlyEmBodyAnnotation::GetBodyId(annotation1));

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
  ASSERT_EQ("user 2", ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_NAMING_USER, ""));
  ASSERT_EQ(20, ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_TIMESTAMP, 0));
//  ASSERT_EQ(uint64_t(1), ZFlyEmBodyAnnotation::GetBodyId(annotation1));


  annotation1.clear();
  annotation2.clear();
  ZFlyEmBodyAnnotation::SetStatus(annotation1, "Traced");
  ZFlyEmBodyAnnotation::SetStatus(annotation2, "Roughly traced");
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_LOCATION, "test location");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_LOCATION, "test location 2");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_HEMILINEAGE, "hemilineage 2");
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_HEMILINEAGE, "");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_OUT_OF_BOUNDS, true);
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_NAMING_USER, "user 1");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_INSTANCE, "name 2");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_NAMING_USER, "user 2");
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
  ASSERT_EQ("user 1", ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_NAMING_USER, ""));
  ASSERT_EQ(10, ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_TIMESTAMP, 0));

  annotation1.clear();
  annotation2.clear();
  ZFlyEmBodyAnnotation::SetStatus(annotation1, "Prelim Roughly traced");
  ZFlyEmBodyAnnotation::SetStatus(annotation2, "Prelim Roughly traced");
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_INSTANCE, "name 1");
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_NAMING_USER, "user 1");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_INSTANCE, "name 2");
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_NAMING_USER, "user 2");
  annotation1.setEntry(ZFlyEmBodyAnnotation::KEY_TIMESTAMP, int64_t(20));
  annotation2.setEntry(ZFlyEmBodyAnnotation::KEY_TIMESTAMP, int64_t(10));
  merged = ZFlyEmBodyAnnotation::MergeAnnotation(
          annotation1, annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("name 1", ZFlyEmBodyAnnotation::GetName(merged));
  ASSERT_EQ("user 1", ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_NAMING_USER, ""));
  ASSERT_EQ(20, ZJsonObjectParser::GetValue(
              merged, ZFlyEmBodyAnnotation::KEY_TIMESTAMP, 0));



  /*
  annotation2.setStatus("Prelim Roughly traced");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Prelim Roughly traced", annotation1.getStatus());

  annotation2.setStatus("Roughly traced");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Roughly traced", annotation1.getStatus());
  ASSERT_EQ(uint64_t(1), annotation1.getBodyId());

  annotation2.setStatus("Anchor");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Roughly traced", annotation1.getStatus());

  annotation2.setStatus("Traced in ROI");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Traced in ROI", annotation1.getStatus());
  ASSERT_EQ(uint64_t(1), annotation1.getBodyId());

  annotation2.setStatus("traced");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("traced", annotation1.getStatus());

  annotation2.setStatus("Finalized");
  annotation1.mergeAnnotation(annotation2, &ZFlyEmBodyAnnotation::GetStatusRank);
  ASSERT_EQ("Finalized", annotation1.getStatus());
  ASSERT_EQ(uint64_t(1), annotation1.getBodyId());
  */
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

  ZFlyEmBodyAnnotation::SetNamingUser(obj, "test_user2");
  ASSERT_EQ("test_user2", ZJsonObjectParser::GetValue(obj, "instance_user", ""));
  ASSERT_EQ("test_user2", ZFlyEmBodyAnnotation::GetNamingUser(obj));

  obj.setEntry(ZFlyEmBodyAnnotation::KEY_NAMING_USER_OLD, "test_user");
  ASSERT_EQ("test_user2", ZFlyEmBodyAnnotation::GetNamingUser(obj));
  obj.removeKey(ZFlyEmBodyAnnotation::KEY_NAMING_USER);
  ASSERT_EQ("test_user", ZFlyEmBodyAnnotation::GetNamingUser(obj));
  ZFlyEmBodyAnnotation::SetNamingUser(obj, "test_user3");
  ASSERT_EQ("test_user3", ZFlyEmBodyAnnotation::GetNamingUser(obj));
  ASSERT_FALSE(obj.hasKey(ZFlyEmBodyAnnotation::KEY_NAMING_USER_OLD));

  ZJsonObject oldObj;
  oldObj.setEntry(ZFlyEmBodyAnnotation::KEY_INSTANCE, "old instance");
  ASSERT_EQ("old instance", ZFlyEmBodyAnnotation::GetName(oldObj));

  ASSERT_EQ("test_user", ZFlyEmBodyAnnotation::GetUser(obj));
  obj.setEntry(ZFlyEmBodyAnnotation::KEY_INSTANCE, "old instance");
  ZFlyEmBodyAnnotation::UpdateUserFields(obj, "test_user4", oldObj);
  ASSERT_EQ("test_user3", ZFlyEmBodyAnnotation::GetNamingUser(obj));
  ASSERT_EQ("test_user", ZFlyEmBodyAnnotation::GetUser(obj));
  ASSERT_EQ("test_user4", ZFlyEmBodyAnnotation::GetLastModifiedBy(obj));

  ZFlyEmBodyAnnotation::SetUser(oldObj, "");
  ZFlyEmBodyAnnotation::SetUser(obj, "");
  ZFlyEmBodyAnnotation::UpdateUserFields(obj, "test_user5", oldObj);
  ASSERT_EQ("test_user3", ZFlyEmBodyAnnotation::GetNamingUser(obj));
  ASSERT_EQ("test_user5", ZFlyEmBodyAnnotation::GetUser(obj));
  ASSERT_EQ("test_user5", ZFlyEmBodyAnnotation::GetLastModifiedBy(obj));

  ZFlyEmBodyAnnotation::UpdateUserFields(obj, "test_user6", oldObj);
  ASSERT_EQ("test_user3", ZFlyEmBodyAnnotation::GetNamingUser(obj));
  ASSERT_EQ("test_user5", ZFlyEmBodyAnnotation::GetUser(obj));
  ASSERT_EQ("test_user6", ZFlyEmBodyAnnotation::GetLastModifiedBy(obj));
}

#endif

#endif // ZFLYEMBODYANNOTATIONTEST_H

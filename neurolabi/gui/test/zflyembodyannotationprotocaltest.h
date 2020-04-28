#ifndef ZFLYEMBODYANNOTATIONMERGERTEST_H
#define ZFLYEMBODYANNOTATIONMERGERTEST_H

#include "ztestheader.h"
#include "flyem/zflyembodyannotationprotocol.h"
#include "zjsonobject.h"
#include "flyem/zflyembodystatus.h"
#include "flyem/zflyembodyannotation.h"

#ifdef _USE_GTEST_

namespace {
const char *json_data =
    "{"
    "  \"status\": ["
    "    {"
    "      \"protection\": 9,"
    "      \"priority\": 0,"
    "      \"name\": \"Finalized\","
    "      \"preserving_id\": true,"
    "      \"expert\": false,"
    "      \"final\": true"
    "    },"
    "    {"
    "      \"protection\": 5,"
    "      \"priority\": 10,"
    "      \"name\": \"Traced\","
    "      \"expert\": true,"
    "      \"final\": false,"
    "      \"preserving_id\": true,"
    "      \"admin_level\": 1"
    "    },"
    "    {"
    "      \"protection\": 9,"
    "      \"priority\": 20,"
    "      \"name\": \"Traced in ROI\","
    "      \"expert\": false,"
    "      \"final\": false"
    "    },"
    "    {"
    "      \"protection\": 5,"
    "      \"priority\": 30,"
    "      \"name\": \"Roughly traced\","
    "      \"expert\": true,"
    "      \"final\": false,"
    "      \"preserving_id\": true,"
    "      \"admin_level\": 1"
    "    },"
    "    {"
    "      \"protection\": 0,"
    "      \"priority\": 40,"
    "      \"name\": \"Prelim Roughly traced\","
    "      \"expert\": false,"
    "      \"final\": false"
    "    },"
    "    {"
    "      \"protection\": 9,"
    "      \"priority\": 50,"
    "      \"name\": \"Partially traced\","
    "      \"expert\": false,"
    "      \"final\": false"
    "    },"
    "    {"
    "      \"protection\": 0,"
    "      \"priority\": 55,"
    "      \"name\": \"Unimportant\","
    "      \"expert\": false,"
    "      \"final\": false,"
    "      \"mergable\": false"
    "    },"
    "    {"
    "      \"protection\": 9,"
    "      \"priority\": 60,"
    "      \"name\": \"Hard to trace\","
    "      \"expert\": false,"
    "      \"final\": false"
    "    },"
    "    {"
    "      \"protection\": 9,"
    "      \"priority\": 70,"
    "      \"name\": \"Anchor\","
    "      \"expert\": false,"
    "      \"final\": false,"
    "      \"admin_level\": 1"
    "    },"
    "    {"
    "      \"protection\": 0,"
    "      \"priority\": 80,"
    "      \"name\": \"Leaves\","
    "      \"expert\": false,"
    "      \"preserving_id\": true,"
    "      \"final\": true"
    "    },"
    "    {"
    "      \"protection\": 9,"
    "      \"priority\": 90,"
    "      \"name\": \"0.5assign\","
    "      \"expert\": false,"
    "      \"final\": false"
    "    },"
    "    {"
    "      \"protection\": 9,"
    "      \"priority\": 100,"
    "      \"name\": \"Not examined\","
    "      \"expert\": false,"
    "      \"final\": false"
    "    },"
    "    {"
    "      \"protection\": 0,"
    "      \"priority\": 110,"
    "      \"name\": \"Orphan hotknife\","
    "      \"expert\": false,"
    "      \"final\": false"
    "    },"
    "    {"
    "      \"protection\": 0,"
    "      \"priority\": 120,"
    "      \"name\": \"Orphan\","
    "      \"expert\": false,"
    "      \"final\": false"
    "    },"
    "    {"
    "      \"protection\": 0,"
    "      \"priority\": 130,"
    "      \"name\": \"Orphan-artifact\","
    "      \"expert\": false,"
    "      \"final\": false"
    "    }"
    "  ],"
    "  \"conflict\": ["
    "    [\"roughly traced\", \"prelim roughly traced\", \"anchor\", \"traced\"]"
    "  ]"
    "}";
};

TEST(ZFlyEmBodyAnnotationProtocal, Basic)
{
  ZFlyEmBodyAnnotationProtocal protocol;

  ZJsonObject obj;
  obj.decode(json_data, true);
  protocol.loadJsonObject(obj);
  ASSERT_EQ(15, int(protocol.getStatusList().size()));
  ASSERT_FALSE(protocol.isEmpty());
  ASSERT_TRUE(protocol.preservingId("Leaves"));
  ASSERT_FALSE(protocol.preservingId("Orphan"));
  ASSERT_FALSE(protocol.isMergable("Unimportant"));
  ASSERT_TRUE(protocol.isExpertStatus("Roughly traced"));
  ASSERT_FALSE(protocol.isExpertStatus("anchor"));
  ASSERT_TRUE(protocol.isFinal("Leaves"));
  ASSERT_EQ(120, protocol.getStatusRank("Orphan"));
  ASSERT_EQ(9999, protocol.getStatusRank(""));
  ASSERT_EQ("Orphan", protocol.getBodyStatus("Orphan").getName());

  QMap<uint64_t, ZFlyEmBodyAnnotation> annotMap;

  {
    ZFlyEmBodyAnnotation annot;
    annot.setStatus("anchor");
    annotMap[1] = annot;
  }

  {
    ZFlyEmBodyAnnotation annot;
    annot.setStatus("Anchor");
    annotMap[2] = annot;
  }

  {
    ZFlyEmBodyAnnotation annot;
    annot.setStatus("Finalized");
    annotMap[3] = annot;
  }

  {
    ZFlyEmBodyAnnotation annot;
    annot.setStatus("Roughly traced");
    annotMap[4] = annot;
  }

  std::vector<std::vector<uint64_t>> bodySet =
      protocol.getConflictBody(annotMap);
  ASSERT_EQ(1, int(bodySet.size()));

  std::vector<uint64_t> bodyArray = bodySet[0];
  ASSERT_EQ(3, int(bodyArray.size()));

  ASSERT_EQ(1, int(bodyArray[0]));
  ASSERT_EQ(2, int(bodyArray[1]));
  ASSERT_EQ(4, int(bodyArray[2]));

  protocol.reset();
  ASSERT_TRUE(protocol.isEmpty());
}

#endif


#endif // ZFLYEMBODYANNOTATIONMERGERTEST_H

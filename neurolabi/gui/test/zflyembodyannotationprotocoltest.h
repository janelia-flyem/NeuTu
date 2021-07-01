#ifndef ZFLYEMBODYANNOTATIONPROTOCOLTEST_H
#define ZFLYEMBODYANNOTATIONPROTOCOLTEST_H

#include <algorithm>

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "flyem/zflyembodyannotationprotocol.h"

#ifdef _USE_GTEST_

class ZFlyEmBodyAnnotationProtocol_F1 : public ::testing::Test {
public:
  ZFlyEmBodyAnnotationProtocol protocol;

  void SetUp() override {
    ZJsonObject obj;
    obj.load(GET_BENCHMARK_DIR + "/flyem/body_status_protocol.json");
    if (!obj.isEmpty()) {
      protocol.loadJsonObject(obj);
    }
  }
};

TEST_F(ZFlyEmBodyAnnotationProtocol_F1, Basic)
{
  ASSERT_EQ(22, protocol.getStatusList().size());
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
  ASSERT_TRUE(protocol.getColorCode("Orphan").empty());
  ASSERT_EQ("#FF00FF00", protocol.getColorCode("Quarantine"));
}

TEST_F(ZFlyEmBodyAnnotationProtocol_F1, Conflict)
{
  {
    QMap<uint64_t, std::string> statusMap;
    statusMap[1] = "Roughly Traced";
    statusMap[2] = "Traced";
    statusMap[3] = "Orphan";

    auto bodyGroups = protocol.getConflictBody(statusMap);
    ASSERT_EQ(1, int(bodyGroups.size()));
    ASSERT_EQ(2, bodyGroups[0].size());
    ASSERT_NE(bodyGroups[0].end(),
        std::find(bodyGroups[0].begin(), bodyGroups[0].end(), 1));
    ASSERT_NE(bodyGroups[0].end(),
        std::find(bodyGroups[0].begin(), bodyGroups[0].end(), 2));
  }

  {
    QMap<uint64_t, std::string> statusMap;
    statusMap[1] = "Roughly Traced";
    statusMap[2] = "Traced";
    statusMap[3] = "Soma Anchor";
    statusMap[4] = "primary anchor";

    auto bodyGroups = protocol.getConflictBody(statusMap);
    ASSERT_EQ(2, int(bodyGroups.size()));
    ASSERT_NE(bodyGroups[0].end(),
        std::find(bodyGroups[0].begin(), bodyGroups[0].end(), 1));
    ASSERT_NE(bodyGroups[0].end(),
        std::find(bodyGroups[0].begin(), bodyGroups[0].end(), 2));
    ASSERT_NE(bodyGroups[1].end(),
        std::find(bodyGroups[1].begin(), bodyGroups[1].end(), 3));
    ASSERT_NE(bodyGroups[1].end(),
        std::find(bodyGroups[1].begin(), bodyGroups[1].end(), 4));
  }

  {
    QMap<uint64_t, std::string> statusMap;
    statusMap[1] = "Roughly Traced";
    statusMap[2] = "Roughly Traced";

    auto bodyGroups = protocol.getConflictBody(statusMap);
    ASSERT_EQ(1, int(bodyGroups.size()));
    ASSERT_NE(bodyGroups[0].end(),
        std::find(bodyGroups[0].begin(), bodyGroups[0].end(), 1));
    ASSERT_NE(bodyGroups[0].end(),
        std::find(bodyGroups[0].begin(), bodyGroups[0].end(), 2));
  }

  {
    QMap<uint64_t, std::string> statusMap;
    statusMap[1] = "Soma Anchor";
    statusMap[2] = "Cervical Anchor";
    statusMap[3] = "Soma Anchor";
    statusMap[4] = "Prelim Roughly Traced";

    auto bodyGroups = protocol.getExclusionBody(statusMap);
    ASSERT_EQ(1, int(bodyGroups.size()));
    ASSERT_NE(bodyGroups[0].end(),
        std::find(bodyGroups[0].begin(), bodyGroups[0].end(), 1));
    ASSERT_NE(bodyGroups[0].end(),
        std::find(bodyGroups[0].begin(), bodyGroups[0].end(), 3));
    ASSERT_NE(bodyGroups[0].end(),
        std::find(bodyGroups[0].begin(), bodyGroups[0].end(), 4));
  }

  {
    QMap<uint64_t, std::string> statusMap;
    statusMap[1] = "Soma Anchor";
    statusMap[2] = "Cervical Anchor";
    statusMap[3] = "Soma Anchor";
    statusMap[4] = "Primary anchor";
    statusMap[5] = "Prelim Roughly Traced";

    auto bodyGroups = protocol.getExclusionBody(statusMap);
    ASSERT_EQ(2, int(bodyGroups.size()));
    ASSERT_NE(bodyGroups[0].end(),
        std::find(bodyGroups[0].begin(), bodyGroups[0].end(), 1));
    ASSERT_NE(bodyGroups[0].end(),
        std::find(bodyGroups[0].begin(), bodyGroups[0].end(), 3));
    ASSERT_NE(bodyGroups[0].end(),
        std::find(bodyGroups[0].begin(), bodyGroups[0].end(), 5));
    ASSERT_NE(bodyGroups[1].end(),
        std::find(bodyGroups[1].begin(), bodyGroups[1].end(), 2));
    ASSERT_NE(bodyGroups[1].end(),
        std::find(bodyGroups[1].begin(), bodyGroups[1].end(), 4));
  }

  {
    QMap<uint64_t, std::string> statusMap;
    statusMap[1] = "Soma Anchor";
    statusMap[3] = "Soma Anchor";

    auto bodyGroups = protocol.getExclusionBody(statusMap);
    ASSERT_EQ(1, int(bodyGroups.size()));
    ASSERT_NE(bodyGroups[0].end(),
        std::find(bodyGroups[0].begin(), bodyGroups[0].end(), 1));
    ASSERT_NE(bodyGroups[0].end(),
        std::find(bodyGroups[0].begin(), bodyGroups[0].end(), 3));
  }

  {
    QMap<uint64_t, std::string> statusMap;
    statusMap[2] = "Prelim Roughly Traced";
    statusMap[4] = "Prelim Roughly Traced";

    auto bodyGroups = protocol.getExclusionBody(statusMap);
    ASSERT_EQ(0, int(bodyGroups.size()));
  }
//  ZFlyEmBodyAnnotation annot1;
//  annot1.setStatus("Roughly Traced");



}

#endif

#endif // ZFLYEMBODYANNOTATIONPROTOCOLTEST_H

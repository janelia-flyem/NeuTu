#ifndef ZFLYEMBODYSTATUSTEST_H
#define ZFLYEMBODYSTATUSTEST_H

#include "ztestheader.h"

#include "flyem/zflyembodystatus.h"
#include "zjsonparser.h"
#include "zjsonobjectparser.h"

#ifdef _USE_GTEST_

TEST(ZFlyEmBodyStatus, Property)
{
  ZFlyEmBodyStatus status("Test");
  ASSERT_FALSE(status.isExpertStatus());
  ASSERT_FALSE(status.isFinal());
  ASSERT_EQ(999, status.getPriority());
  ASSERT_EQ("Test", status.getName());

  status.setExpert(true);
  status.setFinal(true);
  status.setPriority(1);

  ASSERT_TRUE(status.isExpertStatus());
  ASSERT_TRUE(status.isFinal());
  ASSERT_EQ(1, status.getPriority());
}

TEST(ZFlyEmBodyStatus, Json)
{
  ZFlyEmBodyStatus status;

  ZJsonObject json;
  json.setEntry(ZFlyEmBodyStatus::KEY_EXPERT, true);
  json.setEntry(ZFlyEmBodyStatus::KEY_FINAL, true);
  json.setEntry(ZFlyEmBodyStatus::KEY_PRIORITY, 1);

  status.loadJsonObject(json);
  ASSERT_TRUE(status.isExpertStatus());
  ASSERT_TRUE(status.isFinal());
  ASSERT_EQ(1, status.getPriority());

  status.reset();
  ASSERT_FALSE(status.isExpertStatus());
  ASSERT_FALSE(status.isFinal());
  ASSERT_EQ(999, status.getPriority());
  ASSERT_EQ("", status.getName());
}

#endif


#endif // ZFLYEMBODYSTATUSTEST_H

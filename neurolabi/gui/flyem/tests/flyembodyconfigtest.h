#ifndef FLYEMBODYCONFIGTEST_H
#define FLYEMBODYCONFIGTEST_H

#ifdef _USE_GTEST_

#include "test/ztestheader.h"
#include "flyem/flyembodyconfig.h"

TEST(FlyEmBodyConfig, Basic)
{
  FlyEmBodyConfig config;

  ASSERT_FALSE(config.isHybrid());

  config.setBodyId(1);
  ASSERT_EQ(1, config.getBodyId());

  config.setDsLevel(3);
  ASSERT_EQ(3, config.getDsLevel());

  config.setLocalDsLevel(1);
  ASSERT_EQ(1, config.getLocalDsLevel());

  ASSERT_FALSE(config.usingCoarseLevel());
  config.setCoarseLevel(2);
  ASSERT_TRUE(config.usingCoarseLevel());

  ASSERT_TRUE(config.hasNextDsLevel(0));
  config.decDsLevel();
  ASSERT_EQ(1, config.getDsLevel());

  config.decDsLevel();
  ASSERT_EQ(0, config.getDsLevel());
  ASSERT_FALSE(config.hasNextDsLevel(0));

  ASSERT_EQ(neutu::EBodyLabelType::BODY, config.getLabelType());

  config.setBodyColor(QColor(1, 2, 3));
  ASSERT_EQ(QColor(1, 2, 3), config.getBodyColor());

  config.setRange({1, 2, 3, 4, 5, 6});
  ASSERT_TRUE(config.isHybrid());
}

#endif

#endif // FLYEMBODYCONFIGTEST_H

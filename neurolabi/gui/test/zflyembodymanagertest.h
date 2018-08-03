#ifndef ZFLYEMBODYMANAGERTEST_H
#define ZFLYEMBODYMANAGERTEST_H

#include "ztestheader.h"
#include "flyem/zflyembodymanager.h"
#include "flyem/zflyembodyconfig.h"

#ifdef _USE_GTEST_

TEST(ZFlyEmBodyManager, Basic)
{
  ZFlyEmBodyManager bm;
  ASSERT_TRUE(bm.isEmpty());

  bm.registerBody(1);
  ASSERT_TRUE(bm.contains(1));
  ASSERT_FALSE(bm.hasMapping(1));

  bm.registerBody(2, QSet<uint64_t>({200}));
  ASSERT_TRUE(bm.contains(2));
  ASSERT_TRUE(bm.hasMapping(2));

  QSet<uint64_t> bodySet = bm.getNormalBodySet();
  ASSERT_EQ(2, bodySet.size());
  ASSERT_TRUE(bodySet.contains(1));
  ASSERT_TRUE(bodySet.contains(2));
  ASSERT_FALSE(bodySet.contains(200));

  QSet<uint64_t> mappedSet = bm.getMappedSet(1);
  ASSERT_TRUE(mappedSet.isEmpty());

  mappedSet = bm.getMappedSet(2);
  ASSERT_EQ(1, mappedSet.size());
  ASSERT_TRUE(mappedSet.contains(200));


  bm.deregisterBody(1);
  ASSERT_FALSE(bm.contains(1));

  bm.registerBody(2);
  ASSERT_TRUE(bm.hasMapping(2));

  bm.registerBody(2, QSet<uint64_t>());
  ASSERT_TRUE(bm.contains(2));
  ASSERT_FALSE(bm.hasMapping(2));

  bm.registerBody(2, QSet<uint64_t>({200, 300}));

  ASSERT_EQ(2, (int) bm.getAggloId(300));
  ASSERT_EQ(20, (int) bm.getAggloId(20));

//  bm.print();
  ASSERT_EQ(0, (int) bm.getSingleBodyId());

  bm.eraseSupervoxel(300);
  ASSERT_EQ(300, (int) bm.getAggloId(300));
  ASSERT_EQ(0, (int) bm.getSingleBodyId());
  bm.eraseSupervoxel(200);
  ASSERT_EQ(2, (int)bm.getSingleBodyId());

  bm.registerBody(1);
  ASSERT_FALSE(bm.isTodoLoaded(1));
  ASSERT_FALSE(bm.isSynapseLoaded(1));
  bm.setTodoLoaded(1);
  ASSERT_TRUE(bm.isTodoLoaded(1));
  bm.setSynapseLoaded(1);
  ASSERT_TRUE(bm.isSynapseLoaded(1));

  bm.deregisterBody(1);
  ASSERT_FALSE(bm.isTodoLoaded(1));
  ASSERT_FALSE(bm.isSynapseLoaded(1));

  ZFlyEmBodyConfig config(1);
  bm.addBodyConfig(config);
  ASSERT_EQ(1, (int) bm.getBodyConfig(1).getBodyId());

  bm.deregisterBody(1);
  ASSERT_EQ(0, (int) bm.getBodyConfig(1).getBodyId());

  bm.registerBody(ZFlyEmBodyManager::encode(1, 1, true));
  ASSERT_TRUE(bm.contains(1));
  ASSERT_FALSE(bm.hasMapping(1));

  ZFlyEmBodyConfig config2(ZFlyEmBodyManager::encode(1, 0, true));
  bm.addBodyConfig(config2);
  ASSERT_EQ(ZFlyEmBodyManager::encode(1, 0, true),
            bm.getBodyConfig(1).getBodyId());

  bm.deregisterBody(1);
  ASSERT_EQ(0, (int) bm.getBodyConfig(1).getBodyId());

  bm.clear();
  ASSERT_TRUE(bm.isEmpty());

  bm.registerSupervoxel(1);
  ASSERT_EQ(ZFlyEmBodyManager::encodeSupervoxel(1), bm.getSingleBodyId());
  ASSERT_EQ(uint64_t(9900000000001), bm.getSingleBodyId());
}

TEST(ZFlyEmBodyManager, encode)
{
  ASSERT_EQ(uint64_t(1), ZFlyEmBodyManager::encode(1, 0, false));
  ASSERT_EQ(uint64_t(10100000000003), ZFlyEmBodyManager::encode(3, 1));
  ASSERT_EQ(uint64_t(100000000003), ZFlyEmBodyManager::encode(3, 1, false));
  ASSERT_EQ(uint64_t(200000000003), ZFlyEmBodyManager::encode(3, 2, false));
  ASSERT_EQ(uint64_t(9900000000003),
            ZFlyEmBodyManager::encodeSupervoxel(3));
  ASSERT_TRUE(ZFlyEmBodyManager::encodingSupervoxel(9900000000003));
  ASSERT_TRUE(ZFlyEmBodyManager::encodingSupervoxel(19900000000003));
  ASSERT_FALSE(ZFlyEmBodyManager::encodingSupervoxel(9800000000003));


}

#endif

#endif // ZFLYEMBODYMANAGERTEST_H

#ifndef ZFLYEMBODYMANAGERTEST_H
#define ZFLYEMBODYMANAGERTEST_H

#include "ztestheader.h"
#include "flyem/zflyembodymanager.h"

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

  QSet<uint64_t> bodySet = bm.getBodySet();
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

  ASSERT_EQ(2, (int) bm.getHostId(300));
  ASSERT_EQ(20, (int) bm.getHostId(20));

  ASSERT_EQ(2, (int) bm.getSingleBodyId());

  bm.erase(300);
  ASSERT_EQ(300, (int) bm.getHostId(300));
  ASSERT_EQ(2, (int) bm.getSingleBodyId());

  bm.erase(2);
  ASSERT_TRUE(bm.isEmpty());
}

#endif

#endif // ZFLYEMBODYMANAGERTEST_H

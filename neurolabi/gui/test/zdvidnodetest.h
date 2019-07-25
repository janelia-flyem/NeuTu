#ifndef ZDVIDNODETEST_H
#define ZDVIDNODETEST_H

#include "ztestheader.h"
#include "dvid/zdvidnode.h"

#ifdef _USE_GTEST_

TEST(ZDvidNode, Basic)
{
  ZDvidNode node("127.0.0.1", "abcd", 8500);
  ASSERT_EQ("127.0.0.1", node.getAddress());
  ASSERT_EQ("abcd", node.getUuid());
  ASSERT_EQ("abcd", node.getOriginalUuid());
  ASSERT_EQ(8500, node.getPort());
  ASSERT_TRUE(node.hasDvidUuid());

  node.setUuid("@test");
  ASSERT_EQ("@test", node.getUuid());
  ASSERT_EQ("@test", node.getOriginalUuid());
  ASSERT_FALSE(node.hasDvidUuid());

  node.setInferredUuid("1234");
  ASSERT_EQ("1234", node.getUuid());
  ASSERT_EQ("@test", node.getOriginalUuid());
  ASSERT_TRUE(node.hasDvidUuid());

  node.setUuid("abcd");
  ASSERT_EQ("abcd", node.getUuid());
  ASSERT_EQ("abcd", node.getOriginalUuid());
  ASSERT_TRUE(node.hasDvidUuid());

  node.setMappedUuid("@test", "cdef");
  ASSERT_EQ("cdef", node.getUuid());
  ASSERT_EQ("@test", node.getOriginalUuid());
  ASSERT_TRUE(node.hasDvidUuid());
}

#endif

#endif // ZDVIDNODETEST_H

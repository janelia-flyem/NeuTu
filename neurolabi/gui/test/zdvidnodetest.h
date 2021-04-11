#ifndef ZDVIDNODETEST_H
#define ZDVIDNODETEST_H

#include "ztestheader.h"
#include "zjsonobject.h"
#include "zjsonparser.h"
#include "dvid/zdvidnode.h"
#include "qt/network/znetworkutils.h"

#ifdef _USE_GTEST_

TEST(ZDvidNode, Basic)
{
  ZDvidNode node("127.0.0.1", "abcd", 8500);
  ASSERT_EQ("127.0.0.1", node.getHost());
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

TEST(ZDvidNode, Scheme)
{
  ZDvidNode node("127.0.0.1", "abcd", 8500);
  ASSERT_EQ("http", node.getScheme());
  ASSERT_EQ("http:127.0.0.1:8500:abcd", node.getSourceString());
  ASSERT_EQ("127.0.0.1:8500:abcd", node.getSourceString(false));

  node.setScheme("mock");
  ASSERT_EQ("mock", node.getScheme());
  ASSERT_EQ("mock:127.0.0.1:8500:abcd", node.getSourceString());
  ASSERT_EQ("127.0.0.1:8500:abcd", node.getSourceString(false));
}

TEST(ZDvidNode, URL)
{
  ZDvidNode node;
  node.setHost("http://emdata2.int.janelia.org:9000");
  ASSERT_EQ("emdata2.int.janelia.org", node.getHost());
  ASSERT_EQ(9000, node.getPort());

  node.setHost("http://emdata2.int.janelia.org");
  ASSERT_EQ("emdata2.int.janelia.org", node.getHost());
  ASSERT_EQ(9000, node.getPort());

  node.clear();
  node.setHost("http://emdata2.int.janelia.org:9000/api/node/3456/branches/key/master");
  ASSERT_EQ("emdata2.int.janelia.org", node.getHost());
  ASSERT_EQ(9000, node.getPort());

  node.clear();
  node.setHost("http://emdata2.int.janelia.org/9000/api/node/3456/branches/key/master");
  ASSERT_EQ("emdata2.int.janelia.org", node.getHost());
  ASSERT_EQ(-1, node.getPort());

  node.setUuid("234");
  ASSERT_EQ("234", node.getUuid());
  ASSERT_EQ("emdata2.int.janelia.org", node.getAddressWithPort());

  node.clear();
  node.setHost("http://emdata2.int.janelia.org:9000/api/node/23456/branches/key/master");
  ASSERT_EQ("emdata2.int.janelia.org", node.getHost());
  ASSERT_EQ(9000, node.getPort());

  node.setUuid("23456");
  ASSERT_EQ("23456", node.getUuid());

  node.setHost("emdata2.int.janelia.org:9000");
  ASSERT_EQ("emdata2.int.janelia.org", node.getHost());
  ASSERT_EQ(9000, node.getPort());

  node.clear();
  node.setFromUrl_deprecated(
        "http://emdata2.int.janelia.org:9000/api/node/3456/branches/key/master");
  ASSERT_EQ("emdata2.int.janelia.org", node.getHost());
  ASSERT_EQ(9000, node.getPort());
  ASSERT_EQ("3456", node.getUuid());

  node.clear();
  node.setFromUrl_deprecated(
        "http://emdata2.int.janelia.org:9000/api/node/123456/branches/key/master");
  ASSERT_EQ("emdata2.int.janelia.org", node.getHost());
  ASSERT_EQ(9000, node.getPort());
  ASSERT_EQ("123456", node.getUuid());

  ZJsonObject obj;
  obj.decodeString("{\"address\":\"hackathon.janelia.org\", \"uuid\": \"2a3\"}");
  node.loadJsonObject(obj);

  ASSERT_EQ("hackathon.janelia.org", node.getHost());
  ASSERT_EQ(-1, node.getPort());
  ASSERT_EQ("2a3", node.getUuid());

  obj.decodeString("{\"address\":\"hackathon.janelia.org\", \"port\": 8800, "
                   "\"uuid\": \"2a3\"}");
  node.loadJsonObject(obj);

  ZJsonObject obj2 = node.toJsonObject();
  ASSERT_EQ("hackathon.janelia.org", ZJsonParser::stringValue(obj2["host"]));
  ASSERT_EQ("2a3", ZJsonParser::stringValue(obj2["uuid"]));
  ASSERT_EQ(8800, ZJsonParser::integerValue(obj2["port"]));

  obj.decodeString("{\"host\":\"hackathon.janelia.org\", \"port\": 8800, "
                   "\"uuid\": \"2a3\", \"scheme\": \"https\"}");
  node.loadJsonObject(obj);
  ASSERT_EQ("hackathon.janelia.org", node.getHost());
  ASSERT_EQ("https", node.getScheme());
  ASSERT_EQ(8800, node.getPort());
  ASSERT_EQ("2a3", node.getUuid());

//  node.print();

  ZDvidNode node2;
  node2.set("emdata2.int.janelia.org", "uuid", 8000);

  ASSERT_EQ(node, node);
  ASSERT_NE(node, node2);

  ZDvidNode node3;
  node3.set("emdata2.int.janelia.org", "uuid", 8100);
  ASSERT_NE(node2, node3);

  {
    std::vector<std::string> tokens = {"1", "2", "3"};
    node.setFromSourceToken(tokens);
    ASSERT_FALSE(node.isValid());
  }

  {
    std::vector<std::string> tokens = {
      "http", "emdata2.int.janelia.org", "-1", "2b6c"};
    node.setFromSourceToken(tokens);
    ASSERT_TRUE(node.isValid());
    ASSERT_FALSE(node.isMock());
  }

  {
    std::vector<std::string> tokens = {
      "mock", "emdata2.int.janelia.org", "-1", "2b6c"};
    node.setFromSourceToken(tokens);
    ASSERT_TRUE(node.isValid());
    ASSERT_TRUE(node.isMock());
  }

  node.setFromSourceString("http:emdata2.int.janelia.org:-1:2b6c");
  ASSERT_TRUE(node.isValid());
  ASSERT_FALSE(node.isMock());
  ASSERT_EQ("http:emdata2.int.janelia.org:-1:2b6c", node.getSourceString(true));

  node2.set("emdata2.int.janelia.org", "2b6c", -1);
  ASSERT_EQ(node, node2);
  node.setMock(true);
  ASSERT_NE(node, node2);
  node2.setMock(true);
  ASSERT_EQ(node, node2);

  node.setFromSourceString("mock:emdata2.int.janelia.org:-1:2b6c");
  ASSERT_TRUE(node.isValid());
  ASSERT_TRUE(node.isMock());
  ASSERT_EQ("mock:emdata2.int.janelia.org:-1:2b6c", node.getSourceString(true));

  node.setFromUrl_deprecated(
        "http://emdata2.int.janelia.org:9000/api/node/3456/branches/key/master");
  ASSERT_FALSE(node.isMock());
  ASSERT_EQ("http:emdata2.int.janelia.org:9000:3456", node.getSourceString(true));

  node.setFromUrl_deprecated(
        "http://emdata2.int.janelia.org:9000/api/node/123456789/branches/key/master");
  ASSERT_EQ("http:emdata2.int.janelia.org:9000:123456789", node.getSourceString(true));
  ASSERT_EQ("http:emdata2.int.janelia.org:9000:123", node.getSourceString(true, 3));
  ASSERT_EQ("http:emdata2.int.janelia.org:9000:1234", node.getSourceString(true, 4));
  ASSERT_EQ("http:emdata2.int.janelia.org:9000:12345", node.getSourceString(true, 5));
  ASSERT_EQ("http:emdata2.int.janelia.org:9000:123456789", node.getSourceString(true, 30));
  ASSERT_EQ("http:emdata2.int.janelia.org:9000:123456789", node.getSourceString(true, 0));
  ASSERT_EQ("http:emdata2.int.janelia.org:9000:123456789", node.getSourceString(true, -1));

  node.setFromUrl_deprecated(
        "http://emdata2.int.janelia.org:9000/api/node/@FIB25/branches/key/master");
  ASSERT_EQ("http:emdata2.int.janelia.org:9000:@FIB25", node.getSourceString(true));

  node.setFromUrl_deprecated(
        "http://emdata2.int.janelia.org:9000/api/node/ref>FIB25/branches/key/master");
  ASSERT_EQ("http:emdata2.int.janelia.org:9000:", node.getSourceString(true));

  node.setFromUrl_deprecated(
        "mock://emdata2.int.janelia.org:9000/api/node/3456/branches/key/master");
  ASSERT_TRUE(node.isMock());
  ASSERT_EQ("mock:emdata2.int.janelia.org:9000:3456", node.getSourceString(true));
}

TEST(ZDvidNode, uuid) {
  ASSERT_TRUE(ZDvidNode::IsValidDvidUuid("1234"));
  ASSERT_TRUE(ZDvidNode::IsValidDvidUuid("abcd"));
  ASSERT_TRUE(ZDvidNode::IsValidDvidUuid("12ab"));
  ASSERT_TRUE(ZDvidNode::IsValidDvidUuid("ef34"));
  ASSERT_TRUE(ZDvidNode::IsValidDvidUuid("1234:master"));
  ASSERT_TRUE(ZDvidNode::IsValidDvidUuid("ef34:master"));
  ASSERT_TRUE(ZDvidNode::IsValidDvidUuid("1234:main"));
  ASSERT_TRUE(ZDvidNode::IsValidDvidUuid("ef34:test"));

  ASSERT_TRUE(ZDvidNode::IsValidUuid("1234"));
  ASSERT_TRUE(ZDvidNode::IsValidUuid("abcd"));
  ASSERT_TRUE(ZDvidNode::IsValidUuid("12ab"));
  ASSERT_TRUE(ZDvidNode::IsValidUuid("ef34"));
  ASSERT_TRUE(ZDvidNode::IsValidUuid("1234:master"));
  ASSERT_TRUE(ZDvidNode::IsValidUuid("ef34:master"));

  ASSERT_TRUE(ZDvidNode::IsMasterUuid("1234:master"));
  ASSERT_TRUE(ZDvidNode::IsMasterUuid("ef34:master"));

  ASSERT_FALSE(ZDvidNode::IsValidDvidUuid(" ef34"));
  ASSERT_FALSE(ZDvidNode::IsValidDvidUuid("-1234:master"));
  ASSERT_FALSE(ZDvidNode::IsValidDvidUuid(":master"));
  ASSERT_FALSE(ZDvidNode::IsValidDvidUuid("!234"));
  ASSERT_FALSE(ZDvidNode::IsValidDvidUuid(": master"));

  ASSERT_FALSE(ZDvidNode::IsValidUuid(" ef34"));
  ASSERT_FALSE(ZDvidNode::IsValidUuid("-1234:master"));
  ASSERT_FALSE(ZDvidNode::IsValidUuid(":master"));
  ASSERT_FALSE(ZDvidNode::IsValidUuid("!234"));

  ASSERT_TRUE(ZDvidNode::IsUuidRef("ref:test"));
  ASSERT_TRUE(ZDvidNode::IsUuidRef("ref>test"));
  ASSERT_FALSE(ZDvidNode::IsUuidRef("ef34"));
  ASSERT_FALSE(ZDvidNode::IsUuidRef("1234:master"));
  ASSERT_FALSE(ZDvidNode::IsValidDvidUuid("ref:test"));
  ASSERT_FALSE(ZDvidNode::IsValidDvidUuid("ref>test"));

  ASSERT_TRUE(ZDvidNode::IsValidUuid("ref:test"));
  ASSERT_TRUE(ZDvidNode::IsValidUuid("ref>test"));

  ASSERT_TRUE(ZDvidNode::IsUuidAlias("@alias"));
  ASSERT_TRUE(ZDvidNode::IsUuidAlias("@@alias"));
  ASSERT_FALSE(ZDvidNode::IsUuidAlias("@"));
  ASSERT_FALSE(ZDvidNode::IsUuidAlias("1234"));
  ASSERT_FALSE(ZDvidNode::IsUuidAlias("1234:master"));

  ASSERT_TRUE(ZDvidNode::IsValidUuid("@alias"));
  ASSERT_TRUE(ZDvidNode::IsValidUuid("@@alias"));
  ASSERT_FALSE(ZDvidNode::IsValidUuid("@"));
  ASSERT_TRUE(ZDvidNode::IsValidUuid("1234"));
  ASSERT_TRUE(ZDvidNode::IsValidUuid("1234:master"));
}

TEST(ZDvidNode, deref)
{
  ASSERT_EQ("1234", ZDvidNode::DerefUuid("1234"));
  ASSERT_EQ("", ZDvidNode::DerefUuid("1234", -1));
  ASSERT_EQ("1234", ZDvidNode::DerefUuid("1234", 0));

  ASSERT_EQ("", ZDvidNode::DerefUuid("ref:test"));
  ASSERT_EQ("", ZDvidNode::DerefUuid("ref>test"));

  if (ZNetworkUtils::IsAvailable(
        "http://127.0.0.1:1600/api/node/c315/neutu_config/key/uuid1", "HEAD")) {
    ASSERT_EQ("abcd", ZDvidNode::DerefUuid("ref>127.0.0.1:1600/api/node/c315/neutu_config/key/uuid1"));
  }

  if (ZNetworkUtils::IsAvailable(
        "http://127.0.0.1:1600/api/node/c315/neutu_config/key/uuid2", "HEAD")) {
    ASSERT_EQ("abcd", ZDvidNode::DerefUuid("ref>127.0.0.1:1600/api/node/c315/neutu_config/key/uuid2"));
    ASSERT_EQ("abcd", ZDvidNode::DerefUuid("ref>http://127.0.0.1:1600/api/node/c315/neutu_config/key/uuid2"));
  }

  if (ZNetworkUtils::IsAvailable(
        "http://127.0.0.1:1600/api/node/c315/neutu_config/key/uuid3", "HEAD")) {
    ASSERT_EQ("", ZDvidNode::DerefUuid("ref>127.0.0.1:1600/api/node/c315/neutu_config/key/uuid3", 10));
  }
}

#endif

#endif // ZDVIDNODETEST_H

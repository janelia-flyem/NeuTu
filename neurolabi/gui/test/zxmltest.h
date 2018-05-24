#ifndef ZXMLTEST_H
#define ZXMLTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zxmldoc.h"

#ifdef _USE_GTEST_
TEST(ZXmlDoc, Basic)
{
  ZXmlDoc doc;
  doc.parseFile(GET_BENCHMARK_DIR + "/config.xml");

  ASSERT_FALSE(doc.getRootElement().empty());
  ASSERT_EQ("neutube", doc.getRootElement().name());

  doc.printInfo();

  ZXmlNode root = doc.getRootElement();
  ASSERT_TRUE(root.isElement());

  ASSERT_TRUE(root.stringValue().empty());
  ASSERT_FALSE(root.firstChild().empty());

  ZXmlNode node = root.firstChild();
  ASSERT_EQ("docUrl", node.name());
  ASSERT_EQ("https://wiki.janelia.org/wiki/display/flyem/NeuTu", node.stringValue());

  node = node.nextSibling();
  ASSERT_EQ("dataPath", node.name());

  ASSERT_TRUE(node.getAttribute("test").empty());
  ASSERT_TRUE(node.isElement());

  node = node.queryNode("Graph");
  ASSERT_TRUE(node.empty());

  node = node.nextSibling();
  ASSERT_TRUE(node.empty());

  node = root.queryNode("Graph");
  ASSERT_FALSE(node.empty());
  ASSERT_EQ("on", node.getAttribute("status"));

  node = root.queryNode("Visible");
  ASSERT_EQ(1, node.intValue());
}

#endif


#endif // ZXMLTEST_H

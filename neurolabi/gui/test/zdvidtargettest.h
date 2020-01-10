#ifndef ZDVIDTARGETTEST_H
#define ZDVIDTARGETTEST_H

#include "ztestheader.h"
#include "dvid/zdvidtarget.h"

#ifdef _USE_GTEST_

TEST(ZDvidTarget, Basic)
{
  {
    ZDvidTarget target("emdata.janelia.org", "1234", 1000);
    ASSERT_FALSE(target.isInferred());
    ASSERT_EQ("1234", target.getUuid());
    ASSERT_EQ("1234", target.getOriginalUuid());
    ASSERT_TRUE(target.getBodyLabelName().empty());
    ASSERT_TRUE(target.getSegmentationName().empty());
    ASSERT_FALSE(target.hasBodyLabel());
    ASSERT_FALSE(target.hasSegmentation());
    ASSERT_FALSE(target.hasLabelMapData());
    ASSERT_TRUE(target.getSkeletonName().empty());

    target.setSegmentationName("test");
    ASSERT_EQ("test", target.getSegmentationName());
    ASSERT_TRUE(target.getBodyLabelName().empty());
    ASSERT_TRUE(target.getSkeletonName().empty());
    ASSERT_TRUE(target.getBodyAnnotationName().empty());
    ASSERT_TRUE(target.getMeshName().empty());


    target.setSegmentationType(ZDvidData::EType::LABELMAP);
    ASSERT_EQ("test", target.getBodyLabelName());
    ASSERT_EQ("test_skeletons", target.getSkeletonName());
    ASSERT_EQ("test_annotations", target.getBodyAnnotationName());
    ASSERT_EQ("test_meshes", target.getMeshName());


  }

  {
    ZDvidTarget target("emdata.janelia.org", "@test", 1000);
    ASSERT_FALSE(target.isInferred());
    target.setMappedUuid(target.getUuid(), "1234");

    ASSERT_EQ("1234", target.getUuid());
    ASSERT_EQ("@test", target.getOriginalUuid());
  }

  {
    ASSERT_TRUE(ZDvidTarget::Test());

    ZDvidTarget target;
    target.setServer("http://emdata2.int.janelia.org:9000");
    ASSERT_EQ("emdata2.int.janelia.org", target.getAddress());
    ASSERT_EQ(9000, target.getPort());

    target.setServer("http://emdata2.int.janelia.org");
    ASSERT_EQ("emdata2.int.janelia.org", target.getAddress());
    ASSERT_EQ(9000, target.getPort());

    target.clear();
    target.setServer(
          "http://emdata2.int.janelia.org:9000/api/node/3456/branches/key/master");
    ASSERT_EQ("emdata2.int.janelia.org", target.getAddress());
    ASSERT_EQ(9000, target.getPort());

    target.clear();
    target.setServer(
          "http://emdata2.int.janelia.org/9000/api/node/3456/branches/key/master");
    ASSERT_EQ("emdata2.int.janelia.org", target.getAddress());
    ASSERT_EQ(-1, target.getPort());

    target.setUuid("234");
    ASSERT_EQ("234", target.getUuid());
    ASSERT_EQ("emdata2.int.janelia.org", target.getAddressWithPort());

    target.setUuid("12345");
    ASSERT_EQ("12345", target.getUuid());

    target.setServer("emdata2.int.janelia.org:9000");
    ASSERT_EQ("emdata2.int.janelia.org", target.getAddress());
    ASSERT_EQ(9000, target.getPort());

    target.clear();
    target.setFromUrl_deprecated(
          "http://emdata2.int.janelia.org:9000/api/node/3456/branches/key/master");
    ASSERT_EQ("emdata2.int.janelia.org", target.getAddress());
    ASSERT_EQ(9000, target.getPort());
    ASSERT_EQ("3456", target.getUuid());
    target.setTodoListName("test");
    ASSERT_EQ("test", target.getTodoListName());

    ZJsonObject obj;
    obj.decodeString("{\"gray_scale\":{\"address\":\"hackathon.janelia.org\", \"port\": 8800, "
                     "\"uuid\": \"2a3\"}}");
    target.setSourceConfig(obj);
    target.prepareTile();
    ASSERT_EQ("emdata2.int.janelia.org", target.getAddress());
    ASSERT_EQ(9000, target.getPort());
    ASSERT_EQ("3456", target.getUuid());

    target.prepareGrayScale();
    ASSERT_EQ("hackathon.janelia.org", target.getAddress());
    ASSERT_EQ("2a3", target.getUuid());
    ASSERT_EQ(8800, target.getPort());

    obj.decodeString("{\"multires_tile\":{\"address\":\"hackathon2.janelia.org\", \"port\": 9800, "
                     "\"uuid\": \"1a3\"}}");
    target.setSourceConfig(obj);
    target.prepareTile();
    ASSERT_EQ("hackathon2.janelia.org", target.getAddress());
    ASSERT_EQ("1a3", target.getUuid());
    ASSERT_EQ(9800, target.getPort());
    target.print();

    target.setTileSource(ZDvidNode("emdata2.int.janelia.org", "1234", 9000));
    ZDvidNode node = target.getTileSource();
    ASSERT_EQ("emdata2.int.janelia.org", node.getAddress());
    ASSERT_EQ(9000, node.getPort());
    ASSERT_EQ("1234", node.getUuid());

    ASSERT_FALSE(target.isLowQualityTile("tiles"));
    target.configTile("tiles", true);
    ASSERT_TRUE(target.isLowQualityTile("tiles"));

    target.setGrayScaleSource(ZDvidNode("emdata3.int.janelia.org", "2234", 9100));
    node = target.getGrayScaleSource();
    ASSERT_EQ("emdata3.int.janelia.org", node.getAddress());
    ASSERT_EQ(9100, node.getPort());
    ASSERT_EQ("2234", node.getUuid());

    target.setTileSource(ZDvidNode("", "", -1));
    node = target.getTileSource();
    ASSERT_EQ("hackathon2.janelia.org", node.getAddress());
    ASSERT_EQ(9800, node.getPort());
    ASSERT_EQ("1a3", node.getUuid());

    target.clear();
    target.setFromUrl_deprecated(
          "mock://emdata2.int.janelia.org:9000/api/node/3456/branches/key/master");
    ASSERT_EQ("emdata2.int.janelia.org", target.getAddress());
    ASSERT_EQ(9000, target.getPort());
    ASSERT_EQ("3456", target.getUuid());
    target.setTodoListName("test");
    ASSERT_EQ("test", target.getTodoListName());
    ASSERT_TRUE(target.isMock());
    ASSERT_EQ("mock:emdata2.int.janelia.org:9000:3456", target.getSourceString());
    ASSERT_EQ("mock:emdata2.int.janelia.org:9000:3456::",
              target.getGrayscaleSourceString());

    target.setFromUrl_deprecated(
          "http://emdata3.int.janelia.org:9100/api/node/1234/body_test/sparsevol/123");
    ASSERT_EQ("emdata3.int.janelia.org", target.getAddress());
    ASSERT_EQ(9100, target.getPort());
    ASSERT_EQ("1234", target.getUuid());
    target.setTodoListName("test");
    ASSERT_EQ("test", target.getTodoListName());
    ASSERT_FALSE(target.isMock());
    ASSERT_EQ("body_test", target.getBodyLabelName());
    ASSERT_EQ("http:emdata3.int.janelia.org:9100:1234:body_test", target.getSourceString());
    ASSERT_EQ("http:emdata3.int.janelia.org:9100:1234::",
              target.getGrayscaleSourceString());

    target.setFromUrl_deprecated(
          "mock://emdata3.int.janelia.org:9100/api/node/1234/body_test/sparsevol/123");
    ASSERT_EQ("emdata3.int.janelia.org", target.getAddress());
    ASSERT_EQ(9100, target.getPort());
    ASSERT_EQ("1234", target.getUuid());
    target.setTodoListName("test");
    ASSERT_EQ("test", target.getTodoListName());
    ASSERT_TRUE(target.isMock());
    ASSERT_EQ("body_test", target.getBodyLabelName());
    ASSERT_EQ("mock:emdata3.int.janelia.org:9100:1234:body_test",
              target.getSourceString());
    target.setGrayScaleName("grayscale");
    ASSERT_EQ("mock:emdata3.int.janelia.org:9100:1234::grayscale",
              target.getGrayscaleSourceString());
  }
}

#endif

#endif // ZDVIDTARGETTEST_H

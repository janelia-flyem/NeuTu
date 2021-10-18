#ifndef ZDVIDTARGETTEST_H
#define ZDVIDTARGETTEST_H

#include "ztestheader.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidtargetfactory.h"

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
    ASSERT_TRUE(target.isValid());

    ASSERT_EQ("", target.getBodyAnnotationName());
    ASSERT_EQ("", target.getMeshName());
    ASSERT_EQ("", target.getThumbnailName());


    target.setSegmentationName("test");
    ASSERT_EQ("test", target.getSegmentationName());
    ASSERT_FALSE(target.hasLabelMapData());
    ASSERT_FALSE(target.hasBlockCoding());
    ASSERT_FALSE(target.hasSupervoxel());
    ASSERT_FALSE(target.hasSparsevolSizeApi());
    ASSERT_FALSE(target.hasMultiscaleSegmentation());
    ASSERT_FALSE(target.segmentationAsBodyLabel());
    ASSERT_TRUE(target.hasCoarseSplit());
    ASSERT_TRUE(target.hasSegmentation());
    ASSERT_TRUE(target.getBodyLabelName().empty());
    ASSERT_TRUE(target.getSkeletonName().empty());
    ASSERT_TRUE(target.getBodyAnnotationName().empty());
    ASSERT_TRUE(target.getMeshName().empty());

    target.setSegmentationType(ZDvidData::EType::LABELMAP);
    ASSERT_TRUE(target.hasLabelMapData());
    ASSERT_TRUE(target.hasBlockCoding());
    ASSERT_TRUE(target.hasSupervoxel());
    ASSERT_TRUE(target.segmentationAsBodyLabel());
    ASSERT_TRUE(target.hasMultiscaleSegmentation());
    ASSERT_TRUE(target.hasSparsevolSizeApi());
    ASSERT_FALSE(target.hasCoarseSplit());
    ASSERT_EQ("test_1", target.getSegmentationName(1));
    ASSERT_TRUE(target.getValidSegmentationName(1).empty());
    ASSERT_EQ("test", target.getBodyLabelName());
    ASSERT_EQ("test_skeletons", target.getSkeletonName());
    ASSERT_EQ("test_annotations", target.getBodyAnnotationName());
    ASSERT_EQ("test_meshes", target.getMeshName());
    ASSERT_EQ("test_thumbnails", target.getThumbnailName());

    target.setBodyLabelName("bodies");
    ASSERT_EQ("bodies", target.getBodyLabelName());
    ASSERT_EQ("bodyinfo", target.getBodyInfoName());
    ASSERT_EQ("skeletons", target.getSkeletonName());
    ASSERT_EQ("annotations", target.getBodyAnnotationName());

    target.setBodyLabelName("bodies2");
    ASSERT_EQ("bodies2", target.getBodyLabelName());
    ASSERT_EQ("bodies2_bodyinfo", target.getBodyInfoName());
    ASSERT_EQ("bodies2_skeletons", target.getSkeletonName());

    target.setMultiscale2dName("tiles");
    ASSERT_EQ("tiles", target.getMultiscale2dName());
    target.setDefaultMultiscale2dName();
    ASSERT_TRUE(target.getMultiscale2dName().empty());

    ASSERT_FALSE(target.isLowQualityTile("tiles"));
    target.configTile("tiles", true);
    ASSERT_TRUE(target.isLowQualityTile("tiles"));

    target.setNullBodyLabelName();
    ASSERT_FALSE(target.hasBodyLabel());
    ASSERT_TRUE(target.getBodyInfoName().empty());

    ASSERT_FALSE(target.hasSynapse());
    ASSERT_FALSE(target.hasSynapseLabelsz());

    target.setSynapseName("synapses");
    ASSERT_TRUE(target.hasSynapse());
    ASSERT_TRUE(target.hasSynapseLabelsz());

    target.enableSynapseLabelsz(false);
    ASSERT_FALSE(target.hasSynapseLabelsz());

    ASSERT_FALSE(target.hasGrayScaleData());
    target.setGrayScaleName("gray");
    ASSERT_EQ("gray", target.getGrayScaleName());
    ASSERT_EQ("gray", target.getGrayScaleName(0));
    ASSERT_EQ("gray_1", target.getGrayScaleName(1));
    ASSERT_TRUE(target.getValidGrayScaleName(1).empty());

    target.setMaxGrayscaleZoom(3);
    ASSERT_EQ("gray_1", target.getValidGrayScaleName(1));
    ASSERT_EQ("gray_3", target.getValidGrayScaleName(3));
    ASSERT_TRUE(target.getValidGrayScaleName(4).empty());

    target.setRoiName("roi");
    ASSERT_EQ("roi", target.getRoiName());

    ASSERT_EQ("bookmark_annotations", target.getBookmarkName());
    ASSERT_EQ("bookmarks", target.getBookmarkKeyName());
    ASSERT_TRUE(target.hasBookmark());

    ASSERT_EQ(dvid::ENodeStatus::OFFLINE, target.getNodeStatus());
    target.setNodeStatus(dvid::ENodeStatus::NORMAL);
    ASSERT_EQ(dvid::ENodeStatus::NORMAL, target.getNodeStatus());

    target.setNullSegmentationName();
    ASSERT_TRUE(target.getSegmentationName().empty());
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
    ASSERT_FALSE(target.isValid());
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

    target.setTileSource(ZDvidNode("test.host.org", "1234", 9000));
    ZDvidNode node = target.getTileSource();
    ASSERT_EQ("test.host.org", node.getHost());
    ASSERT_EQ(9000, node.getPort());
    ASSERT_EQ("1234", node.getUuid());

    ASSERT_FALSE(target.isLowQualityTile("tiles"));
    target.configTile("tiles", true);
    ASSERT_TRUE(target.isLowQualityTile("tiles"));

    target.setGrayScaleSource(ZDvidNode("test.host.org", "2234", 9100));
    node = target.getGrayScaleSource();
    ASSERT_EQ("test.host.org", node.getHost());
    ASSERT_EQ(9100, node.getPort());
    ASSERT_EQ("2234", node.getUuid());

    target.setTileSource(ZDvidNode("", "", -1));
    node = target.getTileSource();
    ASSERT_EQ("hackathon2.janelia.org", node.getHost());
    ASSERT_EQ(9800, node.getPort());
    ASSERT_EQ("1a3", node.getUuid());

    target.clearGrayScale();
    ASSERT_FALSE(target.hasGrayScaleData());

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
    ASSERT_EQ("body_test", target.getBodyLabelName(0));
    ASSERT_EQ("body_test_1", target.getBodyLabelName(1));
    ASSERT_EQ("mock:emdata3.int.janelia.org:9100:1234:body_test",
              target.getSourceString());
    target.setGrayScaleName("grayscale");
    ASSERT_EQ("mock:emdata3.int.janelia.org:9100:1234::grayscale",
              target.getGrayscaleSourceString());
  }
}

TEST(ZDvidTarget, json) {
  ZJsonObject obj;
  obj.decode("{"
    "  \"port\": 1600,"
    "  \"host\": \"127.0.0.1\","
    "  \"uuid\": \"c315\","
    "  \"name\": \"local_test\","
    "  \"background\": 255,"
    "  \"label_block\": \"segmentation\","
    "  \"gray_scale\": \"grayscale\","
    "  \"roi\": \"test\","
    "  \"todo\": \"segmentation_todo\","
    "  \"proofreading\": true,"
    "  \"roi_list\": [],"
    "  \"synapse\": \"synapses\","
    "  \"supervised\": false,"
    "  \"default\": false"
    "}", true);

  {
    ZDvidTarget target;
    target.setFromJson(obj.dumpString());
    ASSERT_EQ("127.0.0.1", target.getAddress());
    ASSERT_EQ("127.0.0.1:1600", target.getAddressWithPort());
    ASSERT_EQ("127.0.0.1", target.getHostWithScheme());
    ASSERT_EQ("c315", target.getUuid());
    ASSERT_EQ("segmentation", target.getSegmentationName());
    ASSERT_EQ("synapses", target.getSynapseName());
    ASSERT_EQ("grayscale", target.getGrayScaleName());
    ASSERT_EQ("segmentation_todo", target.getTodoListName());
  }

  {
    ZDvidTarget target;
    target.loadJsonObject(obj);
    ASSERT_EQ("127.0.0.1", target.getAddress());
    ASSERT_EQ("127.0.0.1:1600", target.getAddressWithPort());
    ASSERT_EQ("127.0.0.1", target.getHostWithScheme());
    ASSERT_EQ("c315", target.getUuid());
    ASSERT_EQ("segmentation", target.getSegmentationName());
    ASSERT_EQ("synapses", target.getSynapseName());
    ASSERT_EQ("grayscale", target.getGrayScaleName());
    ASSERT_EQ("segmentation_todo", target.getTodoListName());

    ZJsonObject dvidSetting;
    dvidSetting.setEntry("segmentation", "seg");
    dvidSetting.setEntry("todos", "test_todos");
    target.loadDvidDataSetting(dvidSetting);
    ASSERT_EQ("seg", target.getSegmentationName());
    ASSERT_EQ("synapses", target.getSynapseName());
    ASSERT_EQ("grayscale", target.getGrayScaleName());
    ASSERT_EQ("test_todos", target.getTodoListName());

  }
}

TEST(ZDvidTarget, Factory)
{
  {
    ZDvidTarget target = ZDvidTargetFactory::MakeFromSourceString(
          "mock:emdata2.int.janelia.org:9000:3456");
    ASSERT_EQ("mock", target.getScheme());
    ASSERT_EQ("emdata2.int.janelia.org", target.getAddress());
    ASSERT_EQ(9000, target.getPort());
    ASSERT_EQ("3456", target.getUuid());

    target = ZDvidTargetFactory::MakeFromSpec(
              "https://emdata2.int.janelia.org:9000"
              "?uuid=3456&segmentation=seg&grayscale=gs");
    ASSERT_EQ("https", target.getScheme());
    ASSERT_EQ("emdata2.int.janelia.org", target.getAddress());
    ASSERT_EQ(9000, target.getPort());
    ASSERT_EQ("3456", target.getUuid());
    ASSERT_EQ("seg", target.getSegmentationName());
    ASSERT_EQ("gs", target.getGrayScaleName());

  }
}

TEST(ZDvidTarget, Json)
{
  {
    ZDvidTarget target;
    target.setFromJson(
          "{\"host\": \"emdata2.int.janelia.org\", "
          "\"port\": 9000, \"scheme\": \"mock\", \"name\": \"test\"}");
    target.toJsonObject().print();
    ASSERT_EQ("test", target.getName());
    ASSERT_TRUE(target.isMock());
  }
}

#endif

#endif // ZDVIDTARGETTEST_H

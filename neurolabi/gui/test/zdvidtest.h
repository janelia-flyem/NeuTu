#ifndef ZDVIDTEST_H
#define ZDVIDTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "dvid/zdvidinfo.h"
#include "dvid/zdvidbuffer.h"
#include "dvid/zdvidtarget.h"
#include "dialogs/zdviddialog.h"
#include "zstring.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidurl.h"
#include "dvid/zdviddata.h"
#include "zdvidutil.h"
#include "dvid/zdvidnode.h"

#ifdef _USE_GTEST_

TEST(ZDvidTest, ZDvidInfo)
{
  ZDvidInfo info;
  info.print();
  /*
  const char* ZDvidInfo::m_minPointKey = "MinPoint";
  const char* ZDvidInfo::m_maxPointKey = "MaxPoint";
  const char* ZDvidInfo::m_blockSizeKey = "BlockSize";
  const char* ZDvidInfo::m_voxelSizeKey = "VoxelSize";
  const char* ZDvidInfo::m_blockMinIndexKey = "MinIndex";
*/
  info.setFromJsonString("{ \"MinPoint\": [1, 2, 3]}");
  info.print();

  ASSERT_EQ(1, info.getStartCoordinates().getX());
  ASSERT_EQ(2, info.getStartCoordinates().getY());
  ASSERT_EQ(3, info.getStartCoordinates().getZ());
}

TEST(ZDvidTest, Util)
{
  ASSERT_TRUE(ZDvid::IsUuidMatched("12345", "123"));
  ASSERT_TRUE(ZDvid::IsUuidMatched("aad345", "aad"));
  ASSERT_TRUE(ZDvid::IsUuidMatched("123", "123fwrq424q"));
  ASSERT_FALSE(ZDvid::IsUuidMatched("", "123"));
  ASSERT_FALSE(ZDvid::IsUuidMatched("12345", ""));
  ASSERT_FALSE(ZDvid::IsUuidMatched("12345", "12346"));
  ASSERT_FALSE(ZDvid::IsUuidMatched("234", "12346"));
}

TEST(ZDvidTest, ZDvidUrl)
{
  ZDvidTarget target("emdata.janelia.org", "bf1");
  target.setLabelBlockName("labels");
  //const std::vector<ZDvidTarget> &dvidRepo =
  //    NeutubeConfig::getInstance().getFlyEmConfig().getDvidRepo();
  ZDvidUrl dvidUrl(target);
  std::cout << dvidUrl.getHelpUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/help", dvidUrl.getHelpUrl());

//  std::cout << dvidUrl.getSkeletonUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/skeletons",
            dvidUrl.getSkeletonUrl());

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/skeletons/key/1_swc",
            dvidUrl.getSkeletonUrl(1));

  ASSERT_TRUE(dvidUrl.getSkeletonUrl("").empty());

//  std::cout << dvidUrl.getMergeOperationUrl(
//                 ZDvidData::GetName(ZDvidData::ROLE_MERGE_OPERATION))
//            << std::endl;
  /*
  ASSERT_EQ(std::string("http://emdata.janelia.org/api/node/bf1/") +
            ZDvidData::GetName(ZDvidData::ROLE_MERGE_OPERATION),
            dvidUrl.getMergeOperationUrl(
              ZDvidData::GetName(ZDvidData::ROLE_MERGE_OPERATION)));
*/
//  std::cout << dvidUrl.getNodeUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1", dvidUrl.getNodeUrl());

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test",
            dvidUrl.getDataUrl("test"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale",
            dvidUrl.getDataUrl(ZDvidData::ROLE_GRAY_SCALE));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale",
            dvidUrl.getDataUrl(ZDvidData::ROLE_GRAY_SCALE,
                               ZDvidData::ROLE_BODY_LABEL, "bodies"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_grayscale",
            dvidUrl.getDataUrl(ZDvidData::ROLE_GRAY_SCALE,
                               ZDvidData::ROLE_BODY_LABEL, "bodies2"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/info",
            dvidUrl.getInfoUrl("test"));

  ASSERT_EQ("http://emdata.janelia.org/api/help",
            dvidUrl.getHelpUrl());

  ASSERT_EQ("http://emdata.janelia.org/api/server/info",
            dvidUrl.getServerInfoUrl());
//  std::cout << dvidUrl.getServerInfoUrl() << std::endl;

  ASSERT_EQ("http://emdata.janelia.org/api", dvidUrl.getApiUrl());
//  std::cout << dvidUrl.getApiUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/repo/bf1", dvidUrl.getRepoUrl());
//  std::cout << dvidUrl.getRepoUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/repo/bf1/instance",
            dvidUrl.getInstanceUrl());

//  target.setBodyLabelName("bodies2");
//  std::cout << dvidUrl.getSkeletonUrl("") << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_skeletons",
            dvidUrl.getSkeletonUrl("bodies2"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_skeletons/key/1_swc",
            dvidUrl.getSkeletonUrl(1, "bodies2"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/thumbnails/key/1_mraw",
            dvidUrl.getThumbnailUrl(1));

  target.setGrayScaleName("grayscale2");
  target.setBodyLabelName("bodies2");
  target.setLabelBlockName("labels2");
  ZDvidUrl dvidUrl2(target);
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_skeletons",
            dvidUrl2.getSkeletonUrl());
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_skeletons/key/1_swc",
            dvidUrl2.getSkeletonUrl(1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies/sparsevol",
            dvidUrl.getSparsevolUrl("bodies"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies/sparsevol/1",
            dvidUrl.getSparsevolUrl(1, "bodies"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies/sparsevol/1",
            dvidUrl.getSparsevolUrl(1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2/sparsevol",
            dvidUrl.getSparsevolUrl("bodies2"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2/sparsevol/1",
            dvidUrl.getSparsevolUrl(1, "bodies2"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2/sparsevol/1",
            dvidUrl2.getSparsevolUrl(1));


  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies/sparsevol-coarse",
            dvidUrl.getCoarseSparsevolUrl("bodies"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies/sparsevol-coarse/1",
            dvidUrl.getCoarseSparsevolUrl(1, "bodies"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies/sparsevol-coarse/1",
            dvidUrl.getCoarseSparsevolUrl(1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2/sparsevol-coarse",
            dvidUrl.getCoarseSparsevolUrl("bodies2"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2/sparsevol-coarse/1",
            dvidUrl.getCoarseSparsevolUrl(1, "bodies2"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2/sparsevol-coarse/1",
            dvidUrl2.getCoarseSparsevolUrl(1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale",
            dvidUrl.getGrayscaleUrl());
//  std::cout << dvidUrl.getGrayscaleUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale/raw/0_1/512_1024/1_2_3",
            dvidUrl.getGrayscaleUrl(512, 1024, 1, 2, 3));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale/raw/0_1/512_1024/1_2_3/png",
            dvidUrl.getGrayscaleUrl(512, 1024, 1, 2, 3, "png"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale2",
            dvidUrl2.getGrayscaleUrl());
//  std::cout << dvidUrl.getGrayscaleUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale2/raw/0_1/512_1024/1_2_3",
            dvidUrl2.getGrayscaleUrl(512, 1024, 1, 2, 3));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale2/raw/0_1/512_1024/1_2_3/png",
            dvidUrl2.getGrayscaleUrl(512, 1024, 1, 2, 3, "png"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale/blocks/1_2_3/1",
            dvidUrl.getGrayScaleBlockUrl(1, 2, 3));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale/blocks/1_2_3/2",
            dvidUrl.getGrayScaleBlockUrl(1, 2, 3, 2));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale2/blocks/1_2_3/1",
            dvidUrl2.getGrayScaleBlockUrl(1, 2, 3));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale2/blocks/1_2_3/2",
            dvidUrl2.getGrayScaleBlockUrl(1, 2, 3, 2));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labels/raw/0_1_2/100_200_300/1_2_3",
            dvidUrl.getLabels64Url("labels", 100, 200, 300, 1, 2, 3));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labels/raw/0_1_2/100_200_300/1_2_3",
            dvidUrl.getLabels64Url(100, 200, 300, 1, 2, 3));
  ASSERT_TRUE(dvidUrl.getLabels64Url(100, 200, 300, 1, 2, 3, 1).empty());

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labels/raw/0_1_2/100_200_300/1_2_3",
            dvidUrl2.getLabels64Url("labels", 100, 200, 300, 1, 2, 3));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labels2/raw/0_1_2/100_200_300/1_2_3",
            dvidUrl2.getLabels64Url(100, 200, 300, 1, 2, 3));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/key/1",
            dvidUrl.getKeyUrl("test", "1"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/keys",
            dvidUrl.getAllKeyUrl("test"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/keyrange/1/3",
            dvidUrl.getKeyRangeUrl("test", "1", "3"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/annotations",
            dvidUrl.getBodyAnnotationUrl("bodies"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_annotations",
            dvidUrl.getBodyAnnotationUrl("bodies2"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/annotations/key/1",
            dvidUrl.getBodyAnnotationUrl(1));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_annotations/key/1",
            dvidUrl2.getBodyAnnotationUrl(1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodyinfo",
            dvidUrl.getBodyInfoUrl("bodies"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_bodyinfo",
            dvidUrl.getBodyInfoUrl("bodies2"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodyinfo/key/1",
            dvidUrl.getBodyInfoUrl(1, "bodies"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodyinfo/key/1",
            dvidUrl.getBodyInfoUrl(1));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_bodyinfo/key/1",
            dvidUrl2.getBodyInfoUrl(1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bound_box",
            dvidUrl.getBoundBoxUrl());
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bound_box/key/1",
            dvidUrl.getBoundBoxUrl(1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies",
            dvidUrl.getBodyLabelUrl());
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2",
            dvidUrl.getBodyLabelUrl("bodies2"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies/sizerange/100",
            dvidUrl.getBodyListUrl(100));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies/sizerange/100/200",
            dvidUrl.getBodyListUrl(100, 200));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies/allsynapse",
            dvidUrl.getSynapseListUrl());
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/annotations/key/test",
            dvidUrl.getSynapseAnnotationUrl("test"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/merge",
            dvidUrl.getMergeUrl("test"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/split/1",
            dvidUrl.getSplitUrl("test", 1));

  /*
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test",
            dvidUrl.getMergeOperationUrl("test"));
            */
//  std::cout << dvidUrl.getMergeOperationUrl("test") << std::endl;

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/neutu_merge_opr/key/labels",
            dvidUrl.getMergeOperationUrl(""));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/neutu_merge_opr/key/labels_zhaot",
            dvidUrl.getMergeOperationUrl("zhaot"));
//  std::cout << dvidUrl.getMergeOperationUrl() << std::endl;


  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/split/1",
            dvidUrl.getSplitUrl("test", 1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/split/1?splitlabel=2",
            dvidUrl.getSplitUrl("test", 1, 2));


  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/split-coarse/1",
            dvidUrl.getCoarseSplitUrl("test", 1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bookmarks/key/1_2_3",
            dvidUrl.getBookmarkKeyUrl(1, 2, 3));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bookmark_annotations/elements/100_200_300/1_2_3",
            dvidUrl.getBookmarkUrl(1, 2, 3, 100, 200, 300));


//  std::string getMergeOperationUrl(const std::string &dataName) const;

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/tiles",
            dvidUrl.getTileUrl("tiles"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/tiles/tile/xy/1",
            dvidUrl.getTileUrl("tiles", 1));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/tiles/tile/xy/0/1_2_3",
            dvidUrl.getTileUrl("tiles", 0, 1, 2, 3));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labels/label/1_2_3",
            dvidUrl.getLocalBodyIdUrl(1, 2, 3));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labels/labels",
            dvidUrl.getLocalBodyIdArrayUrl());

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/new_roi/roi",
            dvidUrl.getRoiUrl("new_roi"));


  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test",
            dvidUrl.getAnnotationUrl("test"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/sync",
            dvidUrl.getAnnotationSyncUrl("test"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/sync",
            dvidUrl.getAnnotationSyncUrl("test", ""));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/sync?replace=true",
            dvidUrl.getAnnotationSyncUrl("test", "replace=true"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/sync",
            dvidUrl.getLabelszSyncUrl("test"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/tag/zhaot",
            dvidUrl.getAnnotationUrl("test", "zhaot"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/label/123",
            dvidUrl.getAnnotationUrl("test", 123));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/elements/100_200_300/1_2_3",
            dvidUrl.getAnnotationUrl("test", 1, 2, 3, 100, 200, 300));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/elements",
            dvidUrl.getAnnotationElementsUrl("test"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/element/1_2_3",
            dvidUrl.getAnnotationDeleteUrl("test", 1, 2, 3));

  ASSERT_EQ("",
            dvidUrl.getSynapseUrl(1, 2, 3));
  ASSERT_EQ("",
            dvidUrl.getSynapseUrl(1, 2, 3, 100, 200, 300));
  ASSERT_EQ("",
            dvidUrl.getSynapseMoveUrl(ZIntPoint(1, 2, 3), ZIntPoint(4, 5, 6)));
  ASSERT_EQ("",
            dvidUrl.getSynapseUrl(1, true));
  ASSERT_EQ("",
            dvidUrl.getSynapseUrl(1, false));
  ASSERT_EQ("",
            dvidUrl.getSynapseLabelszUrl(1));
  ASSERT_EQ("",
            dvidUrl.getSynapseLabelszUrl(ZDvid::INDEX_ALL_SYN));

  target.setSynapseName("synapse");
  dvidUrl.setDvidTarget(target);
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/synapse/element/1_2_3",
            dvidUrl.getSynapseUrl(1, 2, 3));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/synapse/elements/100_200_300/1_2_3",
            dvidUrl.getSynapseUrl(1, 2, 3, 100, 200, 300));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/synapse/move/1_2_3/4_5_6",
            dvidUrl.getSynapseMoveUrl(ZIntPoint(1, 2, 3), ZIntPoint(4, 5, 6)));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/synapse/label/1?relationships=true",
            dvidUrl.getSynapseUrl(1, true));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/synapse/label/1?relationships=false",
            dvidUrl.getSynapseUrl(1, false));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/synapse_labelsz/top/1",
            dvidUrl.getSynapseLabelszUrl(1));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/synapse_labelsz/top/1/AllSyn",
            dvidUrl.getSynapseLabelszUrl(1, ZDvid::INDEX_ALL_SYN));

  target.setLabelBlockName("labelstest");
  target.setMaxLabelZoom(5);
  ZDvidUrl dvidUrl3(target);
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labelstest_2/raw/0_1_2/100_200_300/1_2_3",
            dvidUrl3.getLabels64Url(100, 200, 300, 1, 2, 3, 2));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labelstest/raw/0_1_2/100_200_300/1_2_3",
            dvidUrl3.getLabels64Url(100, 200, 300, 1, 2, 3, 0));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labelstest_5/raw/0_1_2/100_200_300/1_2_3",
            dvidUrl3.getLabels64Url(100, 200, 300, 1, 2, 3, 5));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labelstest_1/raw/0_1_2/100_200_300/1_2_3",
            dvidUrl3.getLabels64Url(100, 200, 300, 1, 2, 3, 1));
  ASSERT_EQ("", dvidUrl3.getLabels64Url(100, 200, 300, 1, 2, 3, 6));


  ZDvidUrl dvidUrl4(target, "1234");
  std::cout << dvidUrl4.getHelpUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/help", dvidUrl.getHelpUrl());

//  std::cout << dvidUrl.getSkeletonUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/node/1234/bodies2_skeletons",
            dvidUrl4.getSkeletonUrl());
  ASSERT_EQ("http://emdata.janelia.org/api/node/1234/branches/key/master",
            dvidUrl4.getMasterUrl());

  dvidUrl4.setDvidTarget(target, "3456");
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2_skeletons",
            dvidUrl4.getSkeletonUrl());
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/branches/key/master",
            dvidUrl4.getMasterUrl());
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/default_instances/key/data",
            dvidUrl4.getDefaultDataInstancesUrl());
}

TEST(ZDvidTest, Reader)
{
  ZDvidReader reader;
  ASSERT_FALSE(reader.open("foo:9001"));
  ASSERT_FALSE(reader.open("", "uuid", 1));
  ASSERT_FALSE(reader.open("server", "", 1));


//  ASSERT_TRUE(reader.open("http://emdata2.int.janelia.org:9000:2ad1"));
  if (reader.open("http://emdata2.int.janelia.org:9000:2ad1")) {
    std::cout << "Connected to " << reader.getDvidTarget().getAddressWithPort()
              << std::endl;
    ASSERT_TRUE(reader.good());
    ASSERT_TRUE(reader.isReady());

    ZDvidReader reader2;
    ASSERT_FALSE(reader2.open(reader.getDvidTarget().getAddress().c_str(), "",
                              reader.getDvidTarget().getPort()));
    ASSERT_FALSE(reader2.open("", reader.getDvidTarget().getUuid().c_str(),
                              reader.getDvidTarget().getPort()));
  }
}

TEST(ZDvidTest, ZDvidNode)
{
  ZDvidNode node;
  node.setServer("http://emdata2.int.janelia.org:9000");
  ASSERT_EQ("emdata2.int.janelia.org", node.getAddress());
  ASSERT_EQ(9000, node.getPort());

  node.setServer("http://emdata2.int.janelia.org");
  ASSERT_EQ("emdata2.int.janelia.org", node.getAddress());
  ASSERT_EQ(9000, node.getPort());

  node.clear();
  node.setServer("http://emdata2.int.janelia.org:9000/api/node/3456/branches/key/master");
  ASSERT_EQ("emdata2.int.janelia.org", node.getAddress());
  ASSERT_EQ(9000, node.getPort());

  node.clear();
  node.setServer("http://emdata2.int.janelia.org/9000/api/node/3456/branches/key/master");
  ASSERT_EQ("emdata2.int.janelia.org", node.getAddress());
  ASSERT_EQ(-1, node.getPort());

  node.setUuid("234");
  ASSERT_EQ("234", node.getUuid());
  ASSERT_EQ("emdata2.int.janelia.org", node.getAddressWithPort());

  node.setServer("emdata2.int.janelia.org:9000");
  ASSERT_EQ("emdata2.int.janelia.org", node.getAddress());
  ASSERT_EQ(9000, node.getPort());

  node.clear();
  node.setFromUrl(
        "http://emdata2.int.janelia.org:9000/api/node/3456/branches/key/master");
  ASSERT_EQ("emdata2.int.janelia.org", node.getAddress());
  ASSERT_EQ(9000, node.getPort());
  ASSERT_EQ("3456", node.getUuid());

  ZJsonObject obj;
  obj.decodeString("{\"address\":\"hackathon.janelia.org\", \"uuid\": \"2a3\"}");
  node.loadJsonObject(obj);

  ASSERT_EQ("hackathon.janelia.org", node.getAddress());
  ASSERT_EQ(-1, node.getPort());
  ASSERT_EQ("2a3", node.getUuid());

  obj.decodeString("{\"address\":\"hackathon.janelia.org\", \"port\": 8800, "
                   "\"uuid\": \"2a3\"}");
  node.loadJsonObject(obj);

  ZJsonObject obj2 = node.toJsonObject();
  ASSERT_STREQ("hackathon.janelia.org", ZJsonParser::stringValue(obj2["address"]));
  ASSERT_STREQ("2a3", ZJsonParser::stringValue(obj2["uuid"]));
  ASSERT_EQ(8800, ZJsonParser::integerValue(obj2["port"]));

  node.print();

  ZDvidNode node2;
  node2.set("emdata2.int.janelia.org", "uuid", 8000);

  ASSERT_EQ(node, node);
  ASSERT_NE(node, node2);

  ZDvidNode node3;
  node3.set("emdata2.int.janelia.org", "uuid", 8100);
  ASSERT_NE(node2, node3);
}

TEST(ZDvidTest, ZDvidTarget)
{
  ZDvidTarget target;
  target.setServer("http://emdata2.int.janelia.org:9000");
  ASSERT_EQ("emdata2.int.janelia.org", target.getAddress());
  ASSERT_EQ(9000, target.getPort());

  target.setServer("http://emdata2.int.janelia.org");
  ASSERT_EQ("emdata2.int.janelia.org", target.getAddress());
  ASSERT_EQ(9000, target.getPort());

  target.clear();
  target.setServer("http://emdata2.int.janelia.org:9000/api/node/3456/branches/key/master");
  ASSERT_EQ("emdata2.int.janelia.org", target.getAddress());
  ASSERT_EQ(9000, target.getPort());

  target.clear();
  target.setServer("http://emdata2.int.janelia.org/9000/api/node/3456/branches/key/master");
  ASSERT_EQ("emdata2.int.janelia.org", target.getAddress());
  ASSERT_EQ(-1, target.getPort());

  target.setUuid("234");
  ASSERT_EQ("234", target.getUuid());
  ASSERT_EQ("emdata2.int.janelia.org", target.getAddressWithPort());

  target.setServer("emdata2.int.janelia.org:9000");
  ASSERT_EQ("emdata2.int.janelia.org", target.getAddress());
  ASSERT_EQ(9000, target.getPort());

  target.clear();
  target.setFromUrl(
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
}

#endif

#endif // ZDVIDTEST_H

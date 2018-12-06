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
#include "zintcuboid.h"
#include "zobject3dscan.h"

#ifdef _USE_GTEST_

TEST(ZDvidTest, ZDvidInfo)
{
  ZDvidInfo info;
  info.print();
  std::cout << std::endl;
  /*
  const char* ZDvidInfo::m_minPointKey = "MinPoint";
  const char* ZDvidInfo::m_maxPointKey = "MaxPoint";
  const char* ZDvidInfo::m_blockSizeKey = "BlockSize";
  const char* ZDvidInfo::m_voxelSizeKey = "VoxelSize";
  const char* ZDvidInfo::m_blockMinIndexKey = "MinIndex";
*/
  info.setFromJsonString("{ "
                         " \"MinPoint\": [1, 2, 3], "
                         " \"BlockSize\": [16, 32, 64],"
                         " \"MaxPoint\": [1000, 2000, 3000],"
                         " \"MinIndex\": [4, 5, 6],"
                         " \"MaxIndex\": [1000, 2000, 3000]"
                         "}");
  info.print();

  ASSERT_EQ(1, info.getStartCoordinates().getX());
  ASSERT_EQ(2, info.getStartCoordinates().getY());
  ASSERT_EQ(3, info.getStartCoordinates().getZ());

  ASSERT_EQ(1000, info.getEndCoordinates().getX());
  ASSERT_EQ(2000, info.getEndCoordinates().getY());
  ASSERT_EQ(3000, info.getEndCoordinates().getZ());

  ASSERT_EQ(16, info.getBlockSize().getX());
  ASSERT_EQ(32, info.getBlockSize().getY());
  ASSERT_EQ(64, info.getBlockSize().getZ());

  ASSERT_EQ(ZIntPoint(0, 0, 0), info.getBlockIndex(0, 0, 0));
  ASSERT_EQ(ZIntPoint(-1, -1, -1), info.getBlockIndex(-1, -1, -1));
  ASSERT_EQ(ZIntPoint(1, 1, 1), info.getBlockIndex(16, 32, 64));
  ASSERT_EQ(ZIntPoint(-1, -1, -1), info.getBlockIndex(-16, -32, -64));
  ASSERT_EQ(ZIntPoint(-2, -2, -2), info.getBlockIndex(-17, -33, -65));
  ASSERT_EQ(ZIntPoint(0, 0, 0), info.getBlockCoord(0, 0, 0));
  ASSERT_EQ(ZIntPoint(16, 32, 64), info.getBlockCoord(1, 1, 1));
  ZIntCuboid box = info.getBlockBox(1, 2, 3);
  ASSERT_EQ(ZIntPoint(16, 64, 192), box.getFirstCorner());
  ASSERT_EQ(16, box.getWidth());
  ASSERT_EQ(32, box.getHeight());
  ASSERT_EQ(64, box.getDepth());

  ASSERT_TRUE(info.isValidBlockIndex(ZIntPoint(100, 20, 30)));
  ASSERT_FALSE(info.isValidBlockIndex(ZIntPoint(100, 20, 3)));

  ZObject3dScan obj;
  obj.addSegment(-1, -2, -1, 10);
  ZObject3dScan obj2 = info.getBlockIndex(obj);
  ASSERT_EQ(2, (int) obj2.getVoxelNumber());
  ASSERT_TRUE(obj2.contains(-1, -1, -1));
  ASSERT_TRUE(obj2.contains(0, -1, -1));
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
  target.setSegmentationName("labels");
  target.setBodyLabelName("bodies");
  target.setGrayScaleName("grayscale");

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

  ASSERT_EQ("", dvidUrl.getSkeletonUrl(""));

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

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/meshes/key/1",
            dvidUrl.getMeshUrl(1, 0));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/meshes_2/key/1",
            dvidUrl.getMeshUrl(1, 2));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/meshes/key/1_info",
            dvidUrl.getMeshInfoUrl(1, 0));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/meshes_2/key/1_info",
            dvidUrl.getMeshInfoUrl(1, 2));

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
  target.setSegmentationName("labels2");
  ZDvidUrl dvidUrl2(target);
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_skeletons",
            dvidUrl2.getSkeletonUrl());
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_skeletons/key/1_swc",
            dvidUrl2.getSkeletonUrl(1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_meshes/key/1",
            dvidUrl2.getMeshUrl(1, 0));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_meshes_1/key/1",
            dvidUrl2.getMeshUrl(1, 1));

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

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2/sparsevol/1",
            dvidUrl2.getMultiscaleSparsevolUrl(1, 0));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_1/sparsevol/1",
            dvidUrl2.getMultiscaleSparsevolUrl(1, 1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labels2/mapping",
            dvidUrl2.getLabelMappingUrl());



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

  target.setSegmentationName("labelstest");
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
  ASSERT_TRUE(dvidUrl4.getSparsevolSizeUrl(1).empty());

//  target.useLabelArray(true);
  target.setSegmentationType(ZDvidData::TYPE_LABELARRAY);
  dvidUrl4.setDvidTarget(target, "3456");
//  std::cout << target.getBodyLabelName() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol-size/1",
            dvidUrl4.getSparsevolSizeUrl(1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1",
            dvidUrl4.getSparsevolUrl(1));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?minx=2&maxx=2&exact=true",
            dvidUrl4.getSparsevolUrl(1, 2, neutube::EAxis::X));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?miny=2&maxy=2&exact=true",
            dvidUrl4.getSparsevolUrl(1, 2, neutube::EAxis::Y));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?minz=2&maxz=2&exact=true",
            dvidUrl4.getSparsevolUrl(1, 2, neutube::EAxis::Z));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?minx=2&maxx=3&exact=true",
            dvidUrl4.getSparsevolUrl(1, 2, 3, neutube::EAxis::X));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?miny=2&maxy=3&exact=true",
            dvidUrl4.getSparsevolUrl(1, 2, 3, neutube::EAxis::Y));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?minz=2&maxz=3&exact=true",
            dvidUrl4.getSparsevolUrl(1, 2, 3, neutube::EAxis::Z));
  ZIntCuboid box(10, 20, 30, 40, 50, 60);
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?minx=10&maxx=40&miny=20&maxy=50&minz=30&maxz=60",
            dvidUrl4.getSparsevolUrl(1, box));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1?scale=2",
            dvidUrl4.getMultiscaleSparsevolUrl(1, 2));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1?scale=2",
            dvidUrl4.getMultiscaleSparsevolUrl(1, 2));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?scale=2&minx=10&maxx=40&miny=20&maxy=50&minz=30&maxz=60",
            dvidUrl4.getSparsevolUrl(1, 2, box));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1?supervoxels=true",
            dvidUrl4.getSupervoxelUrl(1));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1?supervoxels=true&minz=2&maxz=3&exact=true",
            dvidUrl4.getSupervoxelUrl(1, 2, 3, neutube::EAxis::Z));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1?supervoxels=true&scale=2",
            dvidUrl4.getMultiscaleSupervoxelUrl(1, 2));


  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/test/label/123",
            dvidUrl4.getAnnotationUrl("test", 123));

  ASSERT_EQ(12345, (int) ZDvidUrl::GetBodyId(
              "http://localhost:8000/api/node/uuid/segname/sparsevol/12345"));
  ASSERT_EQ(0, (int) ZDvidUrl::GetBodyId(
              "http://localhost:8000/api/node/uuid/segname/sparsevol/"));
  ASSERT_EQ(uint64_t(123451234512345L), ZDvidUrl::GetBodyId(
              "http://localhost:8000/api/node/uuid/segname/sparsevol/123451234512345"));

  ASSERT_EQ("head__6c8409b833d57d9c62856b6cab608aa5",
            ZDvidUrl::ExtractSplitTaskKey(
              "http://localhost:8000/api/node/4d3e/task_split/key/"
              "head__6c8409b833d57d9c62856b6cab608aa5"));
  ASSERT_EQ("",
            ZDvidUrl::ExtractSplitTaskKey(
              "http://localhost:8000/api/node/4d3e/split/key/"
              "head__6c8409b833d57d9c62856b6cab608aa5"));
  ASSERT_EQ("",
            ZDvidUrl::ExtractSplitTaskKey(
              "http://localhost:8000/api/node/4d3e/split/key/"));

  target.setSegmentationType(ZDvidData::TYPE_LABELMAP);
  std::cout << target.getSegmentationName() << std::endl;
  dvidUrl4.setDvidTarget(target, "3456");
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/test/label/123",
            dvidUrl4.getAnnotationUrl("test", 123));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/labelstest/size/123",
            dvidUrl4.getBodySizeUrl(123));

  target.setBodyLabelName("segmentation");
  target.setUuid("3456");
  dvidUrl4.setDvidTarget(target);
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/segmentation_sv_meshes/supervoxel/123",
            dvidUrl4.getSupervoxelMeshUrl(123));

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

  node.clear();
  node.setServer("http://emdata2.int.janelia.org:9000/api/node/23456/branches/key/master");
  ASSERT_EQ("emdata2.int.janelia.org", node.getAddress());
  ASSERT_EQ(9000, node.getPort());

  node.setUuid("23456");
  ASSERT_EQ("23456", node.getUuid());

  node.setServer("emdata2.int.janelia.org:9000");
  ASSERT_EQ("emdata2.int.janelia.org", node.getAddress());
  ASSERT_EQ(9000, node.getPort());

  node.clear();
  node.setFromUrl(
        "http://emdata2.int.janelia.org:9000/api/node/3456/branches/key/master");
  ASSERT_EQ("emdata2.int.janelia.org", node.getAddress());
  ASSERT_EQ(9000, node.getPort());
  ASSERT_EQ("3456", node.getUuid());

  node.clear();
  node.setFromUrl(
        "http://emdata2.int.janelia.org:9000/api/node/123456/branches/key/master");
  ASSERT_EQ("emdata2.int.janelia.org", node.getAddress());
  ASSERT_EQ(9000, node.getPort());
  ASSERT_EQ("123456", node.getUuid());

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
  ASSERT_EQ("hackathon.janelia.org", ZJsonParser::stringValue(obj2["address"]));
  ASSERT_EQ("2a3", ZJsonParser::stringValue(obj2["uuid"]));
  ASSERT_EQ(8800, ZJsonParser::integerValue(obj2["port"]));

  node.print();

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

  node.setFromUrl(
        "http://emdata2.int.janelia.org:9000/api/node/3456/branches/key/master");
  ASSERT_FALSE(node.isMock());
  ASSERT_EQ("http:emdata2.int.janelia.org:9000:3456", node.getSourceString(true));

  node.setFromUrl(
        "mock://emdata2.int.janelia.org:9000/api/node/3456/branches/key/master");
  ASSERT_TRUE(node.isMock());
  ASSERT_EQ("mock:emdata2.int.janelia.org:9000:3456", node.getSourceString(true));
}

TEST(ZDvidTest, ZDvidTarget)
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
  target.setFromUrl(
        "mock://emdata2.int.janelia.org:9000/api/node/3456/branches/key/master");
  ASSERT_EQ("emdata2.int.janelia.org", target.getAddress());
  ASSERT_EQ(9000, target.getPort());
  ASSERT_EQ("3456", target.getUuid());
  target.setTodoListName("test");
  ASSERT_EQ("test", target.getTodoListName());
  ASSERT_TRUE(target.isMock());
  ASSERT_EQ("mock:emdata2.int.janelia.org:9000:3456:*", target.getSourceString());

  target.setFromUrl(
        "http://emdata3.int.janelia.org:9100/api/node/1234/body_test/sparsevol/123");
  ASSERT_EQ("emdata3.int.janelia.org", target.getAddress());
  ASSERT_EQ(9100, target.getPort());
  ASSERT_EQ("1234", target.getUuid());
  target.setTodoListName("test");
  ASSERT_EQ("test", target.getTodoListName());
  ASSERT_FALSE(target.isMock());
  ASSERT_EQ("body_test", target.getBodyLabelName());
  ASSERT_EQ("http:emdata3.int.janelia.org:9100:1234:body_test", target.getSourceString());

  target.setFromUrl(
        "mock://emdata3.int.janelia.org:9100/api/node/1234/body_test/sparsevol/123");
  ASSERT_EQ("emdata3.int.janelia.org", target.getAddress());
  ASSERT_EQ(9100, target.getPort());
  ASSERT_EQ("1234", target.getUuid());
  target.setTodoListName("test");
  ASSERT_EQ("test", target.getTodoListName());
  ASSERT_TRUE(target.isMock());
  ASSERT_EQ("body_test", target.getBodyLabelName());
  ASSERT_EQ("mock:emdata3.int.janelia.org:9100:1234:body_test", target.getSourceString());
}

TEST(ZDvidTest, DataType)
{
  ASSERT_EQ(ZDvid::TYPE_LABELBLK, ZDvid::GetDataType("labelblk"));
  ASSERT_EQ(ZDvid::TYPE_ANNOTATION, ZDvid::GetDataType("annotation"));
  ASSERT_EQ(ZDvid::TYPE_IMAGETILE, ZDvid::GetDataType("imagetile"));
  ASSERT_EQ(ZDvid::TYPE_KEYVALUE, ZDvid::GetDataType("keyvalue"));
  ASSERT_EQ(ZDvid::TYPE_LABELGRAPH, ZDvid::GetDataType("labelgraph"));
  ASSERT_EQ(ZDvid::TYPE_LABELSZ, ZDvid::GetDataType("labelsz"));
  ASSERT_EQ(ZDvid::TYPE_LABELVOL, ZDvid::GetDataType("labelvol"));
  ASSERT_EQ(ZDvid::TYPE_ROI, ZDvid::GetDataType("roi"));
  ASSERT_EQ(ZDvid::TYPE_UINT8BLK, ZDvid::GetDataType("uint8blk"));

  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "e2f0", 7000);
  target.setSegmentationName("segmentation2");
  ZDvidReader reader;
  if (reader.open(target)) {
    reader.syncBodyLabelName();
    ASSERT_EQ("segmentation-labelvol", reader.getDvidTarget().getBodyLabelName());
  }
}

#endif

#endif // ZDVIDTEST_H

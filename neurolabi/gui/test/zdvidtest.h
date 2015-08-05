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

TEST(ZDvidTest, ZDvidUrl)
{
  ZDvidTarget target("emdata.janelia.org", "bf1");
  target.setLabelBlockName("labels");
  //const std::vector<ZDvidTarget> &dvidRepo =
  //    NeutubeConfig::getInstance().getFlyEmConfig().getDvidRepo();
  ZDvidUrl dvidUrl(target);
  std::cout << dvidUrl.getHelpUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/help", dvidUrl.getHelpUrl());

  std::cout << dvidUrl.getSkeletonUrl("") << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/skeletons",
            dvidUrl.getSkeletonUrl(""));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/skeletons/key/1_swc",
            dvidUrl.getSkeletonUrl(1));

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
//  std::cout << dvidUrl.getMergeOperationUrl() << std::endl;

//  std::string getMergeOperationUrl(const std::string &dataName) const;

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/tiles",
            dvidUrl.getTileUrl("tiles"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/tiles/tile/xy/1",
            dvidUrl.getTileUrl("tiles", 1));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/tiles/tile/xy/0/1_2_3",
            dvidUrl.getTileUrl("tiles", 0, 1, 2, 3));



//  std::string getRepoInfoUrl() const;
//  std::string getLockUrl() const;
//  std::string getBranchUrl() const;

//  static std::string GetEndPoint(const std::string &url);
}

#endif

#endif // ZDVIDTEST_H
